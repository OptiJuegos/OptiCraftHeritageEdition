#include "GuiSlot.h"
#include "GuiButton.h"
#include "Tessellator.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "platform/RenderAPI.h"
#include "pc/lwjgl/Mouse.h"
#include "java/Arithmetic.h"
#include "java/System.h"

GuiSlot::GuiSlot(Minecraft *minecraft, int_t w, int_t h, int_t t, int_t b, int_t slotH)
	: mc(minecraft)
	, width(w)
	, height(h)
	, top(t)
	, bottom(b)
	, posZ(slotH)
	, right(w)
	, scrollUpButtonID(-1)
	, scrollDownButtonID(-1)
	, initialClickY(-2.0f)
	, scrollMultiplier(1.0f)
	, amountScrolled(0.0f)
	, selectedElement(-1)
	, lastClicked(0LL)
	, showSelectionBox(true)
	, hasHeader(false)
	, headerPadding(0)
{
}

void GuiSlot::setShowSelectionBox(bool show)
{
	showSelectionBox = show;
}

void GuiSlot::setHasHeader(bool flag, int_t padding)
{
	hasHeader = flag;
	headerPadding = padding;
	if (!flag) headerPadding = 0;
}

int_t GuiSlot::getContentHeight()
{
	return getSize() * posZ + headerPadding;
}

void GuiSlot::drawHeader(int_t x, int_t y, Tessellator *tess)
{
}

void GuiSlot::clickedHeader(int_t x, int_t y)
{
}

void GuiSlot::renderHoverState(int_t mouseX, int_t mouseY)
{
}

int_t GuiSlot::getElementAt(int_t mouseX, int_t mouseY)
{
	int_t leftEdge  = width / 2 - 110;
	int_t rightEdge = width / 2 + 110;
	int_t relY = ((mouseY - top - headerPadding) + (int_t)amountScrolled) - 4;
	int_t idx  = relY / posZ;
	if (mouseX >= leftEdge && mouseX <= rightEdge && idx >= 0 && relY >= 0 && idx < getSize())
		return idx;
	return -1;
}

void GuiSlot::registerScrollButtons(std::vector<GuiButton *> &list, int_t upId, int_t downId)
{
	scrollUpButtonID   = upId;
	scrollDownButtonID = downId;
}

void GuiSlot::bindAmountScrolled()
{
	int_t maxScroll = getContentHeight() - (bottom - top - 4);
	if (maxScroll < 0) maxScroll /= 2;
	if (amountScrolled < 0.0f)         amountScrolled = 0.0f;
	if (amountScrolled > (float_t)maxScroll) amountScrolled = (float_t)maxScroll;
}

void GuiSlot::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;
	if (button->id == scrollUpButtonID)
	{
		amountScrolled -= (posZ * 2) / 3;
		initialClickY = -2.0f;
		bindAmountScrolled();
	}
	else if (button->id == scrollDownButtonID)
	{
		amountScrolled += (posZ * 2) / 3;
		initialClickY = -2.0f;
		bindAmountScrolled();
	}
}

