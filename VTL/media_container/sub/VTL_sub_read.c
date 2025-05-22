#include "VTL_sub_read.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VTL_sub_data.h"

struct VTL_SubReadSource {
    VTL_SubList* p_list;
    size_t current_index;
    VTL_SubReadMeta meta;
};

VTL_AppResult VTL_sub_read_OpenSource(const char* file_path, VTL_SubReadSource** pp_source) {
    if (!file_path || !pp_source) return VTL_res_kNullArgument;
    *pp_source = NULL;
    VTL_SubList* p_list = NULL;
    VTL_sub_Format format = VTL_sub_format_kUNKNOWN;
    VTL_AppResult res = VTL_sub_LoadFromFile(file_path, &format, &p_list);
    if (res != VTL_res_kOk) return res;
    VTL_SubReadSource* src = (VTL_SubReadSource*)malloc(sizeof(VTL_SubReadSource));
    if (!src) {
        VTL_sub_ListDestroy(&p_list);
        return VTL_res_kAllocError;
    }
    src->p_list = p_list;
    src->current_index = 0;
    src->meta.format = format;
    src->meta.entry_count = p_list ? p_list->count : 0;
    *pp_source = src;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_read_CloseSource(VTL_SubReadSource** pp_source) {
    if (!pp_source || !*pp_source) return VTL_res_kNullArgument;
    VTL_SubReadSource* src = *pp_source;
    if (src->p_list) VTL_sub_ListDestroy(&src->p_list);
    free(src);
    *pp_source = NULL;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_read_ReadMetaData(VTL_SubReadSource* p_source, VTL_SubReadMeta* p_meta) {
    if (!p_source || !p_meta) return VTL_res_kNullArgument;
    *p_meta = p_source->meta;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_read_ReadPart(VTL_SubReadSource* p_source, VTL_SubEntry* p_entry) {
    if (!p_source || !p_entry) return VTL_res_kNullArgument;
    if (!p_source->p_list || p_source->current_index >= p_source->p_list->count) return VTL_res_kEndOfFile;
    VTL_SubEntry* src_entry = &p_source->p_list->entries[p_source->current_index++];
    *p_entry = *src_entry;
    // Глубокое копирование строк
    if (src_entry->text) p_entry->text = strdup(src_entry->text);
    if (src_entry->style) p_entry->style = strdup(src_entry->style);
    return VTL_res_kOk;
} 