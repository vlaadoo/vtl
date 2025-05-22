#ifndef _VTL_SUB_DATA_H
#define _VTL_SUB_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <VTL/utils/VTL_buffer_data.h>
#include <VTL/utils/VTL_string.h>
#include <VTL/VTL_app_result.h>
#include <stdint.h>
#include <stddef.h>
#include "VTL_sub_style.h"

typedef enum _VTL_sub_Format
{
    VTL_sub_format_kASS = 0,
    VTL_sub_format_kSRT,
    VTL_sub_format_kVTT,
    VTL_sub_format_kUNKNOWN
} VTL_sub_Format;

typedef enum _VTL_sub_HorizontalAlign
{
    VTL_sub_horizontal_align_kLeft = 0,
    VTL_sub_format_kCenter,
    VTL_sub_format_kRight
} VTL_sub_HorizontalAlign;

typedef VTL_StandartString VTL_sub_FontName;
typedef uint_least8_t VTL_sub_Size;
typedef uint_least8_t VTL_sub_ColorPart;

typedef struct _VTL_sub_color_RGB
{
    VTL_sub_ColorPart r;
    VTL_sub_ColorPart g;
    VTL_sub_ColorPart b;
} VTL_sub_color_RGB;

typedef struct _VTL_sub_Params
{
    VTL_sub_HorizontalAlign horizontal_align;
    VTL_sub_color_RGB color;    
    VTL_sub_color_RGB background_color;    
    VTL_sub_color_RGB outline_color;
    VTL_sub_Size text_size;
    VTL_sub_Size margin;
    VTL_sub_FontName font_name;
} VTL_sub_Params;

typedef struct _VTL_sub_MetaData
{
    size_t data_size;    
    VTL_sub_Params params;
} VTL_sub_MetaData;

typedef char VTL_sub_Symbol;
typedef struct _VTL_sub_Str
{
    VTL_sub_Symbol* p_data;
    size_t length;
} VTL_sub_Str;

typedef struct _VTL_Sub
{
    VTL_sub_MetaData meta_data;
    VTL_BufferData* data;
    size_t length;
} VTL_Sub;

// Универсальное представление одного субтитра (для любого формата)
typedef struct _VTL_SubEntry {
    int index;    // номер субтитра (для SRT обязателен, для VTT/ASS может быть идентификатором или отсутствовать)
    double start; // время начала отображения субтитра в секундах
    double end;   // время конца отображения субтитра в секундах
    char* text;   // текст субтитра (для ASS - очищенный от тегов)
    char* style;  // информация о стиле (для ASS - имя стиля, для VTT - идентификатор реплики, для SRT - обычно NULL)
} VTL_SubEntry;

// Массив записей субтитров (универсальный для SRT/ASS/VTT)
typedef struct _VTL_SubList {
    VTL_SubEntry* entries; // указатель на массив записей
    size_t count;          // количество записей в массиве
} VTL_SubList;

// Функции для VTL_sub_Params
VTL_AppResult VTL_sub_ParamsSetHorizontalAlign(VTL_sub_Params* p_params, VTL_sub_HorizontalAlign align);
VTL_AppResult VTL_sub_ParamsSetColor(VTL_sub_Params* p_params, VTL_sub_color_RGB color);
VTL_AppResult VTL_sub_ParamsSetBackgroundColor(VTL_sub_Params* p_params, VTL_sub_color_RGB background_color);
VTL_AppResult VTL_sub_ParamsSetOutlineColor(VTL_sub_Params* p_params, VTL_sub_color_RGB outline_color);
VTL_AppResult VTL_sub_ParamsSetTextSize(VTL_sub_Params* p_params, VTL_sub_Size text_size);
VTL_AppResult VTL_sub_ParamsSetMargin(VTL_sub_Params* p_params, VTL_sub_Size margin);
VTL_AppResult VTL_sub_ParamsSetFontName(VTL_sub_Params* p_params, VTL_sub_FontName font_name);

// Функции для VTL_SubList
VTL_AppResult VTL_sub_ListCreate(VTL_SubList** pp_sub_list);
VTL_AppResult VTL_sub_ListDestroy(VTL_SubList** pp_sub_list);
VTL_AppResult VTL_sub_ListAddEntry(VTL_SubList* p_sub_list, const VTL_SubEntry* p_entry);
VTL_AppResult VTL_sub_ListGetEntry(const VTL_SubList* p_sub_list, size_t index, VTL_SubEntry** pp_entry);
VTL_AppResult VTL_sub_ListRemoveEntry(VTL_SubList* p_sub_list, size_t index);

// Функции для парсинга и форматирования субтитров
VTL_AppResult VTL_sub_Parse(VTL_BufferData* p_buffer_data, VTL_sub_Format format, VTL_SubList** pp_sub_list);
VTL_AppResult VTL_sub_FormatToString(const VTL_SubList* p_sub_list, VTL_sub_Format format, VTL_BufferData** pp_buffer_data, const VTL_SubStyleParams* style_params);

// Функции для загрузки и сохранения списка субтитров из/в файл
// Пытается определить формат файла по его расширению. 
// Если file_path равен NULL или не удалось определить формат, возвращает ошибку.
// detected_format (выходной) - сюда будет записан определенный формат файла.
VTL_AppResult VTL_sub_LoadFromFile(const char* file_path, VTL_sub_Format* detected_format, VTL_SubList** pp_sub_list);

// Сохраняет список субтитров в файл в указанном формате.
// Если format равен VTL_sub_format_kUNKNOWN, пытается определить формат по расширению file_path.
// Если p_sub_list равен NULL или file_path равен NULL, возвращает ошибку.
VTL_AppResult VTL_sub_SaveToFile(const char* file_path, VTL_sub_Format format, const VTL_SubList* p_sub_list, const VTL_SubStyleParams* style_params);

// Вспомогательная функция для дублирования строки или возврата NULL, если src равен NULL
char* VTL_sub_StrdupNullable(const char* src);

#ifdef __cplusplus
}
#endif


#endif