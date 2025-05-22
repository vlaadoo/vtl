#include "VTL_sub_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Получить расширение файла
static const char* VTL_sub_getFileExt(const char* filename) {
    const char* dot = strrchr(filename, '.');
    return dot ? dot + 1 : "";
}

// Определить формат субтитров по расширению
VTL_sub_Format VTL_sub_detectFormat(const char* filename) {
    const char* ext = VTL_sub_getFileExt(filename);
    if (strcasecmp(ext, "srt") == 0) return VTL_sub_format_kSRT;
    if (strcasecmp(ext, "ass") == 0) return VTL_sub_format_kASS;
    if (strcasecmp(ext, "vtt") == 0) return VTL_sub_format_kVTT;
    return VTL_sub_format_kUNKNOWN;
}

// Очистка памяти списка субтитров
void VTL_sub_ListFree(VTL_SubList* list) {
    if (!list || !list->entries) return;
    for (size_t i = 0; i < list->count; ++i) {
        free(list->entries[i].text);
        free(list->entries[i].style);
    }
    free(list->entries);
    list->entries = NULL;
    list->count = 0;
}

// Парсер времени
static double VTL_sub_ParseTime(const char* str, VTL_sub_Format format) {
    int h = 0, m = 0, s = 0, frac = 0;
    if (format == VTL_sub_format_kSRT) {
        if (sscanf(str, "%d:%d:%d,%d", &h, &m, &s, &frac) == 4)
            return h * 3600.0 + m * 60.0 + s + frac / 1000.0;
    } else if (format == VTL_sub_format_kVTT) {
        if (sscanf(str, "%d:%d:%d.%d", &h, &m, &s, &frac) == 4) // VTT использует точку для миллисекунд
            return h * 3600.0 + m * 60.0 + s + frac / 1000.0;
    } else if (format == VTL_sub_format_kASS) {
        if (sscanf(str, "%d:%d:%d.%d", &h, &m, &s, &frac) == 4) // ASS использует точку для сантисекунд (сотых долей)
            return h * 3600.0 + m * 60.0 + s + frac / 100.0; // Делим на 100
    }
    return 0.0;
}

// Вспомогательная функция для изменения размера массива субтитров
static VTL_AppResult VTL_sub_resizeSubEntryArray(VTL_SubEntry** arr_ptr, size_t* capacity_ptr, size_t current_len) {
    if (!arr_ptr || !capacity_ptr) return VTL_res_kArgumentError; // Проверяем указатели
    // Если *arr_ptr это NULL, это может быть начальное выделение, current_len должен быть 0
    if (!*arr_ptr && current_len != 0) return VTL_res_kArgumentError; 
    if (*arr_ptr && current_len == 0) { /* Это может быть ок, если просто уменьшаем capacity до 0 */ }

    if (current_len < *capacity_ptr && *arr_ptr != NULL) { // Добавил проверку *arr_ptr != NULL
        return VTL_res_kOk; // Вместимости достаточно
    }
    size_t new_capacity = *capacity_ptr ? (*capacity_ptr) * 2 : 16;
    VTL_SubEntry* temp_arr = realloc(*arr_ptr, new_capacity * sizeof(VTL_SubEntry));
    if (!temp_arr) {
        // Вызывающая функция должна будет сама освободить *arr_ptr, если он не NULL, перед возвратом ошибки.
        // Это важно, так как VTL_sub_ListFree ожидает VTL_SubList.
        return VTL_res_kMemoryError;
    }
    *arr_ptr = temp_arr;
    *capacity_ptr = new_capacity;
    return VTL_res_kOk;
}

// Вспомогательная функция для чтения текстового блока субтитра
static void VTL_sub_readSubtitleTextBlock(FILE* f, char* textbuf, size_t buf_size) {
    char line_reader[1024]; // Буфер для чтения строк из файла
    char* p = textbuf;
    *p = '\0'; // Инициализация textbuf пустой строкой

    while (fgets(line_reader, sizeof(line_reader), f)) {
        if (line_reader[0] == '\n' || line_reader[0] == '\r') { // Пустая строка означает конец блока
            break;
        }
        size_t line_len = strlen(line_reader);
        // Проверка на переполнение textbuf, оставляем место для null-терминатора
        if ((p + line_len) >= (textbuf + buf_size - 1)) {
            size_t remaining_space = (textbuf + buf_size - 1) - p;
            if (remaining_space > 0) {
                memcpy(p, line_reader, remaining_space);
                p += remaining_space;
            }
            break; // Буфер заполнен
        }
        memcpy(p, line_reader, line_len);
        p += line_len;
    }
    // Удаляем возможные последние переносы строки из собранного textbuf
    while (p > textbuf && (*(p - 1) == '\n' || *(p - 1) == '\r')) {
        p--;
    }
    *p = '\0'; // Завершаем строку текста
}

