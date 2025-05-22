#include <VTL/media_container/sub/VTL_sub_convert.h>
#include <VTL/media_container/sub/VTL_sub_style.h>
#include <VTL/media_container/sub/VTL_sub_burn.h>
#include <stdio.h>

int main(void)
{
    VTL_SubStyleParams style_params;
    if (VTL_sub_style_load_from_json("VTL/test/data/subtitle_style.json", &style_params) != 0) {
        fprintf(stderr, "Ошибка загрузки JSON стиля\\n");
        return 1;
    }
    // 1. Конвертация SRT -> ASS с применением стиля
    VTL_AppResult result = VTL_sub_convert_with_style(
        "VTL/test/data/input.srt", VTL_sub_format_kSRT,
        "VTL/test/data/output.ass", VTL_sub_format_kASS,
        &style_params);
    if (result == VTL_res_kOk) {
        printf("Subtitle conversion with style from JSON success!\n");
        // 2. Прожиг ASS в видео
        int burn_res = VTL_sub_burn_to_video(
            "VTL/test/data/input.mp4",
            "VTL/test/data/output.ass",
            VTL_sub_format_kASS,
            "VTL/test/data/output_burned.mp4",
            &style_params
        );
        if (burn_res == 0) {
            printf("Burn-in subtitles to video success!\n");
        } else {
            printf("Burn-in subtitles to video failed: %d\n", burn_res);
        }
        // 2. Конвертация ASS -> VTT с дефолтным стилем (style_params = NULL)
        result = VTL_sub_convert_with_style(
            "VTL/test/data/output.ass", VTL_sub_format_kASS,
            "VTL/test/data/output.vtt", VTL_sub_format_kVTT,
            NULL
        );
        if (result == VTL_res_kOk) {
            printf("Subtitle conversion to VTT success!\n");
        } else {
            printf("Subtitle conversion to VTT failed: %d\n", result);
        }
        // 3. Конвертация VTT -> ASS со стилем из JSON
        result = VTL_sub_convert_with_style(
            "VTL/test/data/output.vtt", VTL_sub_format_kVTT,
            "VTL/test/data/output.ass", VTL_sub_format_kASS,
            &style_params
        );
        if (result == VTL_res_kOk) {
            printf("Subtitle conversion to ASS success!\n");
        } else {
            printf("Subtitle conversion to ASS failed: %d\n", result);
        }
    } else {
        printf("Subtitle conversion failed: %d\n", result);
    }
    return 0;
} 