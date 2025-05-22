#include <VTL/VTL_publication_markup_text_flags.h>

bool VTL_publication_text_is_standart_md(int flags)
{
    return (flags & VTL_PUBLICATION_TEXT_TYPE_STANDART_MD) == VTL_PUBLICATION_TEXT_TYPE_STANDART_MD;
}

bool VTL_publication_text_is_telegram_md(int flags)
{
    return (flags & VTL_PUBLICATION_TEXT_TYPE_TELEGRAM_MD) == VTL_PUBLICATION_TEXT_TYPE_TELEGRAM_MD;
}

bool VTL_publication_text_is_html(int flags)
{
    return (flags & VTL_PUBLICATION_TEXT_TYPE_HTML) == VTL_PUBLICATION_TEXT_TYPE_HTML;
}

bool VTL_publication_text_is_bb(int flags)
{
    return (flags & VTL_PUBLICATION_TEXT_TYPE_BB) == VTL_PUBLICATION_TEXT_TYPE_BB;
}

bool VTL_publication_text_is_regular(int flags)
{
    return (flags & VTL_PUBLICATION_TEXT_TYPE_REGULAR) == VTL_PUBLICATION_TEXT_TYPE_REGULAR;
}
