#include "GuiIngame.h"
#include "platform/PlatformTuning.h"
#include "platform/Profiler.h"
#include "java/String.h"
#include "java/Arithmetic.h"
#include "ScaledResolution.h"
#include "EntityRenderer.h"
#include "EntityPlayerSP.h"
#include "GuiPlayerInfo.h"
#include "NetClientHandler.h"
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM)
#include "NetworkManager.h"
#endif
#include "EntityClientPlayerMP.h"
#include "InventoryPlayer.h"
#include "GameSettings.h"
#include "KeyBinding.h"
#include "ItemStack.h"
#include "Block.h"
#include "BlockPortal.h"
#include "RenderEngine.h"
#include "PlayerController.h"
#include "Material.h"
#include "WorldInfo.h"
#include "WorldClient.h"
#include "Potion.h"
#include "FoodStats.h"
#include "RenderHelper.h"
#include "RenderItem.h"
#include "RenderDragon.h"
#include "EntityDragon.h"
#include "FontRenderer.h"
#include "MathHelper.h"
#include "GuiChat.h"
#include "ChatLine.h"
#include "ChatClickData.h"
#include "Tessellator.h"
#include "StringTranslate.h"
#include "Minecraft.h"
#include "legacy/LegacyControlTooltipHud.h"
#include "legacy/LegacyTipHud.h"
#include "legacy/LegacyHudLayout.h"
#if PLATFORM_PC_LEGACY || defined(PS2_PLATFORM)
#include "pc/render/PcLegacyHudCachePolicy.h"
#endif
#if PLATFORM_PC_LEGACY
#include "GLAllocation.h"
#include "pc/tuning/PcLegacyTuning.h"
#endif
#include "java/Random.h"
#include "java/System.h"
#include "java/Runtime.h"
#include "platform/RenderAPI.h"
#include "platform/Input.h"
#include <cmath>
#include <algorithm>
#include <cstdio>


namespace
{
	void appendTexturedModalRect(Tessellator &tessellator, float_t zLevel,
		int_t x, int_t y, int_t texX, int_t texY, int_t w, int_t h)
	{
		constexpr float_t textureScale = 1.0f / 256.0f;
		tessellator.addVertexWithUV(x, y + h, zLevel,
			static_cast<float_t>(texX) * textureScale,
			static_cast<float_t>(texY + h) * textureScale);
		tessellator.addVertexWithUV(x + w, y + h, zLevel,
			static_cast<float_t>(texX + w) * textureScale,
			static_cast<float_t>(texY + h) * textureScale);
		tessellator.addVertexWithUV(x + w, y, zLevel,
			static_cast<float_t>(texX + w) * textureScale,
			static_cast<float_t>(texY) * textureScale);
		tessellator.addVertexWithUV(x, y, zLevel,
			static_cast<float_t>(texX) * textureScale,
			static_cast<float_t>(texY) * textureScale);
	}

#if PLATFORM_PC_LEGACY || defined(PS2_PLATFORM)
	PcLegacyHudStatusState makeHudStatusState(Minecraft *mc)
	{
		PcLegacyHudStatusState state{};
		state.health = mc->thePlayer->health;
		state.prevHealth = mc->thePlayer->prevHealth;
		state.armor = mc->thePlayer->getPlayerArmorValue();
		FoodStats *foodStats = mc->thePlayer->getFoodStats();
		state.foodLevel = foodStats != nullptr ? foodStats->getFoodLevel() : 20;
		state.saturationPositive = foodStats == nullptr || foodStats->getSaturationLevel() > 0.0f;
		state.air = mc->thePlayer->getAir();
		state.underwater = mc->thePlayer->isInsideOfMaterial(Material::water);
		state.poisoned = mc->thePlayer->isPotionActive(Potion::poison);
		state.hungry = mc->thePlayer->isPotionActive(Potion::hunger);
		state.hardcore = mc->theWorld != nullptr && mc->theWorld->getWorldInfo() != nullptr &&
			mc->theWorld->getWorldInfo()->isHardcoreModeEnabled();
		state.flashHearts = (mc->thePlayer->heartsLife / 3) % 2 == 1 && mc->thePlayer->heartsLife >= 10;
		state.regeneration = mc->thePlayer->isPotionActive(Potion::regeneration);
		state.xpCap = mc->thePlayer->xpBarCap();
		state.xpFilled = state.xpCap > 0 ? static_cast<int_t>(mc->thePlayer->experience * 183.0f) : 0;
		return state;
	}
#endif

	void resetOverlayGLState()
	{
		renderMatrixMode(RenderMatrixMode::Texture);
		renderLoadIdentity();
		renderMatrixMode(RenderMatrixMode::ModelView);

		renderDisable(RenderCapability::Lighting);
		renderDisable(RenderCapability::Fog);
		renderDisable(RenderCapability::CullFace);
		renderDisable(RenderCapability::RescaleNormal);

		renderEnable(RenderCapability::Texture2D);
		renderEnable(RenderCapability::AlphaTest);
		renderEnable(RenderCapability::Blend);
		renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

		renderEnable(RenderCapability::DepthTest);
		renderDepthFunc(RenderCompare::LessEqual);
		renderDepthMask(true);
		renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void finishOverlayGLState()
	{
		renderMatrixMode(RenderMatrixMode::Texture);
		renderLoadIdentity();
		renderMatrixMode(RenderMatrixMode::ModelView);

		renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
		renderEnable(RenderCapability::Texture2D);
		renderEnable(RenderCapability::AlphaTest);
		renderDisable(RenderCapability::Blend);
		renderDepthFunc(RenderCompare::LessEqual);
		renderDepthMask(true);
		renderEnable(RenderCapability::DepthTest);
	}
}

// HSB to RGB helper (Java Color.HSBtoRGB equivalent)
static int_t hsbToRgb(float_t hue, float_t sat, float_t bri)
{
	float_t r = bri, g = bri, b = bri;
	if (sat != 0.0f)
	{
		float_t h = (hue - std::floor(hue)) * 6.0f;
		float_t f = h - std::floor(h);
		float_t p = bri * (1.0f - sat);
		float_t q = bri * (1.0f - sat * f);
		float_t t = bri * (1.0f - (sat * (1.0f - f)));
		switch ((int)h)
		{
		case 0: r = bri; g = t;   b = p;   break;
		case 1: r = q;   g = bri; b = p;   break;
		case 2: r = p;   g = bri; b = t;   break;
		case 3: r = p;   g = q;   b = bri; break;
		case 4: r = t;   g = p;   b = bri; break;
		case 5: r = bri; g = p;   b = q;   break;
		}
	}
	int_t ri = (int_t)(r * 255.0f + 0.5f);
	int_t gi = (int_t)(g * 255.0f + 0.5f);
	int_t bi = (int_t)(b * 255.0f + 0.5f);
	return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

#ifdef PS2_PLATFORM
struct Ps2HudCache
{
	RenderStaticMesh hotbar;
	RenderStaticMesh crosshair;
	RenderStaticMesh status;
	int_t hotbarWidth = -1;
	int_t hotbarHeight = -1;
	int_t hotbarItem = -1;
	int_t crosshairWidth = -1;
	int_t crosshairHeight = -1;
	int_t statusWidth = -1;
	int_t statusHeight = -1;
	bool hotbarValid = false;
	bool crosshairValid = false;
	bool statusValid = false;
	unsigned long long statusSignature = 0;
};
#endif

RenderItem *GuiIngame::itemRenderer = new RenderItem();

GuiIngame::GuiIngame(Minecraft *minecraft)
	: mc(minecraft)
	, rand(new Random())
	, field_933_a("")
	, updateCounter(0)
	, recordPlaying("")
	, recordPlayingUpFor(0)
	, field_22065_l(false)
	, chatScroll(0)
	, isScrolled(false)
#if PLATFORM_PC_LEGACY
	, pcLegacyHudDisplayLists(0)
	, pcLegacyHudWidth(-1)
	, pcLegacyHudHeight(-1)
	, pcLegacyHotbarItem(-1)
	, pcLegacyHotbarValid(false)
	, pcLegacyCrosshairValid(false)
	, pcLegacyStatusValid(false)
	, pcLegacyStatusSignature(0)
#endif
#ifdef PS2_PLATFORM
	, ps2HudCache(new Ps2HudCache())
#endif
	, damageGuiPartialTime(0.0f)
	, prevVignetteBrightness(1.0f)
{
#ifdef PS2_PLATFORM
	renderStaticMeshCreate(ps2HudCache->hotbar);
	renderStaticMeshCreate(ps2HudCache->crosshair);
	renderStaticMeshCreate(ps2HudCache->status);
#endif
}

GuiIngame::~GuiIngame()
{
#if PLATFORM_PC_LEGACY
	if (pcLegacyHudDisplayLists != 0)
	{
		GLAllocation::deleteDisplayLists(pcLegacyHudDisplayLists);
		pcLegacyHudDisplayLists = 0;
	}
#endif
#ifdef PS2_PLATFORM
	if (ps2HudCache != nullptr)
	{
		renderStaticMeshDestroy(ps2HudCache->hotbar);
		renderStaticMeshDestroy(ps2HudCache->crosshair);
		renderStaticMeshDestroy(ps2HudCache->status);
		delete ps2HudCache;
		ps2HudCache = nullptr;
	}
#endif
	clearChatMessages();
	delete rand;
	rand = nullptr;
}

void GuiIngame::renderFpsOverlay(FontRenderer *fontRenderer)
{
	if (fontRenderer == nullptr || mc == nullptr)
		return;

	std::string fpsLine = mc->debug;
	const std::size_t comma = fpsLine.find(',');
	if (comma != std::string::npos)
		fpsLine.resize(comma);
	if (fpsLine.empty())
		fpsLine = "0 fps";

#ifdef PS2_PLATFORM
	fontRenderer->drawString(fpsLine, 2, 2, 0xe0e0e0);
#else
	fontRenderer->drawStringWithShadow(fpsLine, 2, 2, 0xffffff);
#endif
}

void GuiIngame::renderDebugOverlay(FontRenderer *fontRenderer, int_t screenWidth)
{
	renderPushMatrix();
	if (Minecraft::hasPaidCheckTime > 0LL)
		renderTranslate(0.0f, 32.0f, 0.0f);

#ifdef PS2_PLATFORM
	(void)screenWidth;
	const int_t color = 0xe0e0e0;
	fontRenderer->beginTextBatch();
	fontRenderer->drawString("OptiCraft (" + mc->debug + ")", 2, 2, color);
	fontRenderer->drawString(mc->getDebugLine1(), 2, 12, color);
	fontRenderer->drawString(mc->getDebugLine2(), 2, 22, color);
	fontRenderer->drawString(mc->getDebugLine3(), 2, 32, color);

	Runtime &runtime = Runtime::getRuntime();
	const long_t maxMemory = runtime.maxMemory();
	const long_t usedMemory = runtime.totalMemory() - runtime.freeMemory();
	char performanceLine[80];
	std::snprintf(performanceLine, sizeof(performanceLine),
		"CPU:%d%% GPU:%d%% MEM:%lld/%lldMB",
		(int_t)(mc->cpuUsagePercent + 0.5f),
		(int_t)(mc->gpuUsagePercent + 0.5f),
		(long long)(usedMemory / 1024LL / 1024LL),
		(long long)(maxMemory / 1024LL / 1024LL));
	fontRenderer->drawString(performanceLine, 2, 42, color);

	char positionLine[80];
	std::snprintf(positionLine, sizeof(positionLine), "XYZ: %d %d %d",
		MathHelper::floor_double(mc->thePlayer->posX),
		MathHelper::floor_double(mc->thePlayer->posY),
		MathHelper::floor_double(mc->thePlayer->posZ));
	fontRenderer->drawString(positionLine, 2, 52, color);
	fontRenderer->endTextBatch();
#else
	fontRenderer->drawStringWithShadow("OptiCraft (" + mc->debug + ")", 2, 2, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine1(), 2, 12, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine2(), 2, 22, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine3(), 2, 32, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine4(), 2, 42, 0xffffff);
	std::string cpuGpuLine = "CPU: " + std::to_string((int_t)(mc->cpuUsagePercent + 0.5f)) + "% GPU: "
	    + std::to_string((int_t)(mc->gpuUsagePercent + 0.5f)) + "%";
	fontRenderer->drawStringWithShadow(cpuGpuLine, 2, 52, 0xffffff);
	Runtime &runtime = Runtime::getRuntime();
	long_t maxMemory = runtime.maxMemory();
	long_t totalMemory = runtime.totalMemory();
	long_t freeMemory = runtime.freeMemory();
	long_t usedMemory = totalMemory - freeMemory;
	std::string memoryUsed = "Used memory: " + std::to_string((usedMemory * 100LL) / maxMemory) + "% ("
	    + std::to_string(usedMemory / 1024LL / 1024LL) + "MB) of "
	    + std::to_string(maxMemory / 1024LL / 1024LL) + "MB";
	drawString(fontRenderer, memoryUsed, screenWidth - fontRenderer->getStringWidth(memoryUsed) - 2, 2, 0xe0e0e0);
	std::string memoryAllocated = "Allocated memory: " + std::to_string((totalMemory * 100LL) / maxMemory) + "% ("
	    + std::to_string(totalMemory / 1024LL / 1024LL) + "MB)";
	drawString(fontRenderer, memoryAllocated, screenWidth - fontRenderer->getStringWidth(memoryAllocated) - 2, 12, 0xe0e0e0);
	drawString(fontRenderer, "x: " + std::to_string(mc->thePlayer->posX), 2, 64, 0xe0e0e0);
	drawString(fontRenderer, "y: " + std::to_string(mc->thePlayer->posY), 2, 72, 0xe0e0e0);
	drawString(fontRenderer, "z: " + std::to_string(mc->thePlayer->posZ), 2, 80, 0xe0e0e0);
	drawString(fontRenderer, "f: " + std::to_string(MathHelper::floor_float((mc->thePlayer->rotationYaw * 4.0f) / 360.0f + 0.5f) & 3), 2, 88, 0xe0e0e0);
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM)
	drawString(fontRenderer, platformInputDebugLine(), 2, 96, 0xe0e0e0);
	WorldClient *multiplayerWorld = dynamic_cast<WorldClient *>(mc->theWorld);
	if (multiplayerWorld != nullptr)
	{
		char multiplayerLine[160];
		std::snprintf(multiplayerLine, sizeof(multiplayerLine),
			"MP cache:%zu %zuKB ev:%lu pr:%lu pend:%zu E:%zu/%zu/%zu ep:%lu",
			multiplayerWorld->getDeferredChunkCount(),
			multiplayerWorld->getDeferredChunkBytes() / 1024u,
			(unsigned long)multiplayerWorld->getDeferredChunkEvictions(),
			(unsigned long)multiplayerWorld->getDeferredChunkPromotions(),
			multiplayerWorld->getDeferredPromotionPendingCount(),
			multiplayerWorld->getPendingEntitySpawnCount(),
			multiplayerWorld->getKnownEntityCount(),
			multiplayerWorld->getLoadedEntityList().size(),
			(unsigned long)multiplayerWorld->getDeferredEntityChunkPromotions());
		drawString(fontRenderer, multiplayerLine, 2, 106, 0xe0e0e0);

		EntityClientPlayerMP *multiplayerPlayer = dynamic_cast<EntityClientPlayerMP *>(mc->thePlayer);
		NetClientHandler *handler = multiplayerPlayer != nullptr ? multiplayerPlayer->sendQueue : nullptr;
		NetworkManager *networkManager = handler != nullptr ? handler->getNetworkManager() : nullptr;
		if (handler != nullptr && networkManager != nullptr)
		{
			char packetLine[112];
			std::snprintf(packetLine, sizeof(packetLine),
				"NET 50+:%lu 50-:%lu 51:%lu q:%zu/%zuKB rxE:%u",
				handler->getPreChunkLoadCount(),
				handler->getPreChunkUnloadCount(),
				handler->getMapChunkCount(),
				networkManager->getReadQueuePacketCount(),
				networkManager->getReadQueueByteLength() / 1024u,
				networkManager->getReceivedEntityPacketCount());
			drawString(fontRenderer, packetLine, 2, 116, 0xe0e0e0);

			char socketLine[112];
			std::snprintf(socketLine, sizeof(socketLine),
				"TCP rx:%zuKB tx:%zuKB rd:%d wr:%d",
				networkManager->getSocketReceivedByteCount() / 1024u,
				networkManager->getSocketSentByteCount() / 1024u,
				networkManager->isReadThreadActive() ? 1 : 0,
				networkManager->isWriteThreadActive() ? 1 : 0);
			drawString(fontRenderer, socketLine, 2, 126, 0xe0e0e0);
		}
	}
#endif
#endif
	renderPopMatrix();
}

void GuiIngame::renderBossHealth()
{
	EntityDragon* dragon = RenderDragon::entityDragon;
	if (dragon == nullptr)
		return;

	RenderDragon::entityDragon = nullptr;
	FontRenderer* fontRenderer = mc->fontRenderer;
	ScaledResolution resolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	const int_t screenWidth = resolution.getScaledWidth();
	constexpr int_t barWidth = 182;
	const int_t x = screenWidth / 2 - barWidth / 2;
	const int_t maxHealth = dragon->getMaxHealth();
	const int_t filled = maxHealth > 0
		? static_cast<int_t>(static_cast<float>(dragon->func_41010_ax()) / static_cast<float>(maxHealth) * static_cast<float>(barWidth + 1))
		: 0;
	constexpr int_t y = 12;

	drawTexturedModalRect(x, y, 0, 74, barWidth, 5);
	drawTexturedModalRect(x, y, 0, 74, barWidth, 5);
	if (filled > 0)
		drawTexturedModalRect(x, y, 0, 79, filled, 5);

	const std::string name = "Boss health";
	fontRenderer->drawStringWithShadow(name, screenWidth / 2 - fontRenderer->getStringWidth(name) / 2, y - 10, 0xff00ff);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
}

void GuiIngame::renderPlayerStatusHudGeometry(int_t sw, int_t sh, Tessellator *captureTessellator)
{
	auto emitRect = [&](int_t x, int_t y, int_t texX, int_t texY, int_t w, int_t h)
	{
		if (captureTessellator != nullptr)
		{
			appendTexturedModalRect(*captureTessellator, zLevel, x, y, texX, texY, w, h);
			return;
		}
		drawTexturedModalRect(x, y, texX, texY, w, h);
	};

	bool flashHearts = (mc->thePlayer->heartsLife / 3) % 2 == 1;
	if (mc->thePlayer->heartsLife < 10)
		flashHearts = false;
	const int_t health = mc->thePlayer->health;
	const int_t prevHealth = mc->thePlayer->prevHealth;
	const int_t seed = JavaArithmetic::intFromBits(static_cast<uint_t>(updateCounter) * 0x4c627u);
	rand->setSeed(static_cast<long_t>(seed));
	const int_t left = sw / 2 - 91;
	const int_t right = sw / 2 + 91;
	const int_t xpCap = mc->thePlayer->xpBarCap();
	if (xpCap > 0)
	{
		constexpr int_t XP_BAR_WIDTH = 182;
		const int_t filled = static_cast<int_t>(mc->thePlayer->experience * static_cast<float_t>(XP_BAR_WIDTH + 1));
		const int_t xpY = sh - 32 + 3;
		emitRect(left, xpY, 0, 64, XP_BAR_WIDTH, 5);
		if (filled > 0)
			emitRect(left, xpY, 0, 69, filled, 5);
	}

	const int_t healthY = sh - 39;
	const int_t armorY = healthY - 10;
	const int_t armor = mc->thePlayer->getPlayerArmorValue();
	const int_t regenerationHeart = mc->thePlayer->isPotionActive(Potion::regeneration) ? updateCounter % 25 : -1;
	const bool hardcore = mc->theWorld != nullptr && mc->theWorld->getWorldInfo() != nullptr
		&& mc->theWorld->getWorldInfo()->isHardcoreModeEnabled();

	for (int_t index = 0; index < 10; ++index)
	{
		if (armor > 0)
		{
			const int_t armorX = left + index * 8;
			if (index * 2 + 1 < armor)  emitRect(armorX, armorY, 34, 9, 9, 9);
			if (index * 2 + 1 == armor) emitRect(armorX, armorY, 25, 9, 9, 9);
			if (index * 2 + 1 > armor)  emitRect(armorX, armorY, 16, 9, 9, 9);
		}

		int_t heartTextureX = 16;
		if (mc->thePlayer->isPotionActive(Potion::poison))
			heartTextureX += 36;
		const int_t flash = flashHearts ? 1 : 0;
		const int_t hx = left + index * 8;
		int_t hy = healthY;
		if (health <= 4)
			hy += rand->nextInt(2);
		if (index == regenerationHeart)
			hy -= 2;
		const int_t hardcoreRow = hardcore ? 5 : 0;

		emitRect(hx, hy, 16 + flash * 9, 9 * hardcoreRow, 9, 9);
		if (flashHearts)
		{
			if (index * 2 + 1 < prevHealth)  emitRect(hx, hy, heartTextureX + 54, 9 * hardcoreRow, 9, 9);
			if (index * 2 + 1 == prevHealth) emitRect(hx, hy, heartTextureX + 63, 9 * hardcoreRow, 9, 9);
		}
		if (index * 2 + 1 < health)  emitRect(hx, hy, heartTextureX + 36, 9 * hardcoreRow, 9, 9);
		if (index * 2 + 1 == health) emitRect(hx, hy, heartTextureX + 45, 9 * hardcoreRow, 9, 9);
	}

	FoodStats *foodStats = mc->thePlayer->getFoodStats();
	const int_t foodLevel = foodStats != nullptr ? foodStats->getFoodLevel() : 20;
	const float_t saturation = foodStats != nullptr ? foodStats->getSaturationLevel() : 5.0f;
	for (int_t index = 0; index < 10; ++index)
	{
		int_t fy = healthY;
		int_t foodTextureX = 16;
		int_t backgroundOffset = 0;
		if (mc->thePlayer->isPotionActive(Potion::hunger))
		{
			foodTextureX += 36;
			backgroundOffset = 13;
		}
		if (saturation <= 0.0f && updateCounter % (foodLevel * 3 + 1) == 0)
			fy = healthY + (rand->nextInt(3) - 1);
		const int_t fx = right - index * 8 - 9;
		emitRect(fx, fy, 16 + backgroundOffset * 9, 27, 9, 9);
		if (index * 2 + 1 < foodLevel)  emitRect(fx, fy, foodTextureX + 36, 27, 9, 9);
		if (index * 2 + 1 == foodLevel) emitRect(fx, fy, foodTextureX + 45, 27, 9, 9);
	}

	if (mc->thePlayer->isInsideOfMaterial(Material::water))
	{
		const int_t air = mc->thePlayer->getAir();
		const int_t full = JavaArithmetic::floatToInt(std::ceil((static_cast<float_t>(air - 2) * 10.0f) / 300.0f));
		const int_t empty = JavaArithmetic::floatToInt(std::ceil((static_cast<float_t>(air) * 10.0f) / 300.0f)) - full;
		for (int_t index = 0; index < full + empty; ++index)
		{
			const int_t ax = right - index * 8 - 9;
			if (index < full)
				emitRect(ax, armorY, 16, 18, 9, 9);
			else
				emitRect(ax, armorY, 25, 18, 9, 9);
		}
	}
}

void GuiIngame::renderPlayerStatusHudUncached(int_t sw, int_t sh)
{
	renderPlayerStatusHudGeometry(sw, sh, nullptr);
}

#if PLATFORM_PC_LEGACY
void GuiIngame::pcLegacyEnsureHudCacheLists(int_t sw, int_t sh)
{
#if PC_LEGACY_HUD_DISPLAY_LIST_CACHE
	if (pcLegacyHudDisplayLists == 0)
		pcLegacyHudDisplayLists = GLAllocation::generateDisplayLists(3);

	if (pcLegacyHudWidth != sw || pcLegacyHudHeight != sh)
	{
		pcLegacyHudWidth = sw;
		pcLegacyHudHeight = sh;
		pcLegacyHotbarValid = false;
		pcLegacyCrosshairValid = false;
		pcLegacyStatusValid = false;
	}
#else
	(void)sw;
	(void)sh;
#endif
}

void GuiIngame::pcLegacyRenderHotbarFrame(int_t sw, int_t sh, int_t currentItem)
{
#if PC_LEGACY_HUD_DISPLAY_LIST_CACHE
	pcLegacyEnsureHudCacheLists(sw, sh);
	if (pcLegacyHudDisplayLists != 0)
	{
		if (!pcLegacyHotbarValid || pcLegacyHotbarItem != currentItem)
		{
			pcLegacyHotbarItem = currentItem;
			renderBeginDisplayList(pcLegacyHudDisplayLists);
			zLevel = -90.0f;
			drawTexturedModalRect(sw / 2 - 91, sh - 22, 0, 0, 182, 22);
			drawTexturedModalRect((sw / 2 - 91 - 1) + currentItem * 20, sh - 23, 0, 22, 24, 22);
			renderEndDisplayList();
			pcLegacyHotbarValid = true;
		}
		renderCallDisplayList(pcLegacyHudDisplayLists);
		return;
	}
#endif
	zLevel = -90.0f;
	drawTexturedModalRect(sw / 2 - 91, sh - 22, 0, 0, 182, 22);
	drawTexturedModalRect((sw / 2 - 91 - 1) + currentItem * 20, sh - 23, 0, 22, 24, 22);
}

void GuiIngame::pcLegacyRenderCrosshair(int_t sw, int_t sh)
{
#if PC_LEGACY_HUD_DISPLAY_LIST_CACHE
	pcLegacyEnsureHudCacheLists(sw, sh);
	if (pcLegacyHudDisplayLists != 0)
	{
		if (!pcLegacyCrosshairValid)
		{
			renderBeginDisplayList(pcLegacyHudDisplayLists + 1);
			drawTexturedModalRect(sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
			renderEndDisplayList();
			pcLegacyCrosshairValid = true;
		}
		renderCallDisplayList(pcLegacyHudDisplayLists + 1);
		return;
	}
#endif
	drawTexturedModalRect(sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
}

void GuiIngame::pcLegacyRenderPlayerStatusHud(int_t sw, int_t sh)
{
#if PC_LEGACY_HUD_DISPLAY_LIST_CACHE
	const PcLegacyHudStatusState state = makeHudStatusState(mc);

	if (pcLegacyCanCacheHudStatus(state))
	{
		pcLegacyEnsureHudCacheLists(sw, sh);
		const unsigned long long signature = static_cast<unsigned long long>(pcLegacyHudStatusSignature(state));
		if (pcLegacyHudDisplayLists != 0)
		{
			if (!pcLegacyStatusValid || pcLegacyStatusSignature != signature)
			{
				pcLegacyStatusSignature = signature;
				renderBeginDisplayList(pcLegacyHudDisplayLists + 2);
				renderPlayerStatusHudUncached(sw, sh);
				renderEndDisplayList();
				pcLegacyStatusValid = true;
			}
			renderCallDisplayList(pcLegacyHudDisplayLists + 2);
			return;
		}
	}
	else
	{
		pcLegacyStatusValid = false;
	}
#endif
	renderPlayerStatusHudUncached(sw, sh);
}
#endif

#ifdef PS2_PLATFORM
void GuiIngame::ps2RenderHotbarFrame(int_t sw, int_t sh, int_t currentItem)
{
	Ps2HudCache &cache = *ps2HudCache;
	const bool needsCompile = !cache.hotbarValid || cache.hotbarWidth != sw ||
		cache.hotbarHeight != sh || cache.hotbarItem != currentItem;
	if (needsCompile)
	{
		Tessellator &tessellator = Tessellator::instance;
		zLevel = -90.0f;
		tessellator.startDrawingQuads();
		appendTexturedModalRect(tessellator, zLevel, sw / 2 - 91, sh - 22, 0, 0, 182, 22);
		appendTexturedModalRect(tessellator, zLevel,
			(sw / 2 - 91 - 1) + currentItem * 20, sh - 23, 0, 22, 24, 22);
		cache.hotbarValid = tessellator.finishStaticMesh(cache.hotbar);
		if (cache.hotbarValid)
		{
			cache.hotbarWidth = sw;
			cache.hotbarHeight = sh;
			cache.hotbarItem = currentItem;
		}
	}

	if (cache.hotbarValid && renderStaticMeshDraw(cache.hotbar))
		return;

	cache.hotbarValid = false;
	zLevel = -90.0f;
	drawTexturedModalRect(sw / 2 - 91, sh - 22, 0, 0, 182, 22);
	drawTexturedModalRect((sw / 2 - 91 - 1) + currentItem * 20, sh - 23, 0, 22, 24, 22);
}

void GuiIngame::ps2RenderCrosshair(int_t sw, int_t sh)
{
	Ps2HudCache &cache = *ps2HudCache;
	const bool needsCompile = !cache.crosshairValid || cache.crosshairWidth != sw || cache.crosshairHeight != sh;
	if (needsCompile)
	{
		Tessellator &tessellator = Tessellator::instance;
		zLevel = -90.0f;
		tessellator.startDrawingQuads();
		appendTexturedModalRect(tessellator, zLevel, sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
		cache.crosshairValid = tessellator.finishStaticMesh(cache.crosshair);
		if (cache.crosshairValid)
		{
			cache.crosshairWidth = sw;
			cache.crosshairHeight = sh;
		}
	}

	if (cache.crosshairValid && renderStaticMeshDraw(cache.crosshair))
		return;

	cache.crosshairValid = false;
	zLevel = -90.0f;
	drawTexturedModalRect(sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
}

void GuiIngame::ps2RenderPlayerStatusHud(int_t sw, int_t sh)
{
	Ps2HudCache &cache = *ps2HudCache;
	const PcLegacyHudStatusState state = makeHudStatusState(mc);
	if (!pcLegacyCanCacheHudStatus(state))
	{
		cache.statusValid = false;
		renderPlayerStatusHudUncached(sw, sh);
		return;
	}

	const unsigned long long signature = static_cast<unsigned long long>(pcLegacyHudStatusSignature(state));
	const bool needsCompile = !cache.statusValid || cache.statusWidth != sw || cache.statusHeight != sh ||
		cache.statusSignature != signature;
	if (needsCompile)
	{
		Tessellator &tessellator = Tessellator::instance;
		zLevel = -90.0f;
		tessellator.startDrawingQuads();
		renderPlayerStatusHudGeometry(sw, sh, &tessellator);
		cache.statusValid = tessellator.finishStaticMesh(cache.status);
		if (cache.statusValid)
		{
			cache.statusWidth = sw;
			cache.statusHeight = sh;
			cache.statusSignature = signature;
		}
	}

	if (cache.statusValid && renderStaticMeshDraw(cache.status))
		return;

	cache.statusValid = false;
	renderPlayerStatusHudUncached(sw, sh);
}
#endif

void GuiIngame::renderGameOverlay(float_t partialTick, bool showDebug, int_t mouseX, int_t mouseY)
{
	ScaledResolution sr(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	int_t sw = sr.getScaledWidth();
	int_t sh = sr.getScaledHeight();
	FontRenderer *fr = mc->fontRenderer;

	mc->entityRenderer->setupOverlayRendering();
	resetOverlayGLState();

	if (Minecraft::isFancyGraphicsEnabled())
		renderVignette(mc->thePlayer->getEntityBrightness(partialTick), sw, sh);

	ItemStack *helmet = mc->thePlayer->inventory->armorItemInSlot(3);
	if (!mc->gameSettings->thirdPersonView && helmet != nullptr && helmet->itemID == Block::pumpkin->blockID)
		renderPumpkinBlur(sw, sh);

	float_t portalIntensity = mc->thePlayer->prevTimeInPortal
	    + (mc->thePlayer->timeInPortal - mc->thePlayer->prevTimeInPortal) * partialTick;
	if (!mc->thePlayer->isPotionActive(Potion::confusion) && portalIntensity > 0.0f)
		renderPortalOverlay(portalIntensity, sw, sh);

	// Las entidades pueden dejar GL_BLEND, GL_COLOR, matriz de textura o blend func
	// en un estado no apto para 2D. Reiniciar aca evita hotbar verde/transparente.
	resetOverlayGLState();

	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderBindTexture(mc->renderEngine->getTexture("/gui/gui.png"));
	InventoryPlayer *inv = mc->thePlayer->inventory;
	const int_t hudBottomInset = mc->gameSettings->legacyUI ? legacyHudBottomInset() : 0;
	const int_t hudHeight = sh - hudBottomInset;
#if PLATFORM_PC_LEGACY
	pcLegacyRenderHotbarFrame(sw, hudHeight, inv->currentItem);
#elif defined(PS2_PLATFORM)
	ps2RenderHotbarFrame(sw, hudHeight, inv->currentItem);
#else
	zLevel = -90.0f;
	drawTexturedModalRect(sw / 2 - 91, hudHeight - 22, 0,  0, 182, 22);
	drawTexturedModalRect((sw / 2 - 91 - 1) + inv->currentItem * 20, hudHeight - 22 - 1, 0, 22, 24, 22);
#endif

	renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::OneMinusDstColor, RenderBlendFactor::OneMinusSrcColor);
#if PLATFORM_PC_LEGACY
	pcLegacyRenderCrosshair(sw, sh);
#elif defined(PS2_PLATFORM)
	ps2RenderCrosshair(sw, sh);
#else
	drawTexturedModalRect(sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
#endif
	renderDisable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	renderBossHealth();

	if (mc->playerController->shouldDrawHUD())
	{
#if PLATFORM_PC_LEGACY
		pcLegacyRenderPlayerStatusHud(sw, hudHeight);
#elif defined(PS2_PLATFORM)
		ps2RenderPlayerStatusHud(sw, hudHeight);
#else
		renderPlayerStatusHudUncached(sw, hudHeight);
#endif
	}

	renderDisable(RenderCapability::Blend);
	renderEnable(RenderCapability::RescaleNormal);
#if PLATFORM_PROFILE_RENDER_PHASES
	const std::uint32_t cycHudItems = platformProfileRenderPhaseBegin();
#endif
	RenderHelper::enableGUIStandardItemLighting();
	for (int_t l1 = 0; l1 < 9; l1++)
	{
		int_t ix = (sw / 2 - 90) + l1 * 20 + 2;
		int_t iy = hudHeight - 16 - 3;
		renderInventorySlot(l1, ix, iy, partialTick);
	}
	RenderHelper::disableStandardItemLighting();
#if PLATFORM_PROFILE_RENDER_PHASES
	platformProfileRenderPhaseEnd(cycHudItems, PlatformRenderPhase::HudItems);
#endif
	renderDisable(RenderCapability::RescaleNormal);

	if (mc->thePlayer->getSleepTimer() > 0)
	{
		renderDisable(RenderCapability::DepthTest);
		renderDisable(RenderCapability::AlphaTest);
		int_t sleepT = mc->thePlayer->getSleepTimer();
		float_t f3 = (float_t)sleepT / 100.0f;
		if (f3 > 1.0f) f3 = 1.0f - (float_t)(sleepT - 100) / 10.0f;
		int_t sleepColor = JavaArithmetic::intShl((int_t)(220.0f * f3), 24) | 0x101020;
		drawRect(0, 0, sw, sh, sleepColor);
		renderEnable(RenderCapability::AlphaTest);
		renderEnable(RenderCapability::DepthTest);
	}

	if (mc->playerController->func_35642_f() && mc->thePlayer->experienceLevel > 0)
	{
		const std::string level = std::to_string(mc->thePlayer->experienceLevel);
		const int_t color = 0x80ff20;
		const int_t x = (sw - fr->getStringWidth(level)) / 2;
		const int_t y = hudHeight - 35;
		fr->drawString(level, x + 1, y, 0);
		fr->drawString(level, x - 1, y, 0);
		fr->drawString(level, x, y + 1, 0);
		fr->drawString(level, x, y - 1, 0);
		fr->drawString(level, x, y, color);
	}

	if (mc->gameSettings->showFps && !mc->gameSettings->showDebugInfo)
		renderFpsOverlay(fr);
	if (mc->gameSettings->showDebugInfo)
		renderDebugOverlay(fr, sw);

	if (recordPlayingUpFor > 0)
	{
		float_t f2 = (float_t)recordPlayingUpFor - partialTick;
		int_t alpha = (int_t)((f2 * 256.0f) / 20.0f);
		if (alpha > 255) alpha = 255;
		if (alpha > 0)
		{
			renderPushMatrix();
			renderTranslate((float_t)(sw / 2), (float_t)(sh - 48), 0.0f);
			renderEnable(RenderCapability::Blend);
			renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
			int_t color = 0xffffff;
			if (field_22065_l)
				color = hsbToRgb(f2 / 50.0f, 0.7f, 0.6f) & 0xffffff;
			fr->drawString(recordPlaying, -fr->getStringWidth(recordPlaying) / 2, -4, JavaArithmetic::intAdd(color, JavaArithmetic::intShl(alpha, 24)));
			renderDisable(RenderCapability::Blend);
			renderPopMatrix();
		}
	}

	int_t chatLines = 10;
	bool chatOpen = false;
	if (dynamic_cast<GuiChat *>(mc->currentScreen) != nullptr)
	{
		chatLines = 20;
		chatOpen = true;
	}

#if PLATFORM_PROFILE_RENDER_PHASES
	const std::uint32_t cycHudText = platformProfileRenderPhaseBegin();
#endif
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderDisable(RenderCapability::AlphaTest);
	renderPushMatrix();
	// Anchored to hudHeight, not sh: the Legacy HUD lifts the hotbar and the
	// status bars by legacyHudBottomInset(), and the chat has to keep sitting
	// above them rather than on top of the hearts.
	renderTranslate(0.0f, (float_t)(hudHeight - 48), 0.0f);
	for (int_t i5 = 0; i5 + chatScroll < (int_t)chatMessageList.size() && i5 < chatLines; i5++)
	{
		ChatLine *line = chatMessageList[i5 + chatScroll];
		if (line->updateCounter >= 200 && !chatOpen) continue;
		float d = static_cast<float>(line->updateCounter) / 200.0f;
		d = 1.0f - d;
		d *= 10.0f;
		if (d < 0.0f) d = 0.0f;
		if (d > 1.0f) d = 1.0f;
		d *= d;
		int_t msgAlpha = static_cast<int_t>(255.0f * d);
		if (chatOpen) msgAlpha = 255;
		if (msgAlpha > 0)
		{
			int_t cx = 2;
			int_t cy = -i5 * 9;
			drawRect(cx, cy - 1, cx + 320, cy + 8, JavaArithmetic::intShl(msgAlpha / 2, 24));
			renderEnable(RenderCapability::Blend);
			fr->drawStringWithShadow(line->message, cx, cy, JavaArithmetic::intAdd(0xffffff, JavaArithmetic::intShl(msgAlpha, 24)));
		}
	}
	renderPopMatrix();

	EntityClientPlayerMP *clientPlayer = dynamic_cast<EntityClientPlayerMP *>(mc->thePlayer);
	if (clientPlayer != nullptr && mc->gameSettings->keyBindPlayerList->pressed && clientPlayer->sendQueue != nullptr)
	{
		NetClientHandler *handler = clientPlayer->sendQueue;
		const std::vector<GuiPlayerInfo *> &players = handler->getPlayerNames();
		const int_t maxPlayers = std::max(1, handler->currentServerMaxPlayers);
		int_t columns = 1;
		int_t rows = maxPlayers;
		while (rows > 20)
		{
			++columns;
			rows = (maxPlayers + columns - 1) / columns;
		}

		int_t columnWidth = 300 / columns;
		if (columnWidth > 150)
			columnWidth = 150;
		const int_t left = (sw - columns * columnWidth) / 2;
		const int_t top = 10;
		drawRect(left - 1, top - 1, left + columnWidth * columns, top + 9 * rows, 0x80000000);

		for (int_t index = 0; index < maxPlayers; ++index)
		{
			const int_t x = left + (index % columns) * columnWidth;
			const int_t y = top + (index / columns) * 9;
			drawRect(x, y, x + columnWidth - 1, y + 8, 0x20ffffff);
			renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			renderEnable(RenderCapability::AlphaTest);

			if (index < (int_t)players.size() && players[index] != nullptr)
			{
				GuiPlayerInfo *info = players[index];
				fr->drawStringWithShadow(info->name, x, y, 0xffffff);
				renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
				int_t pingIcon = 0;
				if (info->responseTime < 0) pingIcon = 5;
				else if (info->responseTime < 150) pingIcon = 0;
				else if (info->responseTime < 300) pingIcon = 1;
				else if (info->responseTime < 600) pingIcon = 2;
				else if (info->responseTime < 1000) pingIcon = 3;
				else pingIcon = 4;

				zLevel += 100.0f;
				drawTexturedModalRect(x + columnWidth - 12, y, 0, 176 + pingIcon * 8, 10, 8);
				zLevel -= 100.0f;
			}
		}
	}

#if PLATFORM_PROFILE_RENDER_PHASES
	platformProfileRenderPhaseEnd(cycHudText, PlatformRenderPhase::HudText);
	const std::uint32_t cycHudHints = platformProfileRenderPhaseBegin();
#endif
	LegacyControlTooltipHud::render(mc, sw, sh);
	LegacyTipHud::render(mc, sw, sh);
#if PLATFORM_PROFILE_RENDER_PHASES
	platformProfileRenderPhaseEnd(cycHudHints, PlatformRenderPhase::HudHints);
#endif
	finishOverlayGLState();
}

void GuiIngame::renderPumpkinBlur(int_t w, int_t h)
{
	renderDisable(RenderCapability::DepthTest);
	renderDepthMask(false);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderDisable(RenderCapability::AlphaTest);
	renderBindTexture(mc->renderEngine->getTexture("%blur%/misc/pumpkinblur.png"));
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->addVertexWithUV(0, h, -90.0f, 0.0f, 1.0f);
	tess->addVertexWithUV(w, h, -90.0f, 1.0f, 1.0f);
	tess->addVertexWithUV(w, 0, -90.0f, 1.0f, 0.0f);
	tess->addVertexWithUV(0, 0, -90.0f, 0.0f, 0.0f);
	tess->draw();
	renderDepthMask(true);
	renderEnable(RenderCapability::DepthTest);
	renderEnable(RenderCapability::AlphaTest);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiIngame::renderVignette(float_t brightness, int_t w, int_t h)
{
	float_t f = 1.0f - brightness;
	if (f < 0.0f) f = 0.0f;
	if (f > 1.0f) f = 1.0f;
	prevVignetteBrightness += (f - prevVignetteBrightness) * 0.01f;
	renderDisable(RenderCapability::DepthTest);
	renderDepthMask(false);
	renderBlendFunc(RenderBlendFactor::Zero, RenderBlendFactor::OneMinusSrcColor);
	renderColor4f(prevVignetteBrightness, prevVignetteBrightness, prevVignetteBrightness, 1.0f);
	renderBindTexture(mc->renderEngine->getTexture("%blur%/misc/vignette.png"));
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->addVertexWithUV(0, h, -90.0f, 0.0f, 1.0f);
	tess->addVertexWithUV(w, h, -90.0f, 1.0f, 1.0f);
	tess->addVertexWithUV(w, 0, -90.0f, 1.0f, 0.0f);
	tess->addVertexWithUV(0, 0, -90.0f, 0.0f, 0.0f);
	tess->draw();
	renderDepthMask(true);
	renderEnable(RenderCapability::DepthTest);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
}

void GuiIngame::renderPortalOverlay(float_t intensity, int_t w, int_t h)
{
	if (intensity < 1.0f)
	{
		intensity *= intensity;
		intensity *= intensity;
		intensity = intensity * 0.8f + 0.2f;
	}
	renderDisable(RenderCapability::AlphaTest);
	renderDisable(RenderCapability::DepthTest);
	renderDepthMask(false);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderColor4f(1.0f, 1.0f, 1.0f, intensity);
	renderBindTexture(mc->renderEngine->getTexture("/terrain.png"));
	float_t u1 = (float_t)(Block::portal->blockIndexInTexture % 16)       / 16.0f;
	float_t v1 = (float_t)(Block::portal->blockIndexInTexture / 16)       / 16.0f;
	float_t u2 = (float_t)(Block::portal->blockIndexInTexture % 16 + 1)   / 16.0f;
	float_t v2 = (float_t)(Block::portal->blockIndexInTexture / 16 + 1)   / 16.0f;
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->addVertexWithUV(0, h, -90.0f, u1, v2);
	tess->addVertexWithUV(w, h, -90.0f, u2, v2);
	tess->addVertexWithUV(w, 0, -90.0f, u2, v1);
	tess->addVertexWithUV(0, 0, -90.0f, u1, v1);
	tess->draw();
	renderDepthMask(true);
	renderEnable(RenderCapability::DepthTest);
	renderEnable(RenderCapability::AlphaTest);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiIngame::renderInventorySlot(int_t slot, int_t x, int_t y, float_t partialTick)
{
	ItemStack *stack = mc->thePlayer->inventory->mainInventory[slot];
	if (stack == nullptr) return;

	float_t animF = (float_t)stack->animationsToGo - partialTick;
	if (animF > 0.0f)
	{
		renderPushMatrix();
		float_t scale = 1.0f + animF / 5.0f;
		renderTranslate((float_t)(x + 8), (float_t)(y + 12), 0.0f);
		renderScale(1.0f / scale, (scale + 1.0f) / 2.0f, 1.0f);
		renderTranslate(-(float_t)(x + 8), -(float_t)(y + 12), 0.0f);
	}
	itemRenderer->renderItemIntoGUI(mc->fontRenderer, mc->renderEngine, stack, x, y);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	if (animF > 0.0f) renderPopMatrix();
	itemRenderer->renderItemOverlayIntoGUI(mc->fontRenderer, mc->renderEngine, stack, x, y);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
}

void GuiIngame::updateTick()
{
	// Tips only age while they can be seen, so one queued behind a loading or
	// pause screen is not spent before the player is back in the world.
	if (mc->currentScreen == nullptr)
		LegacyTipHud::tick();
	if (recordPlayingUpFor > 0) recordPlayingUpFor--;
	updateCounter++;
	for (int_t i = 0; i < (int_t)chatMessageList.size(); i++)
		chatMessageList[i]->updateCounter++;
}

void GuiIngame::clearChatMessages()
{
	for (ChatLine *line : chatMessageList)
		delete line;
	chatMessageList.clear();
	sentMessages.clear();
	chatScroll = 0;
	isScrolled = false;
}

void GuiIngame::addChatMessage(const std::string &msg)
{
	const bool chatOpen = isChatOpen();
	jstring remaining(msg);
	bool firstLine = true;
	while (mc->fontRenderer->getStringWidth(remaining) > 320)
	{
		int_t split = 1;
		const int_t length = String::utf16Length(remaining);
		while (split < length &&
		       mc->fontRenderer->getStringWidth(String::substringUtf16(remaining, 0, split + 1)) <= 320)
			++split;

		jstring line = String::substringUtf16(remaining, 0, split);
		if (!firstLine)
			line = " " + line;
		if (chatOpen && chatScroll > 0)
		{
			isScrolled = true;
			scrollChat(1);
		}
		chatMessageList.insert(chatMessageList.begin(), new ChatLine(line));
		remaining = String::substringUtf16(remaining, split, length);
		firstLine = false;
	}

	if (!firstLine)
		remaining = " " + remaining;
	if (chatOpen && chatScroll > 0)
	{
		isScrolled = true;
		scrollChat(1);
	}
	chatMessageList.insert(chatMessageList.begin(), new ChatLine(remaining));
	while ((int_t)chatMessageList.size() > 100)
	{
		delete chatMessageList.back();
		chatMessageList.pop_back();
	}
}

void GuiIngame::resetChatScroll()
{
	chatScroll = 0;
	isScrolled = false;
}

void GuiIngame::scrollChat(int_t amount)
{
	chatScroll += amount;
	int_t maxScroll = std::max(0, (int_t)chatMessageList.size() - 20);
	if (chatScroll > maxScroll)
		chatScroll = maxScroll;
	if (chatScroll <= 0)
	{
		chatScroll = 0;
		isScrolled = false;
	}
}

bool GuiIngame::isChatOpen() const
{
	return dynamic_cast<GuiChat *>(mc->currentScreen) != nullptr;
}

ChatClickData *GuiIngame::getChatClickData(int_t rawMouseX, int_t rawMouseY)
{
	if (!isChatOpen())
		return nullptr;
	ScaledResolution scaled(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	const double guiScale = scaled.getScaleFactorExact();
	int_t mouseY = static_cast<int_t>(rawMouseY / guiScale) - 40;
	int_t mouseX = static_cast<int_t>(rawMouseX / guiScale) - 3;
	if (mouseX < 0 || mouseY < 0)
		return nullptr;
	int_t visible = std::min(20, (int_t)chatMessageList.size());
	if (mouseX > 320 || mouseY >= 9 * visible)
		return nullptr;
	int_t lineIndex = mouseY / 9 + chatScroll;
	if (lineIndex < 0 || lineIndex >= (int_t)chatMessageList.size())
		return nullptr;
	return new ChatClickData(mc->fontRenderer, chatMessageList[lineIndex], mouseX, mouseY - (lineIndex - chatScroll) * 8 + lineIndex);
}

void GuiIngame::setRecordPlayingMessage(const std::string &record)
{
	recordPlaying = "Now playing: " + record;
	recordPlayingUpFor = 60;
	field_22065_l = true;
}

void GuiIngame::addChatMessageTranslate(const std::string &key)
{
	StringTranslate *tr = StringTranslate::getInstance();
	addChatMessage(tr->translateKey(key));
}
