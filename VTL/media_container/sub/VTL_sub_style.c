#include <VTL/media_container/sub/VTL_sub_style.h>
#include <external/parson/parson.h>
#include <stdlib.h> // Для strtoul
#include <string.h> // Для strdup

// Загрузка параметров оформления субтитров из JSON-файла
int VTL_sub_StyleLoadFromJson(const char* json_file, VTL_sub_StyleParams* style_params) {
    JSON_Value* root = json_parse_file(json_file);
    if (!root) return -1;
    JSON_Object* obj = json_value_get_object(root);
    if (!obj) { json_value_free(root); return -2; }
    style_params->fontname = strdup(json_object_get_string(obj, "fontname"));
    style_params->fontsize = (int)json_object_get_number(obj, "fontsize");
    style_params->primary_color = (uint32_t)strtoul(json_object_get_string(obj, "primary_color"), NULL, 0);
    style_params->back_color = (uint32_t)strtoul(json_object_get_string(obj, "back_color"), NULL, 0);
    style_params->outline_color = (uint32_t)strtoul(json_object_get_string(obj, "outline_color"), NULL, 0);
    style_params->outline = (int)json_object_get_number(obj, "outline");
    style_params->border_style = (int)json_object_get_number(obj, "border_style");
    style_params->bold = (int)json_object_get_number(obj, "bold");
    style_params->italic = (int)json_object_get_number(obj, "italic");
    style_params->underline = (int)json_object_get_number(obj, "underline");
    style_params->alignment = (int)json_object_get_number(obj, "alignment");
    style_params->margin_l = (int)json_object_get_number(obj, "margin_l");
    style_params->margin_r = (int)json_object_get_number(obj, "margin_r");
    style_params->margin_v = (int)json_object_get_number(obj, "margin_v");
    json_value_free(root);
    return 0;
} 