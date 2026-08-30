#include "GuiInventory.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "AchievementList.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "RenderHelper.h"
#include "RenderManager.h"
#include "GuiAchievements.h"
#include "GuiStats.h"
#include "GuiButton.h"
#include "StatCollector.h"
#include "PotionEffect.h"
#include "Potion.h"
#include "PlayerController.h"
#include "GuiContainerCreative.h"
#include "Minecraft.h"
#include "platform/RenderAPI.h"
#include <cmath>


GuiInventory::GuiInventory(EntityPlayer *player)
	: GuiContainer(player->inventorySlots)
	, xSize_lo(0.0f)
	, ySize_lo(0.0f)
{
	field_948_f = true;
	player->addStat(AchievementList::openInventory, 1);
}

void GuiInventory::initGui()
{
	for (GuiButton *button : controlList)
		delete button;
	controlList.clear();
	if (mc->playerController->isInCreativeMode())
	{
		mc->displayGuiScreen(new GuiContainerCreative(mc->thePlayer));
		return;
	}

	GuiContainer::initGui();
	if (!mc->thePlayer->getActivePotionEffects().empty())
		guiLeft = 160 + (width - xSize - 200) / 2;
}

void GuiInventory::updateScreen()
{
	if (mc->playerController->isInCreativeMode())
		mc->displayGuiScreen(new GuiContainerCreative(mc->thePlayer));
}

void GuiInventory::drawGuiContainerForegroundLayer()
{
	fontRenderer->drawString(StatCollector::translateToLocal("container.crafting"), 86, 16, 0x404040);
}

void GuiInventory::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	GuiContainer::drawScreen(mouseX, mouseY, partialTick);
	xSize_lo = (float_t)mouseX;
	ySize_lo = (float_t)mouseY;
}

void GuiInventory::drawGuiContainerBackgroundLayer(float_t partialTick)
{
	int_t tex = mc->renderEngine->getTexture("/gui/inventory.png");
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(tex);
	int_t guiX = guiLeft;
	int_t guiY = guiTop;
	drawTexturedModalRect(guiX, guiY, 0, 0, xSize, ySize);
	displayDebuffEffects();

	renderEnable(RenderCapability::RescaleNormal);
	renderEnable(RenderCapability::ColorMaterial);
	// The 3D player preview needs depth testing to sort its own limbs. The 2D GUI
	// pass reaches here with GL_DEPTH_TEST disabled (drawDefaultBackground leaves it
	// off), so without this the model draws in submission order -- back limbs paint
	// over the front (the "broken z-buffer" look). Clear the depth in this region
	// and enable the test for the model; the slot/item pass below wants it on too,
	// and GuiContainer disables it again before the 2D foreground text.
	renderClear(RenderClearMask::Depth);
	renderEnable(RenderCapability::DepthTest);
	renderPushMatrix();
	renderTranslate((float_t)(guiX + 51), (float_t)(guiY + 75), 50.0f);
	float_t scale = 30.0f;
	renderScale(-scale, scale, scale);
	renderRotate(180.0f, 0.0f, 0.0f, 1.0f);

	float_t savedYawOffset = mc->thePlayer->renderYawOffset;
	float_t savedYaw       = mc->thePlayer->rotationYaw;
	float_t savedPitch     = mc->thePlayer->rotationPitch;
	float_t f5 = (float_t)(guiX + 51) - xSize_lo;
	float_t f6 = (float_t)((guiY + 75) - 50) - ySize_lo;

	renderRotate(135.0f, 0.0f, 1.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	renderRotate(-135.0f, 0.0f, 1.0f, 0.0f);
	renderRotate(-(float_t)std::atan(f6 / 40.0f) * 20.0f, 1.0f, 0.0f, 0.0f);

	mc->thePlayer->renderYawOffset = (float_t)std::atan(f5 / 40.0f) * 20.0f;
	mc->thePlayer->rotationYaw     = (float_t)std::atan(f5 / 40.0f) * 40.0f;
	mc->thePlayer->rotationPitch   = -(float_t)std::atan(f6 / 40.0f) * 20.0f;
	mc->thePlayer->rotationYawHead = mc->thePlayer->rotationYaw;
	mc->thePlayer->entityBrightness = 1.0f;

	renderTranslate(0.0f, mc->thePlayer->yOffset, 0.0f);
	RenderManager::instance->playerViewY = 180.0f;
	RenderManager::instance->renderEntityWithPosYaw(mc->thePlayer, 0.0, 0.0, 0.0, 0.0f, 1.0f);

	mc->thePlayer->entityBrightness = 0.0f;
	mc->thePlayer->renderYawOffset  = savedYawOffset;
	mc->thePlayer->rotationYaw      = savedYaw;
	mc->thePlayer->rotationPitch    = savedPitch;

	renderPopMatrix();
	RenderHelper::disableStandardItemLighting();
	renderDisable(RenderCapability::RescaleNormal);
}

void GuiInventory::displayDebuffEffects()
{
	std::vector<PotionEffect *> effects = mc->thePlayer->getActivePotionEffects();
	if (effects.empty())
		return;

	Potion::initPotions();
	int_t x = guiLeft - 124;
	int_t y = guiTop;
	int_t texture = mc->renderEngine->getTexture("/gui/inventory.png");
	int_t spacing = 33;
	if (effects.size() > 5)
		spacing = 132 / ((int_t)effects.size() - 1);

	for (PotionEffect *effect : effects)
	{
		if (effect == nullptr)
			continue;
		Potion *potion = Potion::getPotion(effect->getPotionID());
		if (potion == nullptr)
			continue;

		renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		mc->renderEngine->bindTexture(texture);
		drawTexturedModalRect(x, y, 0, ySize, 140, 32);
		if (potion->hasStatusIcon())
		{
			int_t icon = potion->getStatusIconIndex();
			drawTexturedModalRect(x + 6, y + 7, (icon % 8) * 18, ySize + 32 + (icon / 8) * 18, 18, 18);
		}

		std::string name = StatCollector::translateToLocal(potion->getName());
		if (effect->getAmplifier() == 1)
			name += " II";
		else if (effect->getAmplifier() == 2)
			name += " III";
		else if (effect->getAmplifier() == 3)
			name += " IV";

		fontRenderer->drawStringWithShadow(name, x + 28, y + 6, 0xffffff);
		fontRenderer->drawStringWithShadow(Potion::getDurationString(*effect), x + 28, y + 16, 0x7f7f7f);
		y += spacing;
	}
}

void GuiInventory::actionPerformed(GuiButton *button)
{
	if (button->id == 0)
	{
		mc->displayGuiScreen(new GuiAchievements(mc->statFileWriter));
	}
	if (button->id == 1)
	{
		mc->displayGuiScreen(new GuiStats(this, mc->statFileWriter));
	}
}
