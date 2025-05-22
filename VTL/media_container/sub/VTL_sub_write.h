#ifndef VTL_SUB_WRITE_H
#define VTL_SUB_WRITE_H

#include "VTL_sub_data.h"
#include "VTL_sub_style.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VTL_SubWriteSink VTL_SubWriteSink;

typedef struct VTL_SubWriteMeta {
    VTL_sub_Format format;
    // Можно добавить язык, кодировку и др. при необходимости
} VTL_SubWriteMeta;

// Открыть приёмник субтитров для записи
VTL_AppResult VTL_sub_write_OpenOutput(const char* file_path, const VTL_SubWriteMeta* p_meta, VTL_SubWriteSink** pp_sink);
// Записать одну запись (реплику) субтитров
VTL_AppResult VTL_sub_write_WritePart(VTL_SubWriteSink* p_sink, const VTL_SubEntry* p_entry);
// Закрыть приёмник
VTL_AppResult VTL_sub_write_CloseOutput(VTL_SubWriteSink** pp_sink, const VTL_SubStyleParams* style_params);

#ifdef __cplusplus
}
#endif

#endif // VTL_SUB_WRITE_H 