#include <VTL/content_platform/infra/text/VTL_publication_text_gen.h>
#include <VTL/VTL_publication_markup_text_flags.h>

static VTL_AppResult VTL_publication_marked_text_GenStandartMDFile(const VTL_publication_MarkedText* p_src_marked_text, 
                                                            const VTL_Filename out_file_name)
{
    VTL_publication_Text* p_out_text;
    VTL_publication_marked_text_TransformToStandartMD(&p_out_text, p_src_marked_text);
    VTL_pusblication_text_Write(p_out_text, out_file_name);
    return VTL_res_kOk;
}

static VTL_AppResult VTL_publication_marked_text_GenTelegramMDFile(const VTL_publication_MarkedText* p_src_marked_text,
                                                            const VTL_Filename out_file_name)
{
    VTL_publication_Text* p_out_text;
    VTL_publication_marked_text_TransformToTelegramMD(&p_out_text, p_src_marked_text);
    VTL_pusblication_text_Write(p_out_text, out_file_name);
    return VTL_res_kOk;
}

static VTL_AppResult VTL_publication_marked_text_GenHTMLFile(const VTL_publication_MarkedText* p_src_marked_text, 
                                                            const VTL_Filename out_file_name)
{
    VTL_publication_Text* p_out_text;
    VTL_publication_marked_text_TransformToHTML(&p_out_text, p_src_marked_text);
    VTL_pusblication_text_Write(p_out_text, out_file_name);
    return VTL_res_kOk;
}

static VTL_AppResult VTL_publication_marked_text_GenRegularTextFile(
                                                            const VTL_publication_MarkedText* p_src_marked_text,
                                                            const VTL_Filename out_file_name)
{
    VTL_publication_Text* p_out_text;
    VTL_publication_marked_text_TransformToRegularText(&p_out_text, p_src_marked_text);
    VTL_pusblication_text_Write(p_out_text, out_file_name);
    return VTL_res_kOk;
}

static VTL_AppResult VTL_publication_marked_text_GenBBFile(const VTL_publication_MarkedText* p_src_marked_text, 
                                                            const VTL_Filename out_file_name)
{
    VTL_publication_Text* p_out_text;
    VTL_publication_marked_text_TransformToStandartMD(&p_out_text, p_src_marked_text);
    VTL_pusblication_text_Write(p_out_text, out_file_name);
    return VTL_res_kOk;
}

static VTL_AppResult VTL_publication_marked_text_CheckAndGenStandartMDFile(
                                                    const VTL_publication_MarkedText* p_src_marked_text,
                                                    const VTL_Filename src_file_name, 
                                                    const VTL_publication_marked_text_MarkupType src_markup_type,
                                                    const VTL_Filename out_file_name,
                                                    const VTL_publication_marked_text_type_Flags flags)
{
    if (!VTL_publication_text_is_standart_md(flags))
    {
        return VTL_res_kOk;
    }
    if( src_markup_type == VTL_markup_type_kStandartMD )
    {
        if(!VTL_file_CheckEquality(src_file_name, out_file_name) )
        {
            return VTL_file_Copy(out_file_name, src_file_name);
        }
        return VTL_res_kOk;
    }
    else
    {
        return VTL_publication_marked_text_GenStandartMDFile(p_src_marked_text, out_file_name);
    }        
}


static VTL_AppResult VTL_publication_marked_text_CheckAndGenTelegramMDFile(
                                                    const VTL_publication_MarkedText* p_src_marked_text,
                                                    const VTL_Filename src_file_name, 
                                                    const VTL_publication_marked_text_MarkupType src_markup_type,
                                                    const VTL_Filename out_file_name,
                                                    const VTL_publication_marked_text_type_Flags flags)
{
    if (!VTL_publication_text_is_telegram_md(flags))
    {
        return VTL_res_kOk;
    }
    if( src_markup_type == VTL_markup_type_kTelegramMD )
    {
        if(!VTL_file_CheckEquality(src_file_name, out_file_name) )
        {
            return VTL_file_Copy(out_file_name, src_file_name);
        }
        return VTL_res_kOk;
    }
    else
    {
        return VTL_publication_marked_text_GenTelegramMDFile(p_src_marked_text, out_file_name);
    }        
}


static VTL_AppResult VTL_publication_marked_text_CheckAndGenHTMLFile(
                                                    const VTL_publication_MarkedText* p_src_marked_text,
                                                    const VTL_Filename src_file_name, 
                                                    const VTL_publication_marked_text_MarkupType src_markup_type,
                                                    const VTL_Filename out_file_name,
                                                    const VTL_publication_marked_text_type_Flags flags)
{
    if (!VTL_publication_text_is_html(flags))
    {
        return VTL_res_kOk;
    }
    if( src_markup_type == VTL_markup_type_kHTML )
    {
        if(!VTL_file_CheckEquality(src_file_name, out_file_name) )
        {
            return VTL_file_Copy(out_file_name, src_file_name);
        }
        return VTL_res_kOk;
    }
    else
    {
        return VTL_publication_marked_text_GenHTMLFile(p_src_marked_text, out_file_name);
    }        
}

