#include <VTL/content_platform/infra/audio/VTL_audio_configs_for_gen_init.h>



VTL_AppResult VTL_audio_configs_for_gen_Init(VTL_audio_Configs** pp_new_configs, 
                                    VTL_audio_configs_platforms_Indices** pp_indices,
                                    const VTL_Filename src_file_name, const VTL_content_platform_flags flags)
{
    VTL_audio_MetaData audio_meta_data = {0};
    VTL_audio_meta_data_InitFromSource(&audio_meta_data, src_file_name);

    // Выделяем память только под структуру VTL_audio_Configs (без массива данных)
    *pp_new_configs = (VTL_audio_Configs*)malloc(sizeof(VTL_audio_Configs));
    // Теперь отдельно выделяем память под массив конфигов (гибко, безопасно, удобно для последующего realloc)
    (*pp_new_configs)->data = (VTL_audio_Congif*)malloc(VTL_CONTENT_PLATFORM_MAX_NUM * sizeof(VTL_audio_Congif));
    (*pp_new_configs)->length = 0;

    // Выделяем память под индексы платформ: сначала максимальный размер, потом можно будет уменьшить через realloc
    *pp_indices =   (VTL_audio_configs_platforms_Indices*) 
                    malloc(VTL_CONTENT_PLATFORM_MAX_NUM * sizeof((*pp_indices)->data[0]) + sizeof(*pp_indices));
    (*pp_indices)->length = 0;

    VTL_audio_configs_platforms_Indices* p_indices = *pp_indices;
    p_indices->length = 0;

    
    // Если выбран флаг TG, инициализируем параметры для TG и увеличиваем длину массивов
    if(VTL_content_platform_flags_CheckTg(flags))
    {        
        VTL_audio_tg_SetParams(&(*pp_new_configs)->data[(*pp_new_configs)->length].params, &audio_meta_data.params);
        ++(*pp_new_configs)->length;

        ++(*pp_indices)->length;
    }
    
    // После заполнения массивов уменьшаем выделенную память до реально используемого размера (realloc)
    (*pp_new_configs)->data = (VTL_audio_Congif*)realloc((*pp_new_configs)->data, (*pp_new_configs)->length * sizeof(VTL_audio_Congif));
    *pp_indices = realloc(*pp_indices, (*pp_indices)->length*sizeof((*pp_indices)->data[0]) + sizeof(*pp_indices));
   
    return VTL_res_kOk;
}