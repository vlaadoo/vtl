#ifndef _VTL_SUB_CONVERT_H
#define _VTL_SUB_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <VTL/VTL_app_result.h> // Для VTL_AppResult
#include <VTL/media_container/sub/VTL_sub_data.h>   // Для VTL_sub_Format
#include <VTL/media_container/sub/VTL_sub_style.h>  // Для VTL_sub_StyleParams

// Конвертирует входной файл субтитров в выходной файл субтитров, применяя указанные параметры стиля.
// Если параметры стиля (style_params) равны NULL, будут использованы стили по умолчанию для целевого формата.
// Функция полностью управляет парсингом входного файла, генерацией выходного файла и освобождением памяти.
VTL_AppResult VTL_sub_ConvertWithStyle(const char* input_file, VTL_sub_Format input_format, 
                                       const char* output_file, VTL_sub_Format output_format, 
                                       const VTL_sub_StyleParams* style_params);

#ifdef __cplusplus
}
#endif

#endif // _VTL_SUB_CONVERT_H 