#pragma once

#include "GuiSlot.h"
#include <map>
#include <string>
#include <vector>

class GuiLanguage;

// net.minecraft.src.GuiSlotLanguage
class GuiSlotLanguage : public GuiSlot
{
public:
    explicit GuiSlotLanguage(GuiLanguage *languageGui);

protected:
    int_t getSize() override;
    void elementClicked(int_t index, bool doubleClicked) override;
    bool isSelected(int_t index) override;
    int_t getContentHeight() override;
    void drawBackground() override;
    void drawSlot(int_t index, int_t x, int_t y, int_t height, Tessellator *tess) override;

private:
    GuiLanguage *languageGui;
    std::vector<std::string> languageCodes;
    std::map<std::string, std::string> languages;
};
