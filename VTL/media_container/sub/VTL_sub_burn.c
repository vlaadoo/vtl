#include "VTL_sub_burn.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Для realpath
#include <errno.h> // Для strerror
#include <unistd.h> // Для unlink

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/channel_layout.h>
#include <libavutil/audio_fifo.h>

#include "VTL_sub_read.h"
#include "VTL_sub_write.h"
#include "VTL_sub_style.h"

// Структура для хранения контекстов и информации о потоках
typedef struct ProcessingContext {
    AVFormatContext *ifmt_ctx_video; // Контекст для входного видеофайла
    AVFormatContext *ofmt_ctx;       // Контекст для выходного файла

    AVCodecContext *video_dec_ctx;
    AVCodecContext *video_enc_ctx;
    AVCodecContext *audio_dec_ctx;
    AVCodecContext *audio_enc_ctx;

    AVFilterGraph *filter_graph;
    AVFilterContext *buffersrc_ctx;  // Источник для видео фильтра
    AVFilterContext *buffersink_ctx; // Приемник для видео фильтра

    int video_stream_idx_in;
    int audio_stream_idx_in;
    int video_stream_idx_out;
    int audio_stream_idx_out;

    AVFrame *decoded_frame;
    AVFrame *filtered_frame;
    AVPacket *encoded_packet;
    
    char subs_abs_path[2048];
    const char* output_filename;

} ProcessingContext;

static void cleanup_processing_context(ProcessingContext *pctx) {
    if (!pctx) return;

    if (pctx->video_dec_ctx) avcodec_free_context(&pctx->video_dec_ctx);
    if (pctx->video_enc_ctx) avcodec_free_context(&pctx->video_enc_ctx);
    if (pctx->audio_dec_ctx) avcodec_free_context(&pctx->audio_dec_ctx);
    if (pctx->audio_enc_ctx) avcodec_free_context(&pctx->audio_enc_ctx);

    if (pctx->ifmt_ctx_video) avformat_close_input(&pctx->ifmt_ctx_video);
    if (pctx->ofmt_ctx && !(pctx->ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&pctx->ofmt_ctx->pb);
    }
    if (pctx->ofmt_ctx) avformat_free_context(pctx->ofmt_ctx);

    if (pctx->filter_graph) avfilter_graph_free(&pctx->filter_graph);
    
    if (pctx->decoded_frame) av_frame_free(&pctx->decoded_frame);
    if (pctx->filtered_frame) av_frame_free(&pctx->filtered_frame);
    if (pctx->encoded_packet) av_packet_free(&pctx->encoded_packet);

    // pctx->subs_abs_path и pctx->output_filename не требуют free, если они указывают на внешние данные или являются массивами
    // free(pctx); // Если pctx был динамически выделен
}


