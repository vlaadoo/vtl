#include "VTL_sub_opencl.h"
#include <OpenCL/opencl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Заглушка: CPU-реализация массового удаления тегов (реализация OpenCL ниже)
static void strip_tags_cpu(const char** in_texts, char** out_texts, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        const char* src = in_texts[i];
        size_t len = strlen(src);
        char* dst = (char*)malloc(len + 1);
        if (!dst) { out_texts[i] = NULL; continue; }
        size_t j = 0;
        int in_tag = 0;
        for (size_t k = 0; k < len; ++k) {
            if (src[k] == '{') in_tag = 1;
            else if (src[k] == '}' && in_tag) in_tag = 0;
            else if (!in_tag) dst[j++] = src[k];
        }
        dst[j] = '\0';
        out_texts[i] = dst;
    }
}

// Основная функция массовой обработки через OpenCL (пока только CPU fallback)
int VTL_sub_opencl_strip_tags(const char** in_texts, char** out_texts, size_t count) {
    // TODO: Реализация через OpenCL (инициализация, компиляция ядра, запуск)
    // Пока только CPU fallback
    strip_tags_cpu(in_texts, out_texts, count);
    return 0;
} 