void GuiSlot::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	currentMouseX = mouseX;
	currentMouseY = mouseY;
	drawBackground();
	int_t size = getSize();
	int_t scrollBarX  = width / 2 + 124;
	int_t scrollBarX2 = scrollBarX + 6;

	if (lwjgl::Mouse::isButtonDown(0))
	{
		if (initialClickY == -1.0f)
		{
			bool consumed = true;
			if (mouseY >= top && mouseY <= bottom)
			{
				int_t leftEdge  = width / 2 - 110;
				int_t rightEdge = width / 2 + 110;
				int_t relY = ((mouseY - top - headerPadding) + (int_t)amountScrolled) - 4;
				int_t idx  = relY / posZ;
				if (mouseX >= leftEdge && mouseX <= rightEdge && idx >= 0 && relY >= 0 && idx < size)
				{
					bool dblClick = idx == selectedElement && JavaArithmetic::longSub(System::currentTimeMillis(), lastClicked) < 250LL;
					elementClicked(idx, dblClick);
					selectedElement = idx;
					lastClicked = System::currentTimeMillis();
				}
				else if (mouseX >= leftEdge && mouseX <= rightEdge && relY < 0)
				{
					clickedHeader(mouseX - leftEdge, ((mouseY - top) + (int_t)amountScrolled) - 4);
					consumed = false;
				}
				if (mouseX >= scrollBarX && mouseX <= scrollBarX2)
				{
					scrollMultiplier = -1.0f;
					int_t maxScroll = getContentHeight() - (bottom - top - 4);
					if (maxScroll < 1) maxScroll = 1;
					int_t thumbH = ((bottom - top) * (bottom - top)) / getContentHeight();
					if (thumbH < 32) thumbH = 32;
					if (thumbH > bottom - top - 8) thumbH = bottom - top - 8;
					scrollMultiplier /= (float_t)(bottom - top - thumbH) / (float_t)maxScroll;
				}
				else
				{
					scrollMultiplier = 1.0f;
				}
				initialClickY = consumed ? (float_t)mouseY : -2.0f;
			}
			else
			{
				initialClickY = -2.0f;
			}
		}
		else if (initialClickY >= 0.0f)
		{
			amountScrolled -= ((float_t)mouseY - initialClickY) * scrollMultiplier;
			initialClickY = (float_t)mouseY;
		}
	}
	else
	{
		initialClickY = -1.0f;
	}

	bindAmountScrolled();

	renderDisable(RenderCapability::Lighting);
	renderDisable(RenderCapability::Fog);
	Tessellator *tess = &Tessellator::instance;
	renderBindTexture(mc->renderEngine->getTexture("/gui/background.png"));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	float_t f = 32.0f;
	tess->startDrawingQuads();
	tess->setColorOpaque_I(0x202020);
	tess->addVertexWithUV(left, bottom, 0.0, (float_t)left  / f, (float_t)(bottom + (int_t)amountScrolled) / f);
	tess->addVertexWithUV(right, bottom, 0.0, (float_t)right / f, (float_t)(bottom + (int_t)amountScrolled) / f);
	tess->addVertexWithUV(right, top,    0.0, (float_t)right / f, (float_t)(top    + (int_t)amountScrolled) / f);
	tess->addVertexWithUV(left, top,     0.0, (float_t)left  / f, (float_t)(top    + (int_t)amountScrolled) / f);
	tess->draw();

	int_t contentX = width / 2 - 92 - 16;
	int_t contentY = (top + 4) - (int_t)amountScrolled;

	if (hasHeader)
		drawHeader(contentX, contentY, tess);

	for (int_t i = 0; i < size; i++)
	{
		int_t slotY = contentY + i * posZ + headerPadding;
		int_t slotH = posZ - 4;
		if (slotY > bottom || slotY + slotH < top) continue;

		if (showSelectionBox && isSelected(i))
		{
			int_t selL = width / 2 - 110;
			int_t selR = width / 2 + 110;
			renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			renderDisable(RenderCapability::Texture2D);
			tess->startDrawingQuads();
			tess->setColorOpaque_I(0x808080);
			tess->addVertexWithUV(selL, slotY + slotH + 2, 0.0, 0.0, 1.0);
			tess->addVertexWithUV(selR, slotY + slotH + 2, 0.0, 1.0, 1.0);
			tess->addVertexWithUV(selR, slotY - 2,         0.0, 1.0, 0.0);
			tess->addVertexWithUV(selL, slotY - 2,         0.0, 0.0, 0.0);
			tess->setColorOpaque_I(0x000000);
			tess->addVertexWithUV(selL + 1, slotY + slotH + 1, 0.0, 0.0, 1.0);
			tess->addVertexWithUV(selR - 1, slotY + slotH + 1, 0.0, 1.0, 1.0);
			tess->addVertexWithUV(selR - 1, slotY - 1,         0.0, 1.0, 0.0);
			tess->addVertexWithUV(selL + 1, slotY - 1,         0.0, 0.0, 0.0);
			tess->draw();
			renderEnable(RenderCapability::Texture2D);
		}
		drawSlot(i, contentX, slotY, slotH, tess);
	}

	renderDisable(RenderCapability::DepthTest);
	int_t pad = 4;
	overlayBackground(0, top, 255, 255);
	overlayBackground(bottom, height, 255, 255);

	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderDisable(RenderCapability::AlphaTest);
	renderShadeModel(RenderShadeModel::Smooth);
	renderDisable(RenderCapability::Texture2D);
	tess->startDrawingQuads();
	tess->setColorRGBA_I(0x000000, 0);
	tess->addVertexWithUV(left,  top + pad, 0.0, 0.0, 1.0);
	tess->addVertexWithUV(right, top + pad, 0.0, 1.0, 1.0);
	tess->setColorRGBA_I(0x000000, 255);
	tess->addVertexWithUV(right, top,       0.0, 1.0, 0.0);
	tess->addVertexWithUV(left,  top,       0.0, 0.0, 0.0);
	tess->draw();
	tess->startDrawingQuads();
	tess->setColorRGBA_I(0x000000, 255);
	tess->addVertexWithUV(left,  bottom,       0.0, 0.0, 1.0);
	tess->addVertexWithUV(right, bottom,       0.0, 1.0, 1.0);
	tess->setColorRGBA_I(0x000000, 0);
	tess->addVertexWithUV(right, bottom - pad, 0.0, 1.0, 0.0);
	tess->addVertexWithUV(left,  bottom - pad, 0.0, 0.0, 0.0);
	tess->draw();

	int_t maxScroll = getContentHeight() - (bottom - top - 4);
	if (maxScroll > 0)
	{
		int_t thumbH = ((bottom - top) * (bottom - top)) / getContentHeight();
		if (thumbH < 32) thumbH = 32;
		if (thumbH > bottom - top - 8) thumbH = bottom - top - 8;
		int_t thumbY = ((int_t)amountScrolled * (bottom - top - thumbH)) / maxScroll + top;
		if (thumbY < top) thumbY = top;

		tess->startDrawingQuads();
		tess->setColorRGBA_I(0x000000, 255);
		tess->addVertexWithUV(scrollBarX,  bottom, 0.0, 0.0, 1.0);
		tess->addVertexWithUV(scrollBarX2, bottom, 0.0, 1.0, 1.0);
		tess->addVertexWithUV(scrollBarX2, top,    0.0, 1.0, 0.0);
		tess->addVertexWithUV(scrollBarX,  top,    0.0, 0.0, 0.0);
		tess->draw();
		tess->startDrawingQuads();
		tess->setColorRGBA_I(0x808080, 255);
		tess->addVertexWithUV(scrollBarX,  thumbY + thumbH, 0.0, 0.0, 1.0);
		tess->addVertexWithUV(scrollBarX2, thumbY + thumbH, 0.0, 1.0, 1.0);
		tess->addVertexWithUV(scrollBarX2, thumbY,          0.0, 1.0, 0.0);
		tess->addVertexWithUV(scrollBarX,  thumbY,          0.0, 0.0, 0.0);
		tess->draw();
		tess->startDrawingQuads();
		tess->setColorRGBA_I(0xc0c0c0, 255);
		tess->addVertexWithUV(scrollBarX,      (thumbY + thumbH) - 1, 0.0, 0.0, 1.0);
		tess->addVertexWithUV(scrollBarX2 - 1, (thumbY + thumbH) - 1, 0.0, 1.0, 1.0);
		tess->addVertexWithUV(scrollBarX2 - 1, thumbY,                0.0, 1.0, 0.0);
		tess->addVertexWithUV(scrollBarX,      thumbY,                0.0, 0.0, 0.0);
		tess->draw();
	}

	renderHoverState(mouseX, mouseY);
	renderEnable(RenderCapability::Texture2D);
	renderShadeModel(RenderShadeModel::Flat);
	renderEnable(RenderCapability::AlphaTest);
	renderDisable(RenderCapability::Blend);
}

void GuiSlot::overlayBackground(int_t topY, int_t bottomY, int_t alphaTop, int_t alphaBottom)
{
	Tessellator *tess = &Tessellator::instance;
	renderBindTexture(mc->renderEngine->getTexture("/gui/background.png"));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	float_t f = 32.0f;
	tess->startDrawingQuads();
	tess->setColorRGBA_I(0x404040, alphaBottom);
	tess->addVertexWithUV(0,     bottomY, 0.0, 0.0,              (float_t)bottomY / f);
	tess->addVertexWithUV(width, bottomY, 0.0, (float_t)width / f, (float_t)bottomY / f);
	tess->setColorRGBA_I(0x404040, alphaTop);
	tess->addVertexWithUV(width, topY,    0.0, (float_t)width / f, (float_t)topY    / f);
	tess->addVertexWithUV(0,     topY,    0.0, 0.0,              (float_t)topY    / f);
	tess->draw();
}