static int open_input_file(ProcessingContext *pctx, const char *filename) {
    int ret;
    pctx->ifmt_ctx_video = NULL;
    if ((ret = avformat_open_input(&pctx->ifmt_ctx_video, filename, NULL, NULL)) < 0) {
        fprintf(stderr, "Не удалось открыть входной файл '%s': %s\n", filename, av_err2str(ret));
        return ret;
    }
    if ((ret = avformat_find_stream_info(pctx->ifmt_ctx_video, NULL)) < 0) {
        fprintf(stderr, "Не удалось найти информацию о потоках: %s\n", av_err2str(ret));
        return ret;
    }

    // Находим видео и аудио потоки и их декодеры
    for (unsigned int i = 0; i < pctx->ifmt_ctx_video->nb_streams; i++) {
        AVStream *stream = pctx->ifmt_ctx_video->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
        if (!codec) {
            fprintf(stderr, "Не найден декодер для потока %u (codec_id %d)\n", i, codecpar->codec_id);
            continue;
        }
        AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) {
            fprintf(stderr, "Не удалось выделить контекст для декодера\n");
            return AVERROR(ENOMEM);
        }
        if ((ret = avcodec_parameters_to_context(codec_ctx, codecpar)) < 0) {
            fprintf(stderr, "Не удалось скопировать параметры кодека в контекст: %s\n", av_err2str(ret));
            avcodec_free_context(&codec_ctx);
            return ret;
        }
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            pctx->video_stream_idx_in = i;
            pctx->video_dec_ctx = codec_ctx;
            // Устанавливаем временную базу для декодера из потока
             if (stream->time_base.den && stream->time_base.num) {
                pctx->video_dec_ctx->time_base = stream->time_base;
            }
        } else if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            pctx->audio_stream_idx_in = i;
            pctx->audio_dec_ctx = codec_ctx;
             if (stream->time_base.den && stream->time_base.num) {
                pctx->audio_dec_ctx->time_base = stream->time_base;
            }
        } else {
            avcodec_free_context(&codec_ctx); // Освобождаем, если не видео и не аудио
        }
    }

    if (pctx->video_stream_idx_in == -1) {
        fprintf(stderr, "Входной файл не содержит видео потока.\n");
        return AVERROR_STREAM_NOT_FOUND;
    }
    if (pctx->audio_stream_idx_in == -1) {
        fprintf(stderr, "Входной файл не содержит аудио потока (продолжаем без аудио).\n");
        // Это не фатальная ошибка, можно продолжить только с видео
    }
    
    // Открываем декодеры
    if (pctx->video_dec_ctx && (ret = avcodec_open2(pctx->video_dec_ctx, avcodec_find_decoder(pctx->video_dec_ctx->codec_id), NULL)) < 0) {
        fprintf(stderr, "Не удалось открыть видео декодер: %s\n", av_err2str(ret));
        return ret;
    }
    if (pctx->audio_dec_ctx && (ret = avcodec_open2(pctx->audio_dec_ctx, avcodec_find_decoder(pctx->audio_dec_ctx->codec_id), NULL)) < 0) {
        fprintf(stderr, "Не удалось открыть аудио декодер: %s\n", av_err2str(ret));
        // Можно решить продолжить без аудио, если это приемлемо
        avcodec_free_context(&pctx->audio_dec_ctx);
        pctx->audio_dec_ctx = NULL; 
        pctx->audio_stream_idx_in = -1;
    }
    
    //av_dump_format(pctx->ifmt_ctx_video, 0, filename, 0); // Для отладки
    return 0;
}

