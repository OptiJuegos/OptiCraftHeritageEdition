#pragma once

#include "java/Type.h"
#include <vector>

class Minecraft;
class GuiButton;
class Tessellator;

// net.minecraft.src.GuiSlot
class GuiSlot
{
public:
	GuiSlot(Minecraft *mc, int_t width, int_t height, int_t top, int_t bottom, int_t slotHeight);
	virtual ~GuiSlot() = default;

	void setShowSelectionBox(bool show);

protected:
	void setHasHeader(bool hasHeader, int_t headerHeight);

	virtual int_t getSize() = 0;
	virtual void elementClicked(int_t index, bool doubleClicked) = 0;
	virtual bool isSelected(int_t index) = 0;
	virtual int_t getContentHeight();
	virtual void drawBackground() = 0;
	virtual void drawSlot(int_t index, int_t x, int_t y, int_t height, Tessellator *tess) = 0;
	virtual void drawHeader(int_t x, int_t y, Tessellator *tess);
	virtual void clickedHeader(int_t x, int_t y);
	virtual void renderHoverState(int_t mouseX, int_t mouseY);

public:
	int_t getElementAt(int_t mouseX, int_t mouseY);
	void registerScrollButtons(std::vector<GuiButton *> &list, int_t upId, int_t downId);
	void actionPerformed(GuiButton *button);
	void scrollToElement(int_t index);
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick);

private:
	void bindAmountScrolled();
	void overlayBackground(int_t top, int_t bottom, int_t alphaTop, int_t alphaBottom);

protected:
	Minecraft *mc;
	const int_t width;
	const int_t height;
	const int_t top;
	const int_t bottom;
	const int_t posZ;   // slot height
	int_t currentMouseX = 0;
	int_t currentMouseY = 0;

private:
	const int_t right;
	const int_t left = 0;
	int_t scrollUpButtonID;
	int_t scrollDownButtonID;
	float_t initialClickY;
	float_t scrollMultiplier;
	float_t amountScrolled;
	int_t selectedElement;
	long_t lastClicked;
	bool showSelectionBox;
	bool hasHeader;
	int_t headerPadding;
};
