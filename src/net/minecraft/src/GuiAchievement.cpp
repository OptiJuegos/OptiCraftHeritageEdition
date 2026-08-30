#include "GuiAchievement.h"
#include "Minecraft.h"
#include "Achievement.h"
#include "RenderItem.h"
#include "RenderHelper.h"
#include "RenderEngine.h"
#include "ScaledResolution.h"
#include "FontRenderer.h"
#include "StatCollector.h"
#include "platform/RenderAPI.h"
#include "java/Arithmetic.h"
#include "java/System.h"

GuiAchievement::GuiAchievement(Minecraft *minecraft)
	: theGame(minecraft)
	, achievementWindowWidth(0)
	, achievementWindowHeight(0)
	, theAchievement(nullptr)
	, field_25083_f(0LL)
	, itemRender(new RenderItem())
	, field_27103_i(false)
{
}

GuiAchievement::~GuiAchievement()
{
	delete itemRender;
	itemRender = nullptr;
}

void GuiAchievement::queueTakenAchievement(Achievement *achievement)
{
	field_25085_d = StatCollector::translateToLocal("achievement.get");
	field_25084_e = achievement->statName;
	field_25083_f = System::currentTimeMillis();
	theAchievement = achievement;
	field_27103_i = false;
}

void GuiAchievement::queueAchievementInformation(Achievement *achievement)
{
	field_25085_d = achievement->statName;
	field_25084_e = achievement->getDescription();
	field_25083_f = JavaArithmetic::longSub(System::currentTimeMillis(), 2500LL);
	theAchievement = achievement;
	field_27103_i = true;
}

void GuiAchievement::updateAchievementWindowScale()
{
	renderViewport(0, 0, theGame->displayWidth, theGame->displayHeight);
	renderMatrixMode(RenderMatrixMode::Projection);
	renderLoadIdentity();
	renderMatrixMode(RenderMatrixMode::ModelView);
	renderLoadIdentity();
	achievementWindowWidth  = theGame->displayWidth;
	achievementWindowHeight = theGame->displayHeight;
	ScaledResolution scaledresolution(theGame->gameSettings, theGame->displayWidth, theGame->displayHeight);
	achievementWindowWidth  = scaledresolution.getScaledWidth();
	achievementWindowHeight = scaledresolution.getScaledHeight();
	renderClear(RenderClearMask::Depth);
	renderMatrixMode(RenderMatrixMode::Projection);
	renderLoadIdentity();
	renderOrtho(0.0, achievementWindowWidth, achievementWindowHeight, 0.0, 1000.0, 3000.0);
	renderMatrixMode(RenderMatrixMode::ModelView);
	renderLoadIdentity();
	renderTranslate(0.0f, 0.0f, -2000.0f);
}

void GuiAchievement::updateAchievementWindow()
{
	if (Minecraft::hasPaidCheckTime > 0LL)
	{
		renderDisable(RenderCapability::DepthTest);
		renderDepthMask(false);
		RenderHelper::disableStandardItemLighting();
		updateAchievementWindowScale();
		std::string s  = "Minecraft 1.2.5   Unlicensed Copy :(";
		std::string s1 = "(Or logged in from another location)";
		std::string s2 = "Purchase at minecraft.net";
		theGame->fontRenderer->drawStringWithShadow(s,  2,  2, 0xffffff);
		theGame->fontRenderer->drawStringWithShadow(s1, 2, 11, 0xffffff);
		theGame->fontRenderer->drawStringWithShadow(s2, 2, 20, 0xffffff);
		renderDepthMask(true);
		renderEnable(RenderCapability::DepthTest);
	}
	if (theAchievement == nullptr || field_25083_f == 0LL)
	{
		return;
	}
	float d = (float)JavaArithmetic::longSub(System::currentTimeMillis(), field_25083_f) / 3000.0f;
	if (!field_27103_i && (d < 0.0f || d > 1.0f))
	{
		field_25083_f = 0LL;
		return;
	}
	updateAchievementWindowScale();
	renderDisable(RenderCapability::DepthTest);
	renderDepthMask(false);
	float d1 = d * 2.0f;
	if (d1 > 1.0f) d1 = 2.0f - d1;
	d1 *= 4.0f;
	d1  = 1.0f - d1;
	if (d1 < 0.0f) d1 = 0.0f;
	d1 *= d1;
	d1 *= d1;
	int_t i = achievementWindowWidth - 160;
	int_t j = 0 - (int_t)(d1 * 36.0f);
	int_t k = theGame->renderEngine->getTexture("/achievement/bg.png");
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderEnable(RenderCapability::Texture2D);
	renderBindTexture(k);
	renderDisable(RenderCapability::Lighting);
	drawTexturedModalRect(i, j, 96, 202, 160, 32);
	if (field_27103_i)
	{
		theGame->fontRenderer->drawSplitString(field_25084_e, i + 30, j + 7, 120, -1);
	}
	else
	{
		theGame->fontRenderer->drawString(field_25085_d, i + 30, j + 7, -256);
		theGame->fontRenderer->drawString(field_25084_e, i + 30, j + 18, -1);
	}
	renderPushMatrix();
	renderRotate(180.0f, 1.0f, 0.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	renderPopMatrix();
	renderDisable(RenderCapability::Lighting);
	renderEnable(RenderCapability::RescaleNormal);
	renderEnable(RenderCapability::ColorMaterial);
	renderEnable(RenderCapability::Lighting);
	itemRender->renderItemIntoGUI(theGame->fontRenderer, theGame->renderEngine, theAchievement->theItemStack, i + 8, j + 8);
	renderDisable(RenderCapability::Lighting);
	renderDepthMask(true);
	renderEnable(RenderCapability::DepthTest);
}