static VTL_AppResult VTL_publication_marked_text_CheckAndGenBBFile(
                                                    const VTL_publication_MarkedText* p_src_marked_text,
                                                    const VTL_Filename src_file_name, 
                                                    const VTL_publication_marked_text_MarkupType src_markup_type,
                                                    const VTL_Filename out_file_name,
                                                    const VTL_publication_marked_text_type_Flags flags)
{
    if (!VTL_publication_text_is_bb(flags))
    {
        return VTL_res_kOk;
    }
    if( src_markup_type == VTL_markup_type_kBB )
    {
        if(!VTL_file_CheckEquality(src_file_name, out_file_name) )
        {
            return VTL_file_Copy(out_file_name, src_file_name);
        }
        return VTL_res_kOk;
    }
    else
    {
        return VTL_publication_marked_text_GenBBFile(p_src_marked_text, out_file_name);
    }        
}

static VTL_AppResult VTL_publication_marked_text_CheckAndGenRegularText(
                                                    const VTL_publication_MarkedText* p_src_marked_text,
                                                    const VTL_Filename src_file_name, 
                                                    const VTL_publication_marked_text_MarkupType src_markup_type,
                                                    const VTL_Filename out_file_name,
                                                    const VTL_publication_marked_text_type_Flags flags)
{
    if (!VTL_publication_text_is_regular(flags))
    {
        return VTL_res_kOk;
    }
    return VTL_publication_marked_text_GenRegularTextFile(p_src_marked_text, out_file_name);
}

static VTL_AppResult VTL_publication_marked_text_CheckAndGen(const VTL_publication_MarkedText* p_marked_text,
                                                    const VTL_Filename src_file_name, 
                                                    const VTL_publication_marked_text_MarkupType src_markup_type,
                                                    const VTL_publication_marked_text_Configs* p_configs)
{
    VTL_AppResult app_result = VTL_res_kOk;
    app_result = VTL_publication_marked_text_CheckAndGenStandartMDFile(p_marked_text, src_file_name, src_markup_type, 
                                                        p_configs->file_names[VTL_PUBLICATION_TEXT_TYPE_STANDART_MD_INDEX],
                                                        p_configs->flags);
    if(app_result != VTL_res_kOk)
    {
        return app_result;
    }

    app_result = VTL_publication_marked_text_CheckAndGenTelegramMDFile(p_marked_text, src_file_name, src_markup_type, 
                                                        p_configs->file_names[VTL_PUBLICATION_TEXT_TYPE_TELEGRAM_MD_INDEX],
                                                        p_configs->flags);
    if(app_result != VTL_res_kOk)
    {
        return app_result;
    }

    app_result = VTL_publication_marked_text_CheckAndGenBBFile(p_marked_text, src_file_name, src_markup_type, 
                                                        p_configs->file_names[VTL_PUBLICATION_TEXT_TYPE_BB_INDEX], 
                                                        p_configs->flags);
    if(app_result != VTL_res_kOk)
    {
        return app_result;
    }

    app_result = VTL_publication_marked_text_CheckAndGenHTMLFile(p_marked_text, src_file_name, src_markup_type, 
                                                        p_configs->file_names[VTL_PUBLICATION_TEXT_TYPE_HTML_INDEX], 
                                                        p_configs->flags);
    if(app_result != VTL_res_kOk)
    {
        return app_result;
    }
    
    app_result = VTL_publication_marked_text_CheckAndGenRegularText(p_marked_text, src_file_name, src_markup_type, 
                                                        p_configs->file_names[VTL_PUBLICATION_TEXT_TYPE_REGULAR_INDEX], 
                                                        p_configs->flags);
    if(app_result != VTL_res_kOk)
    {
        return app_result;
    }
    return app_result;
}

VTL_AppResult VTL_publication_marked_text_GenFiles(const VTL_Filename src_file_name, 
                                                    const VTL_publication_marked_text_MarkupType src_markup_type,
                                                    const VTL_publication_marked_text_Configs* p_configs)
{
    VTL_publication_MarkedText* p_marked_text;
    VTL_publication_Text* p_src_text;
    VTL_AppResult app_result = VTL_res_kOk;
    app_result = VTL_pusblication_text_Read(&p_src_text, src_file_name);
    VTL_publication_marked_text_Init(&p_marked_text, p_src_text, src_markup_type);
    VTL_publication_marked_text_CheckAndGen(p_marked_text, src_file_name, src_markup_type, p_configs);
    return VTL_res_kOk;
}

