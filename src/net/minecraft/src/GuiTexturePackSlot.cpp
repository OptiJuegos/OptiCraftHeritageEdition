#include "GuiTexturePackSlot.h"
#include "GuiTexturePacks.h"
#include "TexturePackList.h"
#include "TexturePackBase.h"
#include "RenderEngine.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"

GuiTexturePackSlot::GuiTexturePackSlot(GuiTexturePacks *guitexturepacks)
	: GuiSlot(guitexturepacks->mc, guitexturepacks->width, guitexturepacks->height, 32, (guitexturepacks->height - 55) + 4, 36)
	, parentTexturePackGui(guitexturepacks)
{
}

int_t GuiTexturePackSlot::getSize()
{
	return (int_t)parentTexturePackGui->mc->texturePackList->getAvailableTexturePacks().size();
}

void GuiTexturePackSlot::elementClicked(int_t i, bool flag)
{
	const auto &list = parentTexturePackGui->mc->texturePackList->getAvailableTexturePacks();
	parentTexturePackGui->mc->texturePackList->setTexturePack((TexturePackBase *)list[i]);
	parentTexturePackGui->mc->refreshResources();
}

bool GuiTexturePackSlot::isSelected(int_t i)
{
	const auto &list = parentTexturePackGui->mc->texturePackList->getAvailableTexturePacks();
	return parentTexturePackGui->mc->texturePackList->getSelectedTexturePack() == list[i];
}

int_t GuiTexturePackSlot::getContentHeight()
{
	return getSize() * 36;
}

void GuiTexturePackSlot::drawBackground()
{
	parentTexturePackGui->drawDefaultBackground();
}

void GuiTexturePackSlot::drawSlot(int_t i, int_t x, int_t y, int_t h, Tessellator *tess)
{
	const auto &list = parentTexturePackGui->mc->texturePackList->getAvailableTexturePacks();
	TexturePackBase *texturepackbase = (TexturePackBase *)list[i];
	texturepackbase->bindThumbnailTexture(parentTexturePackGui->mc);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	tess->startDrawingQuads();
	tess->setColorOpaque_I(0xffffff);
	tess->addVertexWithUV(x,      y + h, 0.0, 0.0, 1.0);
	tess->addVertexWithUV(x + 32, y + h, 0.0, 1.0, 1.0);
	tess->addVertexWithUV(x + 32, y,     0.0, 1.0, 0.0);
	tess->addVertexWithUV(x,      y,     0.0, 0.0, 0.0);
	tess->draw();
	parentTexturePackGui->drawString(parentTexturePackGui->fontRenderer, texturepackbase->texturePackFileName,    x + 32 + 2, y + 1,       0xffffff);
	parentTexturePackGui->drawString(parentTexturePackGui->fontRenderer, texturepackbase->firstDescriptionLine,  x + 32 + 2, y + 12,      0x808080);
	parentTexturePackGui->drawString(parentTexturePackGui->fontRenderer, texturepackbase->secondDescriptionLine, x + 32 + 2, y + 12 + 10, 0x808080);
}