static int setup_video_encoder(ProcessingContext *pctx, AVStream *out_stream) {
    int ret;
    const AVCodec *encoder = avcodec_find_encoder_by_name("mpeg4");
    if (!encoder) {
        fprintf(stderr, "Не найден кодек mpeg4\n");
        return AVERROR_ENCODER_NOT_FOUND;
    }
    pctx->video_enc_ctx = avcodec_alloc_context3(encoder);
    if (!pctx->video_enc_ctx) return AVERROR(ENOMEM);

    pctx->video_enc_ctx->height = pctx->video_dec_ctx->height;
    pctx->video_enc_ctx->width = pctx->video_dec_ctx->width;
    pctx->video_enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P; // Требуемый формат пикселей
    
    // Временная база для энкодера должна соответствовать временной базе выходного потока.
    // Для фильтров она обычно 1/framerate. Buffersink установит правильную time_base для кадров.
    // Здесь мы устанавливаем ее на основе входного видео или фильтра.
    // Лучше всего, если buffersink сообщит нам свою time_base.
    // Пока установим из декодера или из фильтра (если уже настроен).
    // В данном случае, после фильтрации, кадры будут иметь time_base от buffersink.
    if (pctx->buffersink_ctx) { // Если фильтр уже настроен
         pctx->video_enc_ctx->time_base = av_buffersink_get_time_base(pctx->buffersink_ctx);
         pctx->video_enc_ctx->framerate = av_buffersink_get_frame_rate(pctx->buffersink_ctx);
    } else { // Если фильтра нет, берем из декодера (упрощение)
        pctx->video_enc_ctx->time_base = pctx->video_dec_ctx->time_base; // Может потребоваться уточнение
        pctx->video_enc_ctx->framerate = pctx->video_dec_ctx->framerate;
    }


    pctx->video_enc_ctx->bit_rate = pctx->video_dec_ctx->bit_rate > 0 ? pctx->video_dec_ctx->bit_rate : 2000000; // Примерный битрейт, если не известен
    // Для -q:v 2 (mpeg4), это обычно означает использование qscale
    pctx->video_enc_ctx->flags |= AV_CODEC_FLAG_QSCALE;
    pctx->video_enc_ctx->global_quality = 2 * FF_QP2LAMBDA; // q:v 2

    if (pctx->ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        pctx->video_enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if ((ret = avcodec_open2(pctx->video_enc_ctx, encoder, NULL)) < 0) {
        fprintf(stderr, "Не удалось открыть видео энкодер: %s\n", av_err2str(ret));
        return ret;
    }
    if ((ret = avcodec_parameters_from_context(out_stream->codecpar, pctx->video_enc_ctx)) < 0) {
        fprintf(stderr, "Не удалось скопировать параметры кодека из контекста в поток: %s\n", av_err2str(ret));
        return ret;
    }
    out_stream->time_base = pctx->video_enc_ctx->time_base;
    return 0;
}

static int setup_audio_encoder(ProcessingContext *pctx, AVStream *out_stream) {
    if (!pctx->audio_dec_ctx) return 0; // Нет входного аудио, нечего кодировать

    int ret;
    const AVCodec *encoder = avcodec_find_encoder_by_name("aac");
    if (!encoder) {
        fprintf(stderr, "Не найден кодек aac\n");
        return AVERROR_ENCODER_NOT_FOUND;
    }
    pctx->audio_enc_ctx = avcodec_alloc_context3(encoder);
    if (!pctx->audio_enc_ctx) return AVERROR(ENOMEM);

    // Параметры для AAC энкодера
    pctx->audio_enc_ctx->sample_fmt = encoder->sample_fmts ? encoder->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    pctx->audio_enc_ctx->bit_rate = pctx->audio_dec_ctx->bit_rate > 0 ? pctx->audio_dec_ctx->bit_rate : 128000; // Типичный битрейт
    pctx->audio_enc_ctx->sample_rate = pctx->audio_dec_ctx->sample_rate;
    
    // Сохранение channel_layout
    AVChannelLayout out_ch_layout;
    av_channel_layout_copy(&out_ch_layout, &pctx->audio_dec_ctx->ch_layout);
    av_channel_layout_copy(&pctx->audio_enc_ctx->ch_layout, &out_ch_layout);
    av_channel_layout_uninit(&out_ch_layout);


    if (pctx->ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        pctx->audio_enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    AVDictionary *enc_opts = NULL;
    // av_dict_set(&enc_opts, "strict", "experimental", 0); // Для некоторых AAC энкодеров может потребоваться

    if ((ret = avcodec_open2(pctx->audio_enc_ctx, encoder, &enc_opts)) < 0) {
        fprintf(stderr, "Не удалось открыть аудио энкодер: %s\n", av_err2str(ret));
        av_dict_free(&enc_opts);
        return ret;
    }
    av_dict_free(&enc_opts);

    if ((ret = avcodec_parameters_from_context(out_stream->codecpar, pctx->audio_enc_ctx)) < 0) {
        fprintf(stderr, "Не удалось скопировать параметры аудио кодека из контекста в поток: %s\n", av_err2str(ret));
        return ret;
    }
    out_stream->time_base = (AVRational){1, pctx->audio_enc_ctx->sample_rate};
    return 0;
}


static int init_filters(ProcessingContext *pctx) {
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVRational time_base = pctx->ifmt_ctx_video->streams[pctx->video_stream_idx_in]->time_base;
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE}; // Формат, который мы хотим на выходе фильтра

    pctx->filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !pctx->filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    // Описание источника видео (buffer)
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             pctx->video_dec_ctx->width, pctx->video_dec_ctx->height, pctx->video_dec_ctx->pix_fmt,
             time_base.num, time_base.den,
             pctx->video_dec_ctx->sample_aspect_ratio.num, pctx->video_dec_ctx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&pctx->buffersrc_ctx, buffersrc, "in", args, NULL, pctx->filter_graph);
    if (ret < 0) {
        fprintf(stderr, "Не удалось создать buffer source: %s\n", av_err2str(ret));
        goto end;
    }

    // Описание приемника видео (buffersink)
    ret = avfilter_graph_create_filter(&pctx->buffersink_ctx, buffersink, "out", NULL, NULL, pctx->filter_graph);
    if (ret < 0) {
        fprintf(stderr, "Не удалось создать buffer sink: %s\n", av_err2str(ret));
        goto end;
    }

    outputs->name = av_strdup("in");
    outputs->filter_ctx = pctx->buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = pctx->buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    // Строка фильтров: subtitles=filename='<path_to_subs_file>'
    // Важно: в Windows путь к файлу может содержать ':', который нужно экранировать.
    // Для Linux/macOS это обычно не проблема. realpath должен дать канонический путь.
    char filter_spec[3072]; // 2048 для пути + остальное
    // Экранирование двоеточий и обратных слешей для Windows-путей в ffmpeg filtergraph
    char escaped_subs_path[2048*2+100] = {0}; // Достаточно места для экранирования
    const char *p_in = pctx->subs_abs_path;
    char *p_out = escaped_subs_path;
    while(*p_in) {
        if (*p_in == '\\' || *p_in == ':') { // Экранируем \ и :
            *p_out++ = '\\';
        }
        *p_out++ = *p_in++;
    }
    *p_out = '\0';

    snprintf(filter_spec, sizeof(filter_spec), "subtitles=filename='%s':original_size=%dx%d,format=pix_fmts=yuv420p",
             escaped_subs_path, pctx->video_dec_ctx->width, pctx->video_dec_ctx->height);


    if ((ret = avfilter_graph_parse_ptr(pctx->filter_graph, filter_spec, &inputs, &outputs, NULL)) < 0) {
        fprintf(stderr, "Не удалось разобрать строку фильтров '%s': %s\n", filter_spec, av_err2str(ret));
        goto end;
    }

    if ((ret = avfilter_graph_config(pctx->filter_graph, NULL)) < 0) {
        fprintf(stderr, "Не удалось сконфигурировать граф фильтров: %s\n", av_err2str(ret));
        goto end;
    }
    
    // Установка временной базы для видео энкодера из buffersink
    if (pctx->video_enc_ctx) {
        pctx->video_enc_ctx->time_base = av_buffersink_get_time_base(pctx->buffersink_ctx);
        pctx->video_enc_ctx->framerate = av_buffersink_get_frame_rate(pctx->buffersink_ctx);
    }


end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return ret;
} 

