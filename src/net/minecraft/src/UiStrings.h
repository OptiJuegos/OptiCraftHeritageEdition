#pragma once
#include "StringTranslate.h"

// English source text is a stable fallback for project-specific UI entries.
// Vanilla wording resolves to existing .lang keys; additions live in lang/ui/.
inline std::string uiText(const std::string &english)
{
    return StringTranslate::getInstance()->translateUi(english);
}
