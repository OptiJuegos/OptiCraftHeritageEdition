#include "GuiSlotLanguage.h"

#include "FontRenderer.h"
#include "GameSettings.h"
#include "GuiLanguage.h"
#include "GuiSmallButton.h"
#include "Minecraft.h"
#include "StringTranslate.h"

GuiSlotLanguage::GuiSlotLanguage(GuiLanguage *languageGuiIn)
    : GuiSlot(languageGuiIn->mc, languageGuiIn->width, languageGuiIn->height,
              32, languageGuiIn->height - 61, 18)
    , languageGui(languageGuiIn)
    , languages(StringTranslate::getInstance()->getLanguageList())
{
    for (const auto &entry : languages)
        languageCodes.push_back(entry.first);
}

int_t GuiSlotLanguage::getSize()
{
    return (int_t)languageCodes.size();
}

void GuiSlotLanguage::elementClicked(int_t index, bool)
{
    if (index < 0 || index >= (int_t)languageCodes.size())
        return;

    const std::string &code = languageCodes[index];
    StringTranslate *translate = StringTranslate::getInstance();
    translate->setLanguage(code);
    languageGui->mc->fontRenderer->setUnicodeFlag(translate->isUnicode());
    languageGui->settings->language = code;
    languageGui->fontRenderer->setBidiFlag(StringTranslate::isBidirectional(code));
    if (languageGui->doneButton != nullptr)
        languageGui->doneButton->displayString = translate->translateKey("gui.done");
}

bool GuiSlotLanguage::isSelected(int_t index)
{
    return index >= 0 && index < (int_t)languageCodes.size()
        && languageCodes[index] == StringTranslate::getInstance()->getCurrentLanguage();
}

int_t GuiSlotLanguage::getContentHeight()
{
    return getSize() * 18;
}

void GuiSlotLanguage::drawBackground()
{
    languageGui->drawDefaultBackground();
}

void GuiSlotLanguage::drawSlot(int_t index, int_t, int_t y, int_t, Tessellator *)
{
    if (index < 0 || index >= (int_t)languageCodes.size())
        return;

    const std::string &code = languageCodes[index];
    auto it = languages.find(code);
    const std::string &display = it != languages.end() ? it->second : code;
    languageGui->fontRenderer->setBidiFlag(true);
    languageGui->drawCenteredString(languageGui->fontRenderer, display, languageGui->width / 2, y + 1, 0xffffff);
    languageGui->fontRenderer->setBidiFlag(StringTranslate::isBidirectional(languageGui->settings->language));
}