static int encode_write_frame(ProcessingContext *pctx, AVFrame *frame, int stream_idx) {
    int ret;
    AVCodecContext *enc_ctx = (stream_idx == pctx->video_stream_idx_out) ? pctx->video_enc_ctx : pctx->audio_enc_ctx;
    AVStream *out_stream = pctx->ofmt_ctx->streams[stream_idx];

    if (frame) {
         // Убедимся, что у кадра правильная временная база перед кодированием
        if (enc_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
             // Для видео кадров из фильтра, time_base должна быть от buffersink
            frame->pict_type = AV_PICTURE_TYPE_NONE; // Позволяем энкодеру решать
        }
    }
   
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Ошибка отправки кадра энкодеру: %s\n", av_err2str(ret));
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pctx->encoded_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return (ret == AVERROR_EOF && frame == NULL) ? 0 : 1; // 1 = нужно больше данных, 0 = EOF обработан
        } else if (ret < 0) {
            fprintf(stderr, "Ошибка получения пакета от энкодера: %s\n", av_err2str(ret));
            return ret;
        }

        pctx->encoded_packet->stream_index = stream_idx;
        av_packet_rescale_ts(pctx->encoded_packet, enc_ctx->time_base, out_stream->time_base);
        
        //printf("Writing packet: pts=%s, dts=%s, duration=%s, size=%d, stream_idx=%d\n", 
        //   av_ts2timestr(pctx->encoded_packet->pts, &out_stream->time_base),
        //   av_ts2timestr(pctx->encoded_packet->dts, &out_stream->time_base),
        //   av_ts2timestr(pctx->encoded_packet->duration, &out_stream->time_base),
        //   pctx->encoded_packet->size,
        //   pctx->encoded_packet->stream_index);


        ret = av_interleaved_write_frame(pctx->ofmt_ctx, pctx->encoded_packet);
        av_packet_unref(pctx->encoded_packet);
        if (ret < 0) {
            fprintf(stderr, "Ошибка записи пакета в выходной файл: %s\n", av_err2str(ret));
            return ret;
        }
    }
    return 0;
}


