#include <VTL/media_container/audio/infra/VTL_audio_read.h>

VTL_AppResult VTL_audio_read_OpenSource(VTL_audio_File** pp_audio_file, const VTL_Filename srt_file_name)
{
    return VTL_res_kOk;
}

VTL_AppResult VTL_audio_read_CloseSource(VTL_audio_File* p_audio_file)
{
    return VTL_res_kOk;
}

VTL_AppResult VTL_audio_ReadMetaData(VTL_audio_MetaData* p_audio_meta_data, VTL_audio_File* p_audio_file,
                                            VTL_audio_file_op_Result* p_file_op_result)
{
    return VTL_res_kOk;
}

VTL_AppResult VTL_audio_ReadPart(VTL_audio_Data** pp_audio_part, const VTL_audio_MetaData* p_audio_meta_data, 
                                    VTL_audio_File* p_audio_file, VTL_audio_file_op_Result* p_file_op_result)
{
    return VTL_res_kOk;
}

VTL_AppResult VTL_audio_meta_data_InitFromSource(VTL_audio_MetaData* p_audio_meta_data, 
                                                    const VTL_Filename src_file_name)
{
    // Добавлена инициализация дефолтных параметров аудио для предотвращения segfault
    if (p_audio_meta_data) {
        p_audio_meta_data->data_size = sizeof(VTL_audio_MetaData);
        p_audio_meta_data->params.bitrate = 128000;
        p_audio_meta_data->params.codec = VTL_audio_codec_kAAC;
        p_audio_meta_data->params.sample_rate = 44100;
        p_audio_meta_data->params.num_channels = 2;
        p_audio_meta_data->params.volume = 100;
    }
    VTL_audio_File* p_audio_r_file = {0};
    VTL_audio_file_op_Result audio_r_op_res;
    VTL_audio_read_OpenSource(&p_audio_r_file, src_file_name);
    VTL_audio_ReadMetaData(p_audio_meta_data, p_audio_r_file, &audio_r_op_res);
    VTL_audio_read_CloseSource(p_audio_r_file);
    return VTL_res_kOk;
}
