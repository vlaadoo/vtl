#include "VTL_sub_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VTL_sub_data.h"
#include "VTL_sub_style.h"

struct VTL_SubWriteSink {
    VTL_SubList* p_list;
    VTL_SubWriteMeta meta;
    char* file_path;
};

VTL_AppResult VTL_sub_write_OpenOutput(const char* file_path, const VTL_SubWriteMeta* p_meta, VTL_SubWriteSink** pp_sink) {
    if (!file_path || !p_meta || !pp_sink) return VTL_res_kNullArgument;
    *pp_sink = NULL;
    VTL_SubWriteSink* sink = (VTL_SubWriteSink*)malloc(sizeof(VTL_SubWriteSink));
    if (!sink) return VTL_res_kAllocError;
    sink->p_list = NULL;
    VTL_sub_ListCreate(&sink->p_list);
    sink->meta = *p_meta;
    sink->file_path = strdup(file_path);
    if (!sink->file_path) { free(sink); return VTL_res_kAllocError; }
    *pp_sink = sink;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_write_WritePart(VTL_SubWriteSink* p_sink, const VTL_SubEntry* p_entry) {
    if (!p_sink || !p_entry) return VTL_res_kNullArgument;
    return VTL_sub_ListAddEntry(p_sink->p_list, p_entry);
}

VTL_AppResult VTL_sub_write_CloseOutput(VTL_SubWriteSink** pp_sink, const VTL_SubStyleParams* style_params) {
    if (!pp_sink || !*pp_sink) return VTL_res_kNullArgument;
    VTL_SubWriteSink* sink = *pp_sink;
    if (sink->p_list && sink->file_path) {
        VTL_sub_SaveToFile(sink->file_path, sink->meta.format, sink->p_list, style_params);
    }
    if (sink->p_list) VTL_sub_ListDestroy(&sink->p_list);
    if (sink->file_path) free(sink->file_path);
    free(sink);
    *pp_sink = NULL;
    return VTL_res_kOk;
} 