int VTL_sub_burn_to_video(const char* input_video, const char* input_subs, VTL_sub_Format subs_format, const char* output_video, const VTL_SubStyleParams* style_params) {
    ProcessingContext pctx_data = {0};
    ProcessingContext *pctx = &pctx_data;
    pctx->video_stream_idx_in = -1;
    pctx->audio_stream_idx_in = -1;
    int ret = 0;
    char temp_ass_path[256] = {0};
    FILE* temp_ass = NULL;
    VTL_SubReadSource* sub_src = NULL;
    VTL_SubWriteSink* sub_sink = NULL;
    VTL_SubWriteMeta wmeta = { .format = VTL_sub_format_kASS };

    if (!input_video || !input_subs || !output_video) {
        fprintf(stderr, "Не указаны входные/выходные файлы или файл субтитров.\n");
        return -1;
    }

    // --- Новый блок: поэтапное чтение и запись субтитров во временный файл ASS ---
    snprintf(temp_ass_path, sizeof(temp_ass_path), "/tmp/vtl_burn_temp_%d.ass", getpid());
    ret = VTL_sub_read_OpenSource(input_subs, &sub_src);
    if (ret != VTL_res_kOk) {
        fprintf(stderr, "[burn] Не удалось открыть субтитры для чтения: %d\n", ret);
        return -1;
    }
    ret = VTL_sub_write_OpenOutput(temp_ass_path, &wmeta, &sub_sink);
    if (ret != VTL_res_kOk) {
        VTL_sub_read_CloseSource(&sub_src);
        fprintf(stderr, "[burn] Не удалось открыть временный файл для записи субтитров: %d\n", ret);
        return -1;
    }
    VTL_SubEntry entry;
    while (VTL_sub_read_ReadPart(sub_src, &entry) == VTL_res_kOk) {
        VTL_sub_write_WritePart(sub_sink, &entry);
        if (entry.text) free(entry.text);
        if (entry.style) free(entry.style);
    }
    VTL_sub_read_CloseSource(&sub_src);
    VTL_sub_write_CloseOutput(&sub_sink, style_params);
    // --- Конец блока ---
    // Получаем абсолютный путь к временному файлу субтитров
    if (realpath(temp_ass_path, pctx->subs_abs_path) == NULL) {
        strncpy(pctx->subs_abs_path, temp_ass_path, sizeof(pctx->subs_abs_path) - 1);
        pctx->subs_abs_path[sizeof(pctx->subs_abs_path) - 1] = '\0';
    }
    pctx->output_filename = output_video;


    // 1. Открытие входного файла и инициализация декодеров
    if ((ret = open_input_file(pctx, input_video)) < 0) {
        fprintf(stderr, "[burn] Ошибка open_input_file: %d (%s)\n", ret, av_err2str(ret));
        goto cleanup;
    }

    // 2. Настройка выходного файла и энкодеров
    avformat_alloc_output_context2(&pctx->ofmt_ctx, NULL, NULL, output_video);
    if (!pctx->ofmt_ctx) {
        fprintf(stderr, "[burn] Не удалось создать выходной контекст\n");
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    // Создание выходных потоков на основе входных (или параметров энкодера)
    if (pctx->video_dec_ctx) {
        AVStream *out_vid_stream = avformat_new_stream(pctx->ofmt_ctx, NULL);
        if (!out_vid_stream) { ret = AVERROR(ENOMEM); goto cleanup; }
        pctx->video_stream_idx_out = out_vid_stream->index;
    }
    if (pctx->audio_dec_ctx) {
        AVStream *out_aud_stream = avformat_new_stream(pctx->ofmt_ctx, NULL);
        if (!out_aud_stream) { ret = AVERROR(ENOMEM); goto cleanup; }
        pctx->audio_stream_idx_out = out_aud_stream->index;
    }
    
    // 3. Инициализация фильтров (после декодеров, до энкодеров)
    if (pctx->video_dec_ctx) {
         if ((ret = init_filters(pctx)) < 0) {
            fprintf(stderr, "[burn] Ошибка init_filters: %d (%s)\n", ret, av_err2str(ret));
            goto cleanup;
        }
    }
    
    // 4. Настройка энкодеров (после фильтров, чтобы time_base была корректной)
    if (pctx->video_dec_ctx) {
        if ((ret = setup_video_encoder(pctx, pctx->ofmt_ctx->streams[pctx->video_stream_idx_out])) < 0) {
            fprintf(stderr, "[burn] Ошибка setup_video_encoder: %d (%s)\n", ret, av_err2str(ret));
            goto cleanup;
        }
    }
    if (pctx->audio_dec_ctx) {
         if ((ret = setup_audio_encoder(pctx, pctx->ofmt_ctx->streams[pctx->audio_stream_idx_out])) < 0) {
            fprintf(stderr, "[burn] Ошибка setup_audio_encoder: %d (%s)\n", ret, av_err2str(ret));
            avcodec_free_context(&pctx->audio_enc_ctx);
            pctx->audio_enc_ctx = NULL;
        }
    }


    if (!(pctx->ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&pctx->ofmt_ctx->pb, output_video, AVIO_FLAG_WRITE)) < 0) {
            fprintf(stderr, "[burn] Не удалось открыть выходной файл '%s': %d (%s)\n", output_video, ret, av_err2str(ret));
            goto cleanup;
        }
    }
    
    // Установка флага faststart для mp4
    if (strstr(pctx->ofmt_ctx->oformat->name, "mp4") || strstr(pctx->ofmt_ctx->oformat->name, "mov")) {
        av_opt_set(pctx->ofmt_ctx->priv_data, "movflags", "faststart", 0);
    }

    if ((ret = avformat_write_header(pctx->ofmt_ctx, NULL)) < 0) {
        fprintf(stderr, "[burn] Ошибка записи заголовка: %d (%s)\n", ret, av_err2str(ret));
        goto cleanup;
    }
    
    //av_dump_format(pctx->ofmt_ctx, 0, output_video, 1); // Для отладки

    // 5. Основной цикл обработки (чтение, декодирование, фильтрация, кодирование, запись)
    AVPacket *input_packet = av_packet_alloc();
    if (!input_packet) { ret = AVERROR(ENOMEM); goto cleanup; }
    
    pctx->decoded_frame = av_frame_alloc();
    pctx->filtered_frame = av_frame_alloc();
    pctx->encoded_packet = av_packet_alloc();
    if (!pctx->decoded_frame || !pctx->filtered_frame || !pctx->encoded_packet) {
        av_packet_free(&input_packet);
        ret = AVERROR(ENOMEM); goto cleanup;
    }

    while (av_read_frame(pctx->ifmt_ctx_video, input_packet) >= 0) {
        if (input_packet->stream_index == pctx->video_stream_idx_in && pctx->video_dec_ctx) {
            ret = avcodec_send_packet(pctx->video_dec_ctx, input_packet);
            if (ret < 0) { fprintf(stderr, "[burn] Ошибка avcodec_send_packet (video): %d (%s)\n", ret, av_err2str(ret)); break; }

            while (ret >= 0) {
                ret = avcodec_receive_frame(pctx->video_dec_ctx, pctx->decoded_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                if (ret < 0) { fprintf(stderr, "Ошибка получения кадра от видео декодера\n"); goto loop_end; }
                
                // Отправляем кадр в фильтр
                if ((ret = av_buffersrc_add_frame_flags(pctx->buffersrc_ctx, pctx->decoded_frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
                    fprintf(stderr, "[burn] Ошибка av_buffersrc_add_frame_flags: %d (%s)\n", ret, av_err2str(ret));
                    av_frame_unref(pctx->decoded_frame);
                    goto loop_end;
                }
                av_frame_unref(pctx->decoded_frame); // add_frame_flags берет владение или копирует

                // Получаем отфильтрованный кадр
                while (1) {
                    ret = av_buffersink_get_frame(pctx->buffersink_ctx, pctx->filtered_frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    if (ret < 0) { fprintf(stderr, "Ошибка получения кадра из графа фильтров\n"); goto loop_end; }
                    
                    // Кодируем и пишем отфильтрованный кадр
                    if (encode_write_frame(pctx, pctx->filtered_frame, pctx->video_stream_idx_out) < 0) {
                         fprintf(stderr, "[burn] Ошибка encode_write_frame (video)\n");
                         av_frame_unref(pctx->filtered_frame);
                         goto loop_end;
                    }
                    av_frame_unref(pctx->filtered_frame);
                }
            }
        } else if (input_packet->stream_index == pctx->audio_stream_idx_in && pctx->audio_dec_ctx && pctx->audio_enc_ctx) {
            ret = avcodec_send_packet(pctx->audio_dec_ctx, input_packet);
            if (ret < 0) { fprintf(stderr, "[burn] Ошибка avcodec_send_packet (audio): %d (%s)\n", ret, av_err2str(ret)); break; }
            
            while (ret >= 0) {
                ret = avcodec_receive_frame(pctx->audio_dec_ctx, pctx->decoded_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                if (ret < 0) { fprintf(stderr, "Ошибка получения кадра от аудио декодера\n"); goto loop_end; }
                
                // Здесь может потребоваться ресемплинг, если форматы не совпадают.
                // Для простоты, предполагаем, что энкодер может принять формат декодера.
                // Если нет, нужен будет AVAudioFifo и/или swr_convert.
                if (encode_write_frame(pctx, pctx->decoded_frame, pctx->audio_stream_idx_out) < 0) {
                     fprintf(stderr, "[burn] Ошибка encode_write_frame (audio)\n");
                     av_frame_unref(pctx->decoded_frame);
                     goto loop_end;
                }
                av_frame_unref(pctx->decoded_frame);
            }
        }
        av_packet_unref(input_packet);
    }
loop_end:
    av_packet_free(&input_packet);

    // Промывка энкодеров
    if (pctx->video_enc_ctx) {
        int flush_ret = encode_write_frame(pctx, NULL, pctx->video_stream_idx_out);
        if (flush_ret < 0) fprintf(stderr, "[burn] Ошибка encode_write_frame (video flush): %d\n", flush_ret);
    }
    if (pctx->audio_enc_ctx) {
        int flush_ret = encode_write_frame(pctx, NULL, pctx->audio_stream_idx_out);
        if (flush_ret < 0) fprintf(stderr, "[burn] Ошибка encode_write_frame (audio flush): %d\n", flush_ret);
    }

    ret = av_write_trailer(pctx->ofmt_ctx);
    if (ret < 0) fprintf(stderr, "[burn] Ошибка av_write_trailer: %d (%s)\n", ret, av_err2str(ret));

    // После завершения прожига удаляем временный файл
    unlink(temp_ass_path);

cleanup:
    cleanup_processing_context(pctx);
    
    // subs_format не используется в этой реализации, т.к. фильтр subtitles обычно автоопределяет формат
    (void)subs_format; 

    return (ret < 0 && ret != AVERROR_EOF) ? -1 : 0; // Возвращаем -1 при ошибке, 0 при успехе
}

// Вспомогательные функции для инициализации, если потребуются (не реализованы):
// static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
// static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, const AVChannelLayout* channel_layout, int sample_rate, int nb_samples);

/* 
Пример компиляции (может потребовать корректировки путей и библиотек):
gcc VTL_sub_burn.c -o VTL_sub_burn_test \\
    -I/path/to/ffmpeg_install/include \\
    -L/path/to/ffmpeg_install/lib \\
    -lavformat -lavcodec -lavfilter -lavutil -lswscale -lswresample -lm -pthread 
*/

// int main_test_example() {
//     // Пример вызова
//     // Не забудьте инициализировать ffmpeg (один раз для всего приложения)
//     // av_register_all(); // Устарело
//     // avformat_network_init(); // Если нужны сетевые протоколы
// 
//     int result = VTL_sub_burn_to_video("input.mp4", "subs.srt", VTL_sub_format_kSRT, "output_burned.mp4");
//     if (result == 0) {
//         printf("Субтитры успешно встроены.\n");
//     } else {
//         printf("Ошибка встраивания субтитров: %d\n", result);
//     }
// 
//     // avformat_network_deinit(); // Если был init
//     return result;
// }

