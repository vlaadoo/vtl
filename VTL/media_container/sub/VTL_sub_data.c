#include <VTL/media_container/sub/VTL_sub_data.h>
#include <stdlib.h> // Для malloc, free, realloc, strtod
#include <string.h> // Для memcpy, memset, strncpy, strdup, strtok_r, strstr, strchr
#include <stdio.h>  // Для sscanf, snprintf
#include <ctype.h>  // Для isspace
#include <VTL/media_container/sub/VTL_sub_opencl.h>

// Прототипы вспомогательных функций времени
static double VTL_sub_ParseSrtVttTime(const char* time_str);
static double VTL_sub_ParseAssTime(const char* time_str);

// Функции для VTL_sub_Params
VTL_AppResult VTL_sub_ParamsSetHorizontalAlign(VTL_sub_Params* p_params, VTL_sub_HorizontalAlign align)
{
    if (!p_params) return VTL_res_kNullArgument;
    p_params->horizontal_align = align;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ParamsSetColor(VTL_sub_Params* p_params, VTL_sub_color_RGB color)
{
    if (!p_params) return VTL_res_kNullArgument;
    p_params->color = color;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ParamsSetBackgroundColor(VTL_sub_Params* p_params, VTL_sub_color_RGB background_color)
{
    if (!p_params) return VTL_res_kNullArgument;
    p_params->background_color = background_color;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ParamsSetOutlineColor(VTL_sub_Params* p_params, VTL_sub_color_RGB outline_color)
{
    if (!p_params) return VTL_res_kNullArgument;
    p_params->outline_color = outline_color;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ParamsSetTextSize(VTL_sub_Params* p_params, VTL_sub_Size text_size)
{
    if (!p_params) return VTL_res_kNullArgument;
    p_params->text_size = text_size;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ParamsSetMargin(VTL_sub_Params* p_params, VTL_sub_Size margin)
{
    if (!p_params) return VTL_res_kNullArgument;
    p_params->margin = margin;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ParamsSetFontName(VTL_sub_Params* p_params, VTL_sub_FontName font_name)
{
    if (!p_params || !font_name) return VTL_res_kNullArgument;
    
    // VTL_sub_FontName это VTL_StandartString, который является char[VTL_STANDART_STRING_MAX_LENGTH]
    // Используем strncpy для копирования и гарантии null-терминации.
    strncpy(p_params->font_name, font_name, VTL_STANDART_STRING_MAX_LENGTH - 1);
    p_params->font_name[VTL_STANDART_STRING_MAX_LENGTH - 1] = '\0'; // Гарантируем null-терминацию
    
    return VTL_res_kOk;
}

// Функции для VTL_sub_List
VTL_AppResult VTL_sub_ListCreate(VTL_sub_List** pp_sub_list)
{
    if (!pp_sub_list) return VTL_res_kNullArgument;
    *pp_sub_list = (VTL_sub_List*)malloc(sizeof(VTL_sub_List));
    if (!*pp_sub_list) return VTL_res_kAllocError;
    (*pp_sub_list)->entries = NULL;
    (*pp_sub_list)->count = 0;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ListDestroy(VTL_sub_List** pp_sub_list)
{
    if (!pp_sub_list || !*pp_sub_list) return VTL_res_kNullArgument;
    
    for (size_t i = 0; i < (*pp_sub_list)->count; ++i) {
        if ((*pp_sub_list)->entries[i].text) {
            free((*pp_sub_list)->entries[i].text);
        }
        if ((*pp_sub_list)->entries[i].style) {
            free((*pp_sub_list)->entries[i].style);
        }
    }
    free((*pp_sub_list)->entries);
    free(*pp_sub_list);
    *pp_sub_list = NULL;
    return VTL_res_kOk;
}

// Вспомогательная функция для дублирования строки или возврата NULL, если src равен NULL
char* VTL_sub_StrdupNullable(const char* src) {
    if (!src) return NULL;
    char* dst = strdup(src);
    if (!dst) return NULL;
    return dst;
}

VTL_AppResult VTL_sub_ListAddEntry(VTL_sub_List* p_sub_list, const VTL_sub_Entry* p_entry)
{
    if (!p_sub_list || !p_entry) return VTL_res_kNullArgument;

    VTL_sub_Entry* new_entries = (VTL_sub_Entry*)realloc(p_sub_list->entries, (p_sub_list->count + 1) * sizeof(VTL_sub_Entry));
    if (!new_entries) return VTL_res_kAllocError;
    
    p_sub_list->entries = new_entries;
    VTL_sub_Entry* new_item = &p_sub_list->entries[p_sub_list->count];

    // Глубокое копирование записи
    new_item->index = p_entry->index;
    new_item->start = p_entry->start;
    new_item->end = p_entry->end;
    
    new_item->text = VTL_sub_StrdupNullable(p_entry->text);
    if (p_entry->text && !new_item->text) {
        // Не удалось дублировать текст, очищаем здесь не строго необходимо, так как realloc мог переместить new_entries
        // но это хорошая практика, если эта функция была бы сложнее и могла бы завершиться ошибкой между выделениями памяти.
        return VTL_res_kAllocError; 
    }

    new_item->style = VTL_sub_StrdupNullable(p_entry->style);
    if (p_entry->style && !new_item->style) {
        if(new_item->text) free(new_item->text); // Очищаем ранее выделенный текст
        return VTL_res_kAllocError; 
    }
    
    p_sub_list->count++;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ListGetEntry(const VTL_sub_List* p_sub_list, size_t index, VTL_sub_Entry** pp_entry)
{
    if (!p_sub_list || !pp_entry) return VTL_res_kNullArgument;
    if (index >= p_sub_list->count) return VTL_res_kArgumentError;
    
    *pp_entry = (VTL_sub_Entry*)&p_sub_list->entries[index]; // Убираем const для выходного параметра
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_ListRemoveEntry(VTL_sub_List* p_sub_list, size_t index)
{
    if (!p_sub_list) return VTL_res_kNullArgument;
    if (index >= p_sub_list->count) return VTL_res_kArgumentError;

    if (p_sub_list->entries[index].text) {
        free(p_sub_list->entries[index].text);
    }
    if (p_sub_list->entries[index].style) {
        free(p_sub_list->entries[index].style);
    }

    // Сдвигаем элементы
    if (index < p_sub_list->count - 1) {
        memmove(&p_sub_list->entries[index], &p_sub_list->entries[index + 1], (p_sub_list->count - index - 1) * sizeof(VTL_sub_Entry));
    }
    
    p_sub_list->count--;
    
    if (p_sub_list->count == 0) {
        free(p_sub_list->entries);
        p_sub_list->entries = NULL;
    } else {
        // Пытаемся уменьшить массив, но не считаем критической ошибкой, если realloc вернет NULL
        // так как данные все еще действительны (хотя и с избыточной емкостью).
        VTL_sub_Entry* resized_entries = (VTL_sub_Entry*)realloc(p_sub_list->entries, p_sub_list->count * sizeof(VTL_sub_Entry));
        if (resized_entries) { 
            p_sub_list->entries = resized_entries;
        }
    }
    
    return VTL_res_kOk;
}

// Вспомогательная функция для удаления начальных и конечных пробельных символов
static char* VTL_sub_TrimWhitespace(char* str) {
    char *end;
    // Удаляем начальные пробелы
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str; // Строка состоит только из пробелов

    // Удаляем конечные пробелы
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

// Вспомогательная функция для удаления ASS-тегов из строки (типа {\b1}, {\c&HFFFFFF&})
static char* VTL_sub_StripAssTags(const char* text_with_tags) {
    if (!text_with_tags) return NULL;

    size_t len = strlen(text_with_tags);
    char* stripped_text = (char*)malloc(len + 1);
    if (!stripped_text) return NULL;

    const char* src = text_with_tags;
    char* dst = stripped_text;
    int in_tag = 0;

    while (*src) {
        if (*src == '{') {
            in_tag = 1;
            src++; // Пропускаем '{'
        } else if (*src == '}' && in_tag) {
            in_tag = 0;
            src++; // Пропускаем '}'
        } else {
            if (!in_tag) {
                *dst++ = *src;
            }
            src++;
        }
    }
    *dst = '\0';
    return stripped_text;
}

// Преобразование ARGB (0xAARRGGBB) в ASS (&HAABBGGRR)
static void VTL_sub_ArgbToAssStr(uint32_t argb, char* buf, size_t bufsz) {
    uint8_t a = (argb >> 24) & 0xFF;
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >> 8) & 0xFF;
    uint8_t b = argb & 0xFF;
    uint8_t a_ass = 0xFF - a; // Инверсия альфы
    snprintf(buf, bufsz, "&H%02X%02X%02X%02X", a_ass, b, g, r);
}

// Функции для парсинга и форматирования субтитров
VTL_AppResult VTL_sub_Parse(VTL_BufferData* p_buffer_data, VTL_sub_Format format, VTL_sub_List** pp_sub_list)
{
    if (!p_buffer_data || !p_buffer_data->data || !pp_sub_list) return VTL_res_kNullArgument;

    VTL_AppResult res = VTL_res_kOk;
    *pp_sub_list = NULL; // Инициализируем на случай ошибки до создания списка

    if (format == VTL_sub_format_kSRT || format == VTL_sub_format_kVTT) {
        res = VTL_sub_ListCreate(pp_sub_list);
        if (res != VTL_res_kOk) return res;

        char* buffer_copy = strdup(p_buffer_data->data);
        if (!buffer_copy) {
            VTL_sub_ListDestroy(pp_sub_list); // pp_sub_list здесь уже не NULL
            return VTL_res_kAllocError;
        }

        char* saveptr1; 
        char* line = strtok_r(buffer_copy, "\n", &saveptr1);
        VTL_sub_Entry current_entry;
        memset(&current_entry, 0, sizeof(VTL_sub_Entry)); 
        int state = 0; // 0: ожидаем индекс/идентификатор, 1: ожидаем время, 2: ожидаем текст
        char text_buffer[4096] = {0}; 
        int auto_increment_index = 1; 
        int vtt_header_checked = 0; // Флаг, что заголовок WEBVTT проверен (или его нет)

        while (line != NULL) {
            line = VTL_sub_TrimWhitespace(line);

            if (!vtt_header_checked && format == VTL_sub_format_kVTT) {
                if (strncmp(line, "WEBVTT", 6) == 0) {
                    // Пропускаем все до первой пустой строки после WEBVTT или до первой значащей строки, не являющейся комментарием/стилем
                    do {
                        line = strtok_r(NULL, "\n", &saveptr1);
                        if (line) line = VTL_sub_TrimWhitespace(line);
                    } while (line && (strlen(line) == 0 || strncmp(line, "NOTE", 4) == 0 || strncmp(line, "STYLE", 5) == 0 || strncmp(line, "REGION", 6) == 0));
                    vtt_header_checked = 1;
                    if (line == NULL) break; // Конец файла после заголовка VTT
                } else {
                     // Если нет "WEBVTT", но формат VTT, считаем, что заголовка нет или он не обязателен
                    vtt_header_checked = 1;
                }
            } else if (format != VTL_sub_format_kVTT) { // Для SRT, всегда считаем, что заголовок проверен
                vtt_header_checked = 1;
            }


            if (strlen(line) == 0) { 
                if (state == 2 && strlen(text_buffer) > 0) {
                    current_entry.text = strdup(text_buffer);
                    if (!current_entry.text) { res = VTL_res_kAllocError; break; }
                    
                    if(current_entry.index == 0 && format == VTL_sub_format_kSRT) current_entry.index = auto_increment_index++;
                    else if (format == VTL_sub_format_kVTT && current_entry.index != 0) { /* VTT c id, index уже в style */ }
                    else if (format == VTL_sub_format_kVTT && current_entry.index == 0) { /* VTT без id, индекс не нужен */ }


                    res = VTL_sub_ListAddEntry(*pp_sub_list, &current_entry);
                    free(current_entry.text); 
                    current_entry.text = NULL;
                    if (current_entry.style) { free(current_entry.style); current_entry.style = NULL; }

                    if (res != VTL_res_kOk) break;
                    
                    memset(text_buffer, 0, sizeof(text_buffer));
                    memset(&current_entry, 0, sizeof(VTL_sub_Entry)); 
                    state = 0; 
                }
                line = strtok_r(NULL, "\n", &saveptr1);
                continue;
            }

            if (!vtt_header_checked) { // Если все еще не обработали заголовок VTT, пропускаем строки
                line = strtok_r(NULL, "\n", &saveptr1);
                continue;
            }

            switch (state) {
                case 0: // Ожидаем индекс (SRT) или идентификатор/время (VTT)
                {
                    int parsed_index = -1;
                    char time_start_str[30], time_end_str[30], vtt_id_buffer[256];

                    if (sscanf(line, "%d", &parsed_index) == 1 && strstr(line, "-->") == NULL) { // Явный индекс SRT
                        current_entry.index = parsed_index;
                        state = 1;
                    } else if (sscanf(line, "%29s --> %29s", time_start_str, time_end_str) == 2 && strstr(line, "-->") != NULL) { // Строка времени (VTT без ID или SRT с индексом на той же строке, что не стандартно, но обработаем)
                        current_entry.start = VTL_sub_ParseSrtVttTime(time_start_str);
                        current_entry.end = VTL_sub_ParseSrtVttTime(time_end_str);
                        if (current_entry.start >= 0 && current_entry.end >= 0 && current_entry.end >= current_entry.start) {
                            state = 2; 
                            if (format == VTL_sub_format_kVTT) current_entry.index = 0; // Для VTT индекс не нужен, если это просто время
                            else if (current_entry.index == 0) current_entry.index = auto_increment_index++; // Для SRT, если индекс не был установлен ранее
                            memset(text_buffer, 0, sizeof(text_buffer));
                        } else { // Ошибка времени
                            state = 0; memset(&current_entry, 0, sizeof(VTL_sub_Entry));
                        }
                    } else if (format == VTL_sub_format_kVTT) { // Идентификатор VTT (не число и не время)
                        if (strlen(line) < sizeof(vtt_id_buffer)) {
                           current_entry.style = strdup(line); // Используем style для VTT ID
                           if(!current_entry.style) { res = VTL_res_kAllocError; goto cleanup_srt_vtt; }
                           current_entry.index = 0; // Явный индекс не используется, когда есть ID
                           state = 1; // Ожидаем время после ID
                        } else {
                             // Слишком длинный ID, пропускаем или ошибка
                            state = 0; memset(&current_entry, 0, sizeof(VTL_sub_Entry));
                        }
                    } else { // Нераспознанная строка для SRT в состоянии 0
                        state = 0; memset(&current_entry, 0, sizeof(VTL_sub_Entry));
                    }
                }
                    break;
                case 1: // Ожидаем время (после индекса SRT или ID VTT)
                {
                    char time_start_str[30], time_end_str[30];
                    if (sscanf(line, "%29s --> %29s", time_start_str, time_end_str) == 2 && strstr(line, "-->") != NULL) {
                        current_entry.start = VTL_sub_ParseSrtVttTime(time_start_str);
                        current_entry.end = VTL_sub_ParseSrtVttTime(time_end_str);
                        if (current_entry.start < 0 || current_entry.end < 0 || current_entry.end < current_entry.start) {
                            state = 0; 
                            if (current_entry.style) { free(current_entry.style); current_entry.style = NULL; }
                            memset(&current_entry, 0, sizeof(VTL_sub_Entry));
                        } else {
                            state = 2; 
                            if (format == VTL_sub_format_kSRT && current_entry.index == 0) current_entry.index = auto_increment_index++;
                            memset(text_buffer, 0, sizeof(text_buffer));
                        }
                    } else {
                        state = 0;
                        if (current_entry.style) { free(current_entry.style); current_entry.style = NULL; }
                        memset(&current_entry, 0, sizeof(VTL_sub_Entry));
                    }
                }
                    break;
                case 2: // Ожидаем текст
                    if (strlen(text_buffer) + strlen(line) + 2 < sizeof(text_buffer)) { // +1 за \n, +1 за \0
                        if (strlen(text_buffer) > 0) {
                            strcat(text_buffer, "\n");
                        }
                        strcat(text_buffer, line);
                    } else {
                        res = VTL_res_kMemoryError; 
                        goto cleanup_srt_vtt; 
                    }
                    break;
            }
            line = strtok_r(NULL, "\n", &saveptr1);
        }

        // Обрабатываем последнюю запись
        if (state == 2 && strlen(text_buffer) > 0) {
            current_entry.text = strdup(text_buffer);
            if (!current_entry.text) { res = VTL_res_kAllocError; goto cleanup_srt_vtt;}
            
            if(current_entry.index == 0 && format == VTL_sub_format_kSRT) current_entry.index = auto_increment_index++;
            else if (format == VTL_sub_format_kVTT && current_entry.index != 0) { /* VTT c id, index уже в style */ }
            else if (format == VTL_sub_format_kVTT && current_entry.index == 0) { /* VTT без id, индекс не нужен */ }

            res = VTL_sub_ListAddEntry(*pp_sub_list, &current_entry);
            free(current_entry.text);
            current_entry.text = NULL; 
            if (current_entry.style) { free(current_entry.style); current_entry.style = NULL; }
        }

    cleanup_srt_vtt:
        free(buffer_copy);
        if (res != VTL_res_kOk && *pp_sub_list) {
            VTL_sub_ListDestroy(pp_sub_list);
        }
        return res;

    } else if (format == VTL_sub_format_kASS) {
        res = VTL_sub_ListCreate(pp_sub_list);
        if (res != VTL_res_kOk) return res;

        char* buffer_copy = strdup(p_buffer_data->data);
        if (!buffer_copy) {
            VTL_sub_ListDestroy(pp_sub_list);
            return VTL_res_kAllocError;
        }

        char* saveptr_line;
        char* line = strtok_r(buffer_copy, "\n", &saveptr_line);
        
        VTL_sub_Entry current_entry;
        int auto_ass_index = 1;
        
        enum AssSection { SECTION_NONE, SECTION_SCRIPT_INFO, SECTION_STYLES, SECTION_EVENTS } current_section = SECTION_NONE;
        
        int format_start = -1, format_end = -1, format_style = -1, format_text = -1;
        int num_event_format_fields = 0;

        while (line != NULL) {
            line = VTL_sub_TrimWhitespace(line);

            if (line[0] == '[' && line[strlen(line)-1] == ']') { 
                if (strncmp(line, "[Script Info]", 13) == 0) current_section = SECTION_SCRIPT_INFO;
                else if (strncmp(line, "[V4+ Styles]", 12) == 0) current_section = SECTION_STYLES;
                else if (strncmp(line, "[Events]", 8) == 0) current_section = SECTION_EVENTS;
                else current_section = SECTION_NONE; 
                
                line = strtok_r(NULL, "\n", &saveptr_line);
                continue;
            }

            if (current_section == SECTION_EVENTS) {
                if (strncmp(line, "Format:", 7) == 0) {
                    char* format_line_ptr = line + 7; 
                    format_line_ptr = VTL_sub_TrimWhitespace(format_line_ptr);
                    char* saveptr_field;
                    char* field = strtok_r(format_line_ptr, ",", &saveptr_field);
                    int current_field_index = 0;
                    num_event_format_fields = 0;
                    format_start = format_end = format_style = format_text = -1; // Сброс индексов

                    while(field != NULL) {
                        field = VTL_sub_TrimWhitespace(field);
                        // Убрали лишние поля из отслеживания, оставили только нужные
                        if (strcmp(field, "Start") == 0) format_start = current_field_index;
                        else if (strcmp(field, "End") == 0) format_end = current_field_index;
                        else if (strcmp(field, "Style") == 0) format_style = current_field_index;
                        else if (strcmp(field, "Text") == 0) format_text = current_field_index;
                        current_field_index++;
                        field = strtok_r(NULL, ",", &saveptr_field);
                    }
                    num_event_format_fields = current_field_index;
                } else if (strncmp(line, "Dialogue:", 9) == 0) {
                    if (format_start == -1 || format_end == -1 || format_text == -1 || num_event_format_fields == 0) {
                        line = strtok_r(NULL, "\n", &saveptr_line);
                        continue;
                    }

                    memset(&current_entry, 0, sizeof(VTL_sub_Entry));
                    char* dialogue_line_ptr = line + 9; 
                    // НЕ используем VTL_sub_trimWhitespace для dialogue_line_ptr здесь, т.к. начальные пробелы могут быть частью первого поля
                    
                    char* fields[64]; 
                    int field_count = 0;
                    char* current_pos = dialogue_line_ptr;
                    int N = num_event_format_fields; 

                    for(int k=0; k < N-1 && current_pos != NULL; ++k) { 
                        char* comma_pos = strchr(current_pos, ',');
                        if (comma_pos) {
                            *comma_pos = '\0'; 
                            fields[field_count++] = VTL_sub_TrimWhitespace(current_pos); // Trim каждое поле индивидуально
                            current_pos = comma_pos + 1;
                        } else { 
                            fields[field_count++] = VTL_sub_TrimWhitespace(current_pos);
                            current_pos = NULL; 
                            break; 
                        }
                    }
                    if(current_pos != NULL && field_count < N) { 
                         fields[field_count++] = VTL_sub_TrimWhitespace(current_pos); 
                    }
                    
                    if (field_count == num_event_format_fields) {
                        current_entry.start = (format_start < field_count) ? VTL_sub_ParseAssTime(fields[format_start]) : -1.0;
                        current_entry.end = (format_end < field_count) ? VTL_sub_ParseAssTime(fields[format_end]) : -1.0;
                        
                        if (current_entry.start < 0 || current_entry.end < 0 || current_entry.end < current_entry.start) {
                            // Ошибка времени, пропускаем запись
                        } else {
                            char* raw_text = (format_text < field_count) ? fields[format_text] : NULL;
                            if (raw_text) {
                                char* cleaned_text = VTL_sub_StripAssTags(raw_text);
                                if (cleaned_text) {
                                    current_entry.text = strdup(cleaned_text);
                                    free(cleaned_text);
                                    if (!current_entry.text) { res = VTL_res_kAllocError; goto cleanup_ass; }
                                } else {
                                    res = VTL_res_kAllocError; goto cleanup_ass; // Ошибка в strip_ass_tags (malloc)
                                }
                            } else {
                                current_entry.text = NULL; // Или strdup("") если нужно не NULL
                            }

                            if (format_style != -1 && format_style < field_count) {
                                current_entry.style = strdup(fields[format_style]);
                                if (fields[format_style] && !current_entry.style) {
                                    if(current_entry.text) free(current_entry.text);
                                    res = VTL_res_kAllocError; goto cleanup_ass;
                                }
                            } else {
                                current_entry.style = NULL;
                            }
                            current_entry.index = auto_ass_index++;
                            res = VTL_sub_ListAddEntry(*pp_sub_list, &current_entry);

                            if(current_entry.text) free(current_entry.text);
                            current_entry.text = NULL; // После AddEntry текст уже скопирован или не нужен
                            if(current_entry.style) free(current_entry.style);
                            current_entry.style = NULL; // Аналогично для стиля
                            if (res != VTL_res_kOk) goto cleanup_ass;
                        }
                    }
                }
            }
            line = strtok_r(NULL, "\n", &saveptr_line);
        }
    cleanup_ass:
        free(buffer_copy);
        if (res != VTL_res_kOk && *pp_sub_list && *pp_sub_list != (void*)-1) { // Проверка что *pp_sub_list не мусор
             if ((*pp_sub_list)->entries || (*pp_sub_list)->count > 0) { // Дополнительная проверка перед destroy
                VTL_sub_ListDestroy(pp_sub_list);
             }
        }
        return res;
    } else {
        return VTL_res_kUnsupportedFormat;
    }
}

// Вспомогательная функция для форматирования секунд в строку HH:MM:SS,ms (SRT), HH:MM:SS.ms (VTT), H:MM:SS.ss (ASS)
static void VTL_sub_FormatSubTime(double time_seconds, char* buffer, size_t buffer_size, VTL_sub_Format format) {
    if (time_seconds < 0) time_seconds = 0;
    int hours = (int)(time_seconds / 3600);
    double remaining_seconds = time_seconds - hours * 3600.0;
    int minutes = (int)(remaining_seconds / 60.0);
    remaining_seconds -= minutes * 60.0;
    int seconds = (int)remaining_seconds;
    
    if (format == VTL_sub_format_kASS) {
        int centiseconds = (int)((remaining_seconds - seconds) * 100.0);
        snprintf(buffer, buffer_size, "%d:%02d:%02d.%02d", hours, minutes, seconds, centiseconds);
    } else { // SRT, VTT
        int milliseconds = (int)((remaining_seconds - seconds) * 1000.0);
        char separator = (format == VTL_sub_format_kVTT) ? '.' : ',';
        snprintf(buffer, buffer_size, "%02d:%02d:%02d%c%03d", hours, minutes, seconds, separator, milliseconds);
    }
}

typedef struct {
    char** output_string;
    size_t* current_length;
    size_t* current_capacity;
} OutputBufferCtx;

static VTL_AppResult VTL_sub_AppendToOutput(OutputBufferCtx* ctx, const char* str_to_add) {
    size_t len_to_add = strlen(str_to_add);
    if (*(ctx->current_length) + len_to_add + 1 > *(ctx->current_capacity)) {
        *(ctx->current_capacity) = (*(ctx->current_length) + len_to_add + 1) * 2;
        char* new_output_string = (char*)realloc(*(ctx->output_string), *(ctx->current_capacity));
        if (!new_output_string) {
            return VTL_res_kAllocError;
        }
        *(ctx->output_string) = new_output_string;
    }
    strcat(*(ctx->output_string), str_to_add);
    *(ctx->current_length) += len_to_add;
    return VTL_res_kOk;
}

VTL_AppResult VTL_sub_FormatToString(const VTL_sub_List* p_sub_list, VTL_sub_Format format, VTL_BufferData** pp_buffer_data, const VTL_sub_StyleParams* style_params)
{
    if (!p_sub_list || !pp_buffer_data) return VTL_res_kNullArgument;

    *pp_buffer_data = (VTL_BufferData*)malloc(sizeof(VTL_BufferData));
    if (!*pp_buffer_data) return VTL_res_kAllocError;
    (*pp_buffer_data)->data = NULL;
    (*pp_buffer_data)->data_size = 0;

    size_t current_capacity = 2048;
    char* output_string = (char*)malloc(current_capacity);
    if (!output_string) {
        free(*pp_buffer_data);
        *pp_buffer_data = NULL;
        return VTL_res_kAllocError;
    }
    output_string[0] = '\0';
    size_t current_length = 0;
    OutputBufferCtx ctx = { &output_string, &current_length, &current_capacity };

    if (format == VTL_sub_format_kVTT) {
        if (VTL_sub_AppendToOutput(&ctx, "WEBVTT\n\n") != VTL_res_kOk) goto format_error;
    } else if (format == VTL_sub_format_kASS) {
        // Базовая информация о скрипте
        if (VTL_sub_AppendToOutput(&ctx, "[Script Info]\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "Title: Subtitles\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "ScriptType: v4.00+\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "WrapStyle: 0\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "PlayResX: 384\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "PlayResY: 288\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "\n") != VTL_res_kOk) goto format_error;

        // Стили
        if (VTL_sub_AppendToOutput(&ctx, "[V4+ Styles]\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n") != VTL_res_kOk) goto format_error;
        char style_line[512];
        if (style_params) {
            char color_buf[16], back_buf[16], outline_buf[16];
            VTL_sub_ArgbToAssStr(style_params->primary_color, color_buf, sizeof(color_buf));
            VTL_sub_ArgbToAssStr(style_params->back_color, back_buf, sizeof(back_buf));
            VTL_sub_ArgbToAssStr(style_params->outline_color, outline_buf, sizeof(outline_buf));
            snprintf(style_line, sizeof(style_line),
                "Style: Default,%s,%d,%s,%s,%s,%s,%d,%d,%d,%d,100,100,0,0,%d,%d,%d,%d,%d,%d,%d,1\n",
                style_params->fontname ? style_params->fontname : "Arial",
                style_params->fontsize > 0 ? style_params->fontsize : 20,
                color_buf,
                color_buf, // SecondaryColour (можно сделать отдельным)
                outline_buf,
                back_buf,
                style_params->bold ? -1 : 0,
                style_params->italic,
                style_params->underline,
                0, // StrikeOut
                style_params->border_style,
                style_params->outline,
                2, // Shadow
                style_params->alignment > 0 ? style_params->alignment : 2,
                style_params->margin_l,
                style_params->margin_r,
                style_params->margin_v
            );
        } else {
            snprintf(style_line, sizeof(style_line), "Style: Default,Arial,20,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,2,2,10,10,10,1\n");
        }
        if (VTL_sub_AppendToOutput(&ctx, style_line) != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "\n") != VTL_res_kOk) goto format_error;
        
        // События
        if (VTL_sub_AppendToOutput(&ctx, "[Events]\n") != VTL_res_kOk) goto format_error;
        if (VTL_sub_AppendToOutput(&ctx, "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n") != VTL_res_kOk) goto format_error;
    }


    if (format == VTL_sub_format_kSRT || format == VTL_sub_format_kVTT || format == VTL_sub_format_kASS) {
        // Если формат ASS — массово удаляем теги из текста через OpenCL (или CPU fallback)
        if (format == VTL_sub_format_kASS && p_sub_list->count > 0) {
            const char** in_texts = (const char**)malloc(sizeof(char*) * p_sub_list->count);
            char** out_texts = (char**)malloc(sizeof(char*) * p_sub_list->count);
            if (in_texts && out_texts) {
                for (size_t i = 0; i < p_sub_list->count; ++i) in_texts[i] = p_sub_list->entries[i].text;
                VTL_sub_OpenclStripTags(in_texts, out_texts, p_sub_list->count);
                for (size_t i = 0; i < p_sub_list->count; ++i) {
                    if (out_texts[i]) {
                        free((void*)p_sub_list->entries[i].text);
                        p_sub_list->entries[i].text = out_texts[i];
                    }
                }
            }
            free(in_texts);
            free(out_texts);
        }

        for (size_t i = 0; i < p_sub_list->count; ++i) {
            VTL_sub_Entry* entry = &p_sub_list->entries[i];
            char entry_render_buffer[2048]; // Увеличенный буфер для рендеринга одной строки субтитра
            char time_start_str[30];
            char time_end_str[30];

            VTL_sub_FormatSubTime(entry->start, time_start_str, sizeof(time_start_str), format);
            VTL_sub_FormatSubTime(entry->end, time_end_str, sizeof(time_end_str), format);

            const char* entry_text = entry->text ? entry->text : "";
            int snprintf_result = -1;

            if (format == VTL_sub_format_kSRT) {
                 snprintf_result = snprintf(entry_render_buffer, sizeof(entry_render_buffer),
                                         "%d\n%s --> %s\n%s",
                                         entry->index ? entry->index : (int)(i+1), // Используем i+1 если index 0
                                         time_start_str, time_end_str,
                                         entry_text);
            } else if (format == VTL_sub_format_kVTT) {
                if (entry->style && strlen(entry->style) > 0) { 
                     snprintf_result = snprintf(entry_render_buffer, sizeof(entry_render_buffer),
                                             "%s\n%s --> %s\n%s",
                                             entry->style, // VTT ID
                                             time_start_str, time_end_str,
                                             entry_text);
                } else { // VTT без ID
                     snprintf_result = snprintf(entry_render_buffer, sizeof(entry_render_buffer),
                                             "%s --> %s\n%s",
                                             time_start_str, time_end_str,
                                             entry_text);
                }
            } else if (format == VTL_sub_format_kASS) {
                const char* style_name = (entry->style && strlen(entry->style) > 0) ? entry->style : "Default";
                // Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
                snprintf_result = snprintf(entry_render_buffer, sizeof(entry_render_buffer),
                                         "Dialogue: 0,%s,%s,%s,,0,0,0,,%s", // Layer 0, Name пусто, Margins 0, Effect пусто
                                         time_start_str, time_end_str, style_name, entry_text);
            }

            if (snprintf_result < 0 || snprintf_result >= (int)sizeof(entry_render_buffer)) {
                 goto format_error;
            }
            if (VTL_sub_AppendToOutput(&ctx, entry_render_buffer) != VTL_res_kOk) goto format_error;

            // Добавляем новые строки после записи
            if (format == VTL_sub_format_kSRT || format == VTL_sub_format_kVTT) {
                int newlines_after_entry = (i < p_sub_list->count - 1) ? 2 : (strlen(entry_text) > 0 || format == VTL_sub_format_kVTT ? 1 : 0);
                 if (format == VTL_sub_format_kVTT && i == p_sub_list->count -1 && strlen(entry_text) == 0) newlines_after_entry = 0;


                for(int nl = 0; nl < newlines_after_entry; ++nl) {
                     if (VTL_sub_AppendToOutput(&ctx, "\n") != VTL_res_kOk) goto format_error;
                }
            } else if (format == VTL_sub_format_kASS) {
                 if (VTL_sub_AppendToOutput(&ctx, "\n") != VTL_res_kOk) goto format_error; // Каждая строка Dialogue на новой строке
            }
        }
        
        (*pp_buffer_data)->data = output_string;
        (*pp_buffer_data)->data_size = current_length; 
        return VTL_res_kOk;

    } else { // Неподдерживаемый формат (хотя проверка уже была выше)
        // Этот блок кода не должен достигаться, если все форматы обрабатываются в if/else if
    }

format_error:
    free(output_string);
    free(*pp_buffer_data);
    *pp_buffer_data = NULL;
    return VTL_res_kAllocError; // Или другая подходящая ошибка
}

// Вспомогательная функция для определения формата субтитров по расширению файла
static VTL_sub_Format VTL_sub_DetectFormatFromExtension(const char* file_path) {
    if (!file_path) return VTL_sub_format_kUNKNOWN;
    const char *dot = strrchr(file_path, '.');
    if (!dot || dot == file_path) return VTL_sub_format_kUNKNOWN;

    if (strcmp(dot, ".srt") == 0) return VTL_sub_format_kSRT;
    if (strcmp(dot, ".ass") == 0) return VTL_sub_format_kASS;
    if (strcmp(dot, ".ssa") == 0) return VTL_sub_format_kASS; // .ssa тоже считаем ASS
    if (strcmp(dot, ".vtt") == 0) return VTL_sub_format_kVTT;
    // TODO: При необходимости добавить другие расширения

    return VTL_sub_format_kUNKNOWN;
}

// Функции для загрузки и сохранения списка субтитров из/в файл
VTL_AppResult VTL_sub_LoadFromFile(const char* file_path, VTL_sub_Format* detected_format_out, VTL_sub_List** pp_sub_list)
{
    if (!file_path || !pp_sub_list || !detected_format_out) return VTL_res_kNullArgument;

    *pp_sub_list = NULL;
    VTL_sub_Format format = VTL_sub_DetectFormatFromExtension(file_path);
    *detected_format_out = format;

    if (format == VTL_sub_format_kUNKNOWN) {
        return VTL_res_kUnsupportedFormat; // Не удалось определить формат или не поддерживается
    }

    FILE* fp = fopen(file_path, "rb"); // Открываем в бинарном режиме для корректного ftell/fread
    if (!fp) {
        return VTL_res_kIOError; // Ошибка открытия файла
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0 && file_size != 0) { // Проверка на ошибку ftell или пустой файл (0 - допустимо для пустого файла субтитров)
        fclose(fp);
        // Если файл пуст, можно создать пустой список субтитров
        if (file_size == 0) return VTL_sub_ListCreate(pp_sub_list);
        return VTL_res_kIOError;
    }
    
    // Создаем VTL_BufferData для передачи в VTL_sub_parse
    VTL_BufferData buffer_data;
    buffer_data.data = (char*)malloc(file_size + 1); // +1 для null-терминатора
    if (!buffer_data.data) {
        fclose(fp);
        return VTL_res_kAllocError;
    }

    size_t bytes_read = fread(buffer_data.data, 1, file_size, fp);
    fclose(fp);

    if (bytes_read != (size_t)file_size) {
        free(buffer_data.data);
        return VTL_res_kIOError; // Ошибка чтения файла
    }
    buffer_data.data[file_size] = '\0'; // Null-терминируем буфер
    buffer_data.data_size = file_size; // Фактический размер данных без null-терминатора

    VTL_AppResult parse_res = VTL_sub_Parse(&buffer_data, format, pp_sub_list);
    
    free(buffer_data.data); // Освобождаем буфер после парсинга

    return parse_res;
}

VTL_AppResult VTL_sub_SaveToFile(const char* file_path, VTL_sub_Format format, const VTL_sub_List* p_sub_list, const VTL_sub_StyleParams* style_params)
{
    if (!file_path || !p_sub_list) return VTL_res_kNullArgument;

    VTL_sub_Format target_format = format;
    if (target_format == VTL_sub_format_kUNKNOWN) {
        target_format = VTL_sub_DetectFormatFromExtension(file_path);
        if (target_format == VTL_sub_format_kUNKNOWN) {
            return VTL_res_kUnsupportedFormat; // Не удалось определить формат для сохранения
        }
    }
    
    VTL_BufferData* buffer_data = NULL;
    VTL_AppResult format_res = VTL_sub_FormatToString(p_sub_list, target_format, &buffer_data, style_params);

    if (format_res != VTL_res_kOk) {
        if (buffer_data) { // VTL_sub_FormatToString может выделить buffer_data даже при ошибке
            free(buffer_data->data);
            free(buffer_data);
        }
        return format_res;
    }

    if (!buffer_data || !buffer_data->data) {
        if (buffer_data) free(buffer_data); // Если data NULL, но сам buffer_data выделен
        // Это может произойти, если список субтитров пуст и FormatToString решил вернуть пустой буфер
        // В этом случае просто создадим пустой файл
    }

    FILE* fp = fopen(file_path, "wb"); // Открываем в бинарном режиме для записи
    if (!fp) {
        if (buffer_data) {
            free(buffer_data->data);
            free(buffer_data);
        }
        return VTL_res_kIOError; // Ошибка открытия файла для записи
    }

    if (buffer_data && buffer_data->data && buffer_data->data_size > 0) {
        size_t bytes_written = fwrite(buffer_data->data, 1, buffer_data->data_size, fp);
        if (bytes_written != buffer_data->data_size) {
            fclose(fp);
            free(buffer_data->data);
            free(buffer_data);
            return VTL_res_kIOError; // Ошибка записи в файл
        }
    } else {
        // Если buffer_data пуст (например, пустой список субтитров), просто создается/перезаписывается пустой файл
        // fwrite ничего не запишет, если data_size равен 0, что корректно
    }

    fclose(fp);
    if (buffer_data) { // buffer_data и data освобождаются в любом случае
        free(buffer_data->data);
        free(buffer_data);
    }

    return VTL_res_kOk;
} 

static double VTL_sub_ParseSrtVttTime(const char* time_str) {
    int h, m, s, ms;
    if (sscanf(time_str, "%d:%d:%d,%d", &h, &m, &s, &ms) == 4 || 
        sscanf(time_str, "%d:%d:%d.%d", &h, &m, &s, &ms) == 4) {
        return h * 3600.0 + m * 60.0 + s + ms / 1000.0;
    }
    return -1.0;
}

static double VTL_sub_ParseAssTime(const char* time_str) {
    int h, m, s, cs;
    if (sscanf(time_str, "%d:%d:%d.%d", &h, &m, &s, &cs) == 4) {
        return h * 3600.0 + m * 60.0 + s + cs / 100.0;
    }
    return -1.0;
} 