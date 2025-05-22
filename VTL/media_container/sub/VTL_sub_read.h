#ifndef VTL_SUB_READ_H
#define VTL_SUB_READ_H

#include "VTL_sub_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VTL_SubReadSource VTL_SubReadSource;

typedef struct VTL_SubReadMeta {
    VTL_sub_Format format;
    size_t entry_count;
    // Можно добавить язык, кодировку и др. при необходимости
} VTL_SubReadMeta;

// Открыть источник субтитров для чтения
VTL_AppResult VTL_sub_read_OpenSource(const char* file_path, VTL_SubReadSource** pp_source);
// Закрыть источник
VTL_AppResult VTL_sub_read_CloseSource(VTL_SubReadSource** pp_source);
// Прочитать метаданные (формат, количество реплик и т.д.)
VTL_AppResult VTL_sub_read_ReadMetaData(VTL_SubReadSource* p_source, VTL_SubReadMeta* p_meta);
// Прочитать одну запись (реплику) субтитров (итеративно)
VTL_AppResult VTL_sub_read_ReadPart(VTL_SubReadSource* p_source, VTL_SubEntry* p_entry);

#ifdef __cplusplus
}
#endif

#endif // VTL_SUB_READ_H 