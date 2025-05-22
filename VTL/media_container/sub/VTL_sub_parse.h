#ifndef _VTL_SUB_PARSE_H
#define _VTL_SUB_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <VTL/VTL_app_result.h>
#include <VTL/media_container/sub/VTL_sub_data.h> // For VTL_sub_Format, VTL_SubList

// Detects subtitle format based on file extension.
VTL_sub_Format VTL_sub_detect_format(const char* filename);

// Parses a subtitle file into a VTL_SubList structure.
VTL_AppResult VTL_sub_ParseFile(const char* input_file, VTL_sub_Format input_format, VTL_SubList* out_list);

// Frees the memory allocated for a VTL_SubList and its entries.
void VTL_sub_ListFree(VTL_SubList* list);

#ifdef __cplusplus
}
#endif

#endif // _VTL_SUB_PARSE_H 