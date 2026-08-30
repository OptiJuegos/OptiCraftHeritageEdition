#include "GuiChest.h"
#include "ContainerChest.h"
#include "IInventory.h"
#include "InventoryLargeChest.h"
#include "InventoryBasic.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "platform/RenderAPI.h"

GuiChest::GuiChest(IInventory *upper, IInventory *lower)
	: GuiContainer(new ContainerChest(upper, lower, (dynamic_cast<InventoryLargeChest *>(lower) != nullptr || dynamic_cast<InventoryBasic *>(lower) != nullptr)), true)
	, upperChestInventory(upper)
	, lowerChestInventory(lower)
	, inventoryRows(0)
{
	field_948_f = false;
	inventoryRows = lower->getSizeInventory() / 9;
	// 0xDE - 108 = 110; ySize base = 110 + rows*18
	ySize = 110 + inventoryRows * 18;
}

void GuiChest::drawGuiContainerForegroundLayer()
{
	fontRenderer->drawString(lowerChestInventory->getInvName(), 8, 6, 0x404040);
	fontRenderer->drawString(upperChestInventory->getInvName(), 8, (ySize - 96) + 2, 0x404040);
}

void GuiChest::drawGuiContainerBackgroundLayer(float_t partialTick)
{
	int_t tex = mc->renderEngine->getTexture("/gui/container.png");
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(tex);
	int_t guiX = (width  - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	drawTexturedModalRect(guiX, guiY,                             0,   0,   xSize, inventoryRows * 18 + 17);
	drawTexturedModalRect(guiX, guiY + inventoryRows * 18 + 17,   0, 126,   xSize, 96);
}