// Парсинг субтитров из файла в список
VTL_AppResult VTL_sub_ParseFile(const char* input_file, VTL_sub_Format input_format, VTL_SubList* out_list) {
    if (!out_list) return VTL_res_kArgumentError;
    out_list->entries = NULL;
    out_list->count = 0;

    FILE* f = fopen(input_file, "r");
    if (!f) return VTL_res_video_fs_r_kMissingFileErr;

    VTL_SubEntry* arr = NULL;
    size_t arr_cap = 0;
    size_t arr_len = 0;
    char line_buffer[1024]; // Общий буфер для чтения строк
    char text_buffer[2048]; // Общий буфер для текста субтитра

    VTL_AppResult res = VTL_res_kOk;

    if (input_format == VTL_sub_format_kSRT) {
        while (fgets(line_buffer, sizeof(line_buffer), f)) {
            // Пропускаем пустые строки между блоками субтитров
            if (line_buffer[0] == '\n' || line_buffer[0] == '\r') continue;
            
            int idx = atoi(line_buffer);
            if (idx == 0 && arr_len > 0) { 
                 // Невалидный индекс после уже прочитанных субтитров или конец файла с мусором
                 continue; 
            }

            if (!fgets(line_buffer, sizeof(line_buffer), f)) { res = VTL_res_kParseError; break; } // Строка времени
            char* arrow = strstr(line_buffer, "-->");
            if (!arrow) { res = VTL_res_kParseError; break; } // Некорректный формат времени
            *arrow = '\0';
            double start = VTL_sub_ParseTime(line_buffer, input_format);
            double end = VTL_sub_ParseTime(arrow + 3, input_format);

            VTL_sub_readSubtitleTextBlock(f, text_buffer, sizeof(text_buffer));
            
            res = VTL_sub_resizeSubEntryArray(&arr, &arr_cap, arr_len);
            if (res != VTL_res_kOk) break;

            arr[arr_len].index = idx;
            arr[arr_len].start = start;
            arr[arr_len].end = end;
            arr[arr_len].text = strdup(text_buffer);
            if (!arr[arr_len].text) { res = VTL_res_kMemoryError; break; }
            arr[arr_len].style = NULL;
            arr_len++;
        }
    } else if (input_format == VTL_sub_format_kVTT) {
        // Пропускаем заголовок WEBVTT и возможные пустые строки/комментарии до него
        while (fgets(line_buffer, sizeof(line_buffer), f)) {
            if (strstr(line_buffer, "WEBVTT")) break;
        }

        int current_vtt_index = 1;
        while (fgets(line_buffer, sizeof(line_buffer), f)) { // Читаем идентификатор или таймкод
            // Пропускаем комментарии NOTE и пустые строки
            if (strncmp(line_buffer, "NOTE", 4) == 0 || line_buffer[0] == '\n' || line_buffer[0] == '\r') continue;

            char* arrow = strstr(line_buffer, "-->");
            if (!arrow) { // Если не таймкод, значит это идентификатор, читаем следующую строку для таймкода
                if (!fgets(line_buffer, sizeof(line_buffer), f)) { res = VTL_res_kParseError; break; }
                arrow = strstr(line_buffer, "-->");
                if (!arrow) { continue; } // Пропускаем блок, если опять не таймкод
            }
            *arrow = '\0';
            double start = VTL_sub_ParseTime(line_buffer, input_format);
            double end = VTL_sub_ParseTime(arrow + 3, input_format);

            VTL_sub_readSubtitleTextBlock(f, text_buffer, sizeof(text_buffer));

            res = VTL_sub_resizeSubEntryArray(&arr, &arr_cap, arr_len);
            if (res != VTL_res_kOk) break;
            
            arr[arr_len].index = current_vtt_index++;
            arr[arr_len].start = start;
            arr[arr_len].end = end;
            arr[arr_len].text = strdup(text_buffer);
            if (!arr[arr_len].text) { res = VTL_res_kMemoryError; break; }
            arr[arr_len].style = NULL; 
            arr_len++;
        }
    } else if (input_format == VTL_sub_format_kASS) {
        // Пропускаем до секции [Events]
        while (fgets(line_buffer, sizeof(line_buffer), f)) {
            if (strstr(line_buffer, "[Events]")) break;
        }
        // Пропускаем строку "Format: ..."
        if (!fgets(line_buffer, sizeof(line_buffer), f)) {
             // Если после [Events] сразу конец файла или нет Format строки
        }

        int current_ass_index = 1;
        while (fgets(line_buffer, sizeof(line_buffer), f)) {
            if (strncmp(line_buffer, "Dialogue:", 9) != 0) continue;
            
            char* p_line = line_buffer + 9; // Пропускаем "Dialogue: "
            char* fields[10] = {0}; // Layer,Start,End,Style,Name,MarginL,MarginR,MarginV,Effect,Text
            int field_count = 0;
            char* token = p_line;
            
            // Разбор полей ASS с учетом того, что последнее поле (Text) идет до конца строки
            // и может содержать запятые.
            for (int i = 0; i < 9 && token; ++i) { // Первые 9 полей разделены запятыми
                fields[field_count++] = token;
                char* comma = strchr(token, ',');
                if (comma) {
                    *comma = '\0';
                    token = comma + 1;
                } else {
                    token = NULL; // Больше нет запятых, это может быть ошибка или конец строки раньше времени
                }
            }
            if (token) { // Все что осталось - это текстовое поле
                 fields[field_count++] = token;
                 // Удаляем перенос строки с конца, если есть
                 char* nl = strchr(token, '\n'); if (nl) *nl = '\0';
                 nl = strchr(token, '\r'); if (nl) *nl = '\0';
            }

            if (field_count < 10) continue; // Ожидаем как минимум 10 полей (0-9)

            if (!fields[1] || !fields[2] || !fields[3] || !fields[9]) continue;

            double start = VTL_sub_ParseTime(fields[1], input_format);
            double end = VTL_sub_ParseTime(fields[2], input_format);
            
            res = VTL_sub_resizeSubEntryArray(&arr, &arr_cap, arr_len);
            if (res != VTL_res_kOk) break;

            arr[arr_len].index = current_ass_index++;
            arr[arr_len].start = start;
            arr[arr_len].end = end;
            arr[arr_len].text = strdup(fields[9]);
            arr[arr_len].style = strdup(fields[3]);

            if (!arr[arr_len].text || !arr[arr_len].style) {
                free(arr[arr_len].text); // Освобождаем то, что могло выделиться
                free(arr[arr_len].style);
                arr[arr_len].text = NULL; 
                arr[arr_len].style = NULL;
                res = VTL_res_kMemoryError; 
                break;
            }
            arr_len++;
        }
    } else {
        res = VTL_res_kUnsupportedFormat;
    }

    fclose(f);

    if (res != VTL_res_kOk && arr != NULL) {
        // Если произошла ошибка в цикле парсинга (кроме ошибки выделения памяти, обработанной выше),
        // и массив arr был выделен, нужно его очистить.
        VTL_SubList tempList = { .entries = arr, .count = arr_len };
        VTL_sub_ListFree(&tempList); // Это обнулит arr_len и arr внутри tempList
        arr = NULL; // Убедимся, что out_list не получит некорректный указатель
        arr_len = 0;
    }
    
    out_list->entries = arr;
    out_list->count = arr_len;
    
    // Если res был VTL_res_kOk, но arr_len == 0 (например, пустой файл субтитров), это все равно Ok.
    // Если res был ошибкой, он просто вернется.
    return res;
}

// Функция для парсинга субтитров из буфера (VTL_BufferData)
VTL_AppResult VTL_sub_parse(VTL_BufferData* p_buffer_data, VTL_sub_Format format, VTL_SubList** pp_sub_list) {
    // Реализация аналогична тому, что было, но работает с p_buffer_data->data вместо чтения из файла
    // ... (реализация парсинга из буфера, а не из файла) ...
} 