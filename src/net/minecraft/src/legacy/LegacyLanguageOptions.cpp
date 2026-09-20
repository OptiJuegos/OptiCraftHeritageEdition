#include "net/minecraft/src/UiStrings.h"
#include "LegacyLanguageOptions.h"

#include <algorithm>
#include <vector>

#include "LegacyGuiButton.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/StringTranslate.h"

namespace
{
constexpr int_t LANGUAGE_ROW_BASE = 500;
constexpr int_t BUTTON_PREVIOUS = 590;
constexpr int_t BUTTON_NEXT = 591;
constexpr int_t BUTTON_DONE = 599;
}

LegacyLanguageOptions::LegacyLanguageOptions(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : LegacyOptionsScreen(parent, settingsValue, backgroundModeValue), currentPage(0), visibleRows(8)
{
}

void LegacyLanguageOptions::initGui()
{
    StringTranslate *translate = StringTranslate::getInstance();
    translate->filterToLatinLanguagesOnPs2();

    // Two of the rows are the pager and Done, so the list keeps what is left of
    // the screen. Eight stays the cap the page model was written against.
    visibleRows = std::max<int_t>(3, std::min<int_t>(8,
        legacyOptionsMaxRows(width, height, LegacyOptionsLayoutPreset::Wide) - 2));
    std::vector<LegacyLanguageEntry> entries;
    const auto &languages = translate->getLanguageList();
    entries.reserve(languages.size());
    for (const auto &entry : languages)
        entries.push_back({entry.first, entry.second});

    languageModel.reset(entries, visibleRows);
    currentPage = languageModel.pageContaining(settings != nullptr ? settings->language : translate->getCurrentLanguage());
    configureLegacyLayout(visibleRows + 2, true, LegacyOptionsLayoutPreset::Wide);
    rebuildButtons();
}

void LegacyLanguageOptions::rebuildButtons()
{
    clearControlList();
    const int_t x = legacyLayout.contentX;
    const int_t w = legacyLayout.contentWidth;
    const int_t h = legacyLayout.rowHeight;

    for (int_t row = 0; row < visibleRows; ++row)
    {
        const LegacyLanguageEntry *entry = languageModel.entryAt(currentPage, row);
        if (entry == nullptr)
            continue;

        std::string label = entry->displayName;
        if (settings != nullptr && entry->code == settings->language)
            label = "> " + label;
        if (fontRenderer != nullptr)
            label = fontRenderer->trimStringToWidth(label, w - 8);
        controlList.push_back(new LegacyGuiButton(LANGUAGE_ROW_BASE + row, x,
            legacyLayout.rowY(row), w, h, label));
    }

    const int_t navY = legacyLayout.rowY(visibleRows);
    const int_t gap = 2;
    const int_t halfWidth = (w - gap) / 2;
    LegacyGuiButton *previous = new LegacyGuiButton(BUTTON_PREVIOUS, x, navY, halfWidth, h, uiText("Previous"));
    LegacyGuiButton *next = new LegacyGuiButton(BUTTON_NEXT, x + halfWidth + gap, navY, w - halfWidth - gap, h, uiText("Next"));
    previous->enabled = currentPage > 0;
    next->enabled = currentPage + 1 < languageModel.pageCount();
    controlList.push_back(previous);
    controlList.push_back(next);
    controlList.push_back(new LegacyGuiButton(BUTTON_DONE, x, legacyLayout.rowY(visibleRows + 1), w, h, uiText("Done")));
}

void LegacyLanguageOptions::applyLanguage(const LegacyLanguageEntry &entry)
{
    StringTranslate *translate = StringTranslate::getInstance();
    translate->setLanguage(entry.code);
    if (mc != nullptr && mc->fontRenderer != nullptr)
        mc->fontRenderer->setUnicodeFlag(translate->isUnicode());
    if (settings != nullptr)
    {
        settings->language = translate->getCurrentLanguage();
        settings->saveOptions();
    }
    if (fontRenderer != nullptr)
        fontRenderer->setBidiFlag(StringTranslate::isBidirectional(entry.code));
    rebuildButtons();
}

void LegacyLanguageOptions::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

    if (button->id >= LANGUAGE_ROW_BASE && button->id < LANGUAGE_ROW_BASE + visibleRows)
    {
        const LegacyLanguageEntry *entry = languageModel.entryAt(currentPage, button->id - LANGUAGE_ROW_BASE);
        if (entry != nullptr)
            applyLanguage(*entry);
        return;
    }

    if (button->id == BUTTON_PREVIOUS)
    {
        currentPage = languageModel.clampPage(currentPage - 1);
        rebuildButtons();
        return;
    }
    if (button->id == BUTTON_NEXT)
    {
        currentPage = languageModel.clampPage(currentPage + 1);
        rebuildButtons();
        return;
    }
    if (button->id == BUTTON_DONE)
    {
        returnToParent();
        return;
    }
}

void LegacyLanguageOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawLegacyBackground(partialTick);
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
