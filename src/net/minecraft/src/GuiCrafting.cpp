#include "GuiCrafting.h"
#include "ContainerWorkbench.h"
#include "InventoryPlayer.h"
#include "World.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "StatCollector.h"
#include "EntityPlayerSP.h"
#include "platform/RenderAPI.h"

GuiCrafting::GuiCrafting(InventoryPlayer *player, World *world, int_t x, int_t y, int_t z)
	: GuiContainer(new ContainerWorkbench(player, world, x, y, z), true)
{
}

void GuiCrafting::onGuiClosed()
{
	GuiContainer::onGuiClosed();
}

void GuiCrafting::drawGuiContainerForegroundLayer()
{
	fontRenderer->drawString(StatCollector::translateToLocal("container.crafting"),  28, 6,             0x404040);
	fontRenderer->drawString(StatCollector::translateToLocal("container.inventory"),  8, (ySize - 96) + 2, 0x404040);
}

void GuiCrafting::drawGuiContainerBackgroundLayer(float_t partialTick)
{
	int_t tex = mc->renderEngine->getTexture("/gui/crafting.png");
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(tex);
	int_t guiX = (width  - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	drawTexturedModalRect(guiX, guiY, 0, 0, xSize, ySize);
}
