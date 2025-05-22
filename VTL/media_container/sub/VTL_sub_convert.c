#include "VTL_sub_convert.h"
#include "VTL_sub_style.h"
#include "VTL_sub_parse.h"
#include "VTL_sub_read.h"
#include "VTL_sub_write.h"
#include <stdio.h>
#undef strdup
#include "VTL_sub_data.h"

// Форматтер времени
static void format_time(char* buf, size_t bufsz, double t, VTL_sub_Format format) {
    int h = (int)(t / 3600);
    int m = (int)(t / 60) % 60;
    int s = (int)t % 60;
    if (format == VTL_sub_format_kSRT) {
        int ms = (int)((t - (int)t) * 1000);
        snprintf(buf, bufsz, "%02d:%02d:%02d,%03d", h, m, s, ms);
    } else if (format == VTL_sub_format_kVTT) {
        int ms = (int)((t - (int)t) * 1000);
        snprintf(buf, bufsz, "%02d:%02d:%02d.%03d", h, m, s, ms);
    } else if (format == VTL_sub_format_kASS) {
        int cs = (int)((t - (int)t) * 100);
        snprintf(buf, bufsz, "%d:%02d:%02d.%02d", h, m, s, cs);
    }
}

// Преобразование цвета
// color_to_str: преобразует ARGB (0xAARRGGBB) в формат ASS (&HAABBGGRR)
// Для данной libass: Alpha в ASS инвертирована по сравнению с ARGB.
// ASS Alpha 0x00 = непрозрачный (соответствует ARGB Alpha 0xFF)
// ASS Alpha 0xFF = прозрачный   (соответствует ARGB Alpha 0x00)
static void color_to_str(uint32_t argb, char* buf, size_t bufsz, VTL_sub_Format format) {
    if (format == VTL_sub_format_kASS) {
        uint8_t r_argb = (argb >> 16) & 0xFF;
        uint8_t g_argb = (argb >> 8) & 0xFF;
        uint8_t b_argb = argb & 0xFF;
        uint8_t a_argb = (argb >> 24) & 0xFF;

        // Инвертируем альфу для ASS
        uint8_t a_ass = 0xFF - a_argb;

        // Формат ASS &HAABBGGRR
        snprintf(buf, bufsz, "&H%02X%02X%02X%02X",
            a_ass, // AA (инвертированная Alpha)
            b_argb,  // BB (Blue)
            g_argb,  // GG (Green)
            r_argb   // RR (Red)
        );
    } else {
        snprintf(buf, bufsz, "#%02X%02X%02X", (argb>>16)&0xFF, (argb>>8)&0xFF, (argb)&0xFF);
    }
}

// Конвертирует файл субтитров из входного файла в выходной, применяя параметры оформления.
// Если style_params равен NULL, используются стили по умолчанию.
VTL_AppResult VTL_sub_convert_with_style(const char* input_file, VTL_sub_Format input_format, 
                                         const char* output_file, VTL_sub_Format output_format, 
                                         const VTL_SubStyleParams* style_params) {
    VTL_SubReadSource* src = NULL;
    VTL_AppResult res = VTL_sub_read_OpenSource(input_file, &src);
    if (res != VTL_res_kOk) return res;
    VTL_SubReadMeta meta;
    VTL_sub_read_ReadMetaData(src, &meta);

    VTL_SubWriteMeta wmeta = { .format = output_format };
    VTL_SubWriteSink* sink = NULL;
    res = VTL_sub_write_OpenOutput(output_file, &wmeta, &sink);
    if (res != VTL_res_kOk) {
        VTL_sub_read_CloseSource(&src);
        return res;
    }

    VTL_SubEntry entry;
    while (VTL_sub_read_ReadPart(src, &entry) == VTL_res_kOk) {
        // Применяем стиль, если нужно (только для SRT/VTT)
        if (style_params && (output_format == VTL_sub_format_kSRT || output_format == VTL_sub_format_kVTT)) {
            VTL_SubEntry styled_entry = entry;
            char styled_text[2048];
            if (output_format == VTL_sub_format_kSRT) {
                char html_color[16];
                snprintf(html_color, sizeof(html_color), "#%02X%02X%02X",
                    (style_params->primary_color>>16)&0xFF,
                    (style_params->primary_color>>8)&0xFF,
                    (style_params->primary_color)&0xFF);
                snprintf(styled_text, sizeof(styled_text),
                    "<font color=\"%s\" size=\"%d\" face=\"%s\">%s</font>",
                    html_color,
                    style_params->fontsize > 0 ? style_params->fontsize : 24,
                    style_params->fontname ? style_params->fontname : "Arial",
                    entry.text ? entry.text : "");
            } else if (output_format == VTL_sub_format_kVTT) {
                snprintf(styled_text, sizeof(styled_text), "%s", entry.text ? entry.text : "");
            }
            char* old_text = styled_entry.text;
            styled_entry.text = VTL_sub_StrdupNullable(styled_text);
            free(old_text);
            VTL_sub_write_WritePart(sink, &styled_entry);
            free(styled_entry.text);
        } else {
            VTL_sub_write_WritePart(sink, &entry);
        }
        if (entry.text) free(entry.text);
        if (entry.style) free(entry.style);
    }
    VTL_sub_read_CloseSource(&src);
    VTL_sub_write_CloseOutput(&sink, style_params);
    return VTL_res_kOk;
}