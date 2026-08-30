#pragma once

#include "GuiSlot.h"

class GuiTexturePacks;
class Tessellator;

// net.minecraft.src.GuiTexturePackSlot
class GuiTexturePackSlot : public GuiSlot
{
public:
	GuiTexturePackSlot(GuiTexturePacks *guitexturepacks);

protected:
	int_t getSize() override;
	void elementClicked(int_t index, bool doubleClicked) override;
	bool isSelected(int_t index) override;
	int_t getContentHeight() override;
	void drawBackground() override;
	void drawSlot(int_t index, int_t x, int_t y, int_t height, Tessellator *tess) override;

	GuiTexturePacks *parentTexturePackGui;
};
