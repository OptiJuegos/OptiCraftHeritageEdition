#include "net/minecraft/src/UiStrings.h"
#include "GuiIngame.h"
#include <cstring>
#include "mods/ModManager.h"
#include "platform/PlatformTuning.h"
#include "platform/Profiler.h"
#if defined(CTR_PLATFORM)
#include "platform/TouchHudLayout.h"
#include "mods/reiminimap/ReiMinimap.h"
#include "MovingObjectPosition.h"
#include "pc/lwjgl/Mouse.h"
#include "legacy/LegacyCraftingScreen.h"
#endif
#include "java/String.h"
#include "java/Arithmetic.h"
#include "ScaledResolution.h"
#include "EntityRenderer.h"
#include "EntityPlayerSP.h"
#include "GuiPlayerInfo.h"
#include "NetClientHandler.h"
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
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
#if PLATFORM_PC_LEGACY || defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
#include "pc/render/PcLegacyHudCachePolicy.h"
#endif
#if defined(PS2_PLATFORM)
#include "ps2/render/Ps2Draw2D.h"
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

#if PLATFORM_PC_LEGACY || defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
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
#if defined(PS2_PLATFORM)
		ps2_draw_2d_flush_pending();
#endif
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

#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
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
#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
	, ps2HudCache(new Ps2HudCache())
#endif
	, damageGuiPartialTime(0.0f)
	, prevVignetteBrightness(1.0f)
{
#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
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
#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
	if (ps2HudCache != nullptr)
	{
		renderStaticMeshDestroy(ps2HudCache->hotbar);
		renderStaticMeshDestroy(ps2HudCache->crosshair);
		renderStaticMeshDestroy(ps2HudCache->status);
		delete ps2HudCache;
		ps2HudCache = nullptr;
	}
#endif
#if defined(CTR_PLATFORM)
	if (ctrHotbarListBase != 0)
	{
		renderDeleteDisplayLists(ctrHotbarListBase, 9);
		ctrHotbarListBase = 0;
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

	// [FIX WII / ISSUE #9] Margen de seguridad para televisores (Title Safe Area).
	// En televisores analógicos, CRT o convertidores HDMI que aplican overscan, las coordenadas (2, 2)
	// quedan tapadas por el borde físico de la pantalla. Añadimos un margen seguro de 12px en consolas.
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM)
	constexpr int_t safeX = 12;
	constexpr int_t safeY = 12;
#else
	constexpr int_t safeX = 2;
	constexpr int_t safeY = 2;
#endif

#ifdef PS2_PLATFORM
	fontRenderer->drawString(fpsLine, safeX, safeY, 0xe0e0e0);
#else
	// One batch for shadow+glyphs (a no-op on display-list backends):
	// drawStringWithShadow submits two draws per line otherwise.
	fontRenderer->beginTextBatch();
	fontRenderer->drawStringWithShadow(fpsLine, safeX, safeY, 0xffffff);
	fontRenderer->endTextBatch();
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
	// [FIX WII / ISSUE #9] Margen de seguridad contra Overscan en el menú F3 para Nintendo Wii.
	// Se desplaza la columna izquierda a 12px y el margen derecho a 12px para evitar recortes en la TV.
#if defined(WII_PLATFORM)
	constexpr int_t safeLeft = 12;
	constexpr int_t safeTop = 10;
	constexpr int_t safeRightMargin = 12;
#else
	constexpr int_t safeLeft = 2;
	constexpr int_t safeTop = 2;
	constexpr int_t safeRightMargin = 2;
#endif

	// The whole overlay is text-only, so one shared batch covers every line
	// (no-op on display-list backends; on FONT_IMMEDIATE targets it turns
	// two submits per line -- shadow + glyphs -- into one for the block).
	fontRenderer->beginTextBatch();
	fontRenderer->drawStringWithShadow("OptiCraft (" + mc->debug + ")", safeLeft, safeTop, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine1(), safeLeft, safeTop + 10, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine2(), safeLeft, safeTop + 20, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine3(), safeLeft, safeTop + 30, 0xffffff);
	fontRenderer->drawStringWithShadow(mc->getDebugLine4(), safeLeft, safeTop + 40, 0xffffff);
	std::string cpuGpuLine = uiText("CPU: ") + std::to_string((int_t)(mc->cpuUsagePercent + 0.5f)) + "% GPU: "
	    + std::to_string((int_t)(mc->gpuUsagePercent + 0.5f)) + "%";
	fontRenderer->drawStringWithShadow(cpuGpuLine, safeLeft, safeTop + 50, 0xffffff);
	Runtime &runtime = Runtime::getRuntime();
	long_t maxMemory = runtime.maxMemory();
	long_t totalMemory = runtime.totalMemory();
	long_t freeMemory = runtime.freeMemory();
	long_t usedMemory = totalMemory - freeMemory;
	std::string memoryUsed = uiText("Used memory: ") + std::to_string((usedMemory * 100LL) / maxMemory) + "% ("
	    + std::to_string(usedMemory / 1024LL / 1024LL) + uiText("MB) of ")
	    + std::to_string(maxMemory / 1024LL / 1024LL) + "MB";
	drawString(fontRenderer, memoryUsed, screenWidth - fontRenderer->getStringWidth(memoryUsed) - safeRightMargin, safeTop, 0xe0e0e0);
	std::string memoryAllocated = uiText("Allocated memory: ") + std::to_string((totalMemory * 100LL) / maxMemory) + "% ("
	    + std::to_string(totalMemory / 1024LL / 1024LL) + "MB)";
	drawString(fontRenderer, memoryAllocated, screenWidth - fontRenderer->getStringWidth(memoryAllocated) - safeRightMargin, safeTop + 10, 0xe0e0e0);
	drawString(fontRenderer, "x: " + std::to_string(mc->thePlayer->posX), safeLeft, safeTop + 62, 0xe0e0e0);
	drawString(fontRenderer, "y: " + std::to_string(mc->thePlayer->posY), safeLeft, safeTop + 70, 0xe0e0e0);
	drawString(fontRenderer, "z: " + std::to_string(mc->thePlayer->posZ), safeLeft, safeTop + 78, 0xe0e0e0);
	drawString(fontRenderer, "f: " + std::to_string(MathHelper::floor_float((mc->thePlayer->rotationYaw * 4.0f) / 360.0f + 0.5f) & 3), safeLeft, safeTop + 86, 0xe0e0e0);
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
	drawString(fontRenderer, platformInputDebugLine(), safeLeft, safeTop + 94, 0xe0e0e0);
	// The NET/TCP readout is the 3DS multiplayer's first diagnostic: rd/wr
	// must both stay 1 (a 0 is a dead network thread -- see NetworkManager's
	// entry points), and q growing while tx stalls means the writer is being
	// starved. The 3DS was left out of this block while its net stack was
	// bring-up; the 2026-09-28 starvation bug is why it must not be.
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
		drawString(fontRenderer, multiplayerLine, safeLeft, safeTop + 104, 0xe0e0e0);

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
			drawString(fontRenderer, packetLine, safeLeft, safeTop + 114, 0xe0e0e0);

			char socketLine[112];
			std::snprintf(socketLine, sizeof(socketLine),
				"TCP rx:%zuKB tx:%zuKB rd:%d wr:%d",
				networkManager->getSocketReceivedByteCount() / 1024u,
				networkManager->getSocketSentByteCount() / 1024u,
				networkManager->isReadThreadActive() ? 1 : 0,
				networkManager->isWriteThreadActive() ? 1 : 0);
			drawString(fontRenderer, socketLine, safeLeft, safeTop + 124, 0xe0e0e0);
		}
	}
#endif
	fontRenderer->endTextBatch();
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
	// [FIX WII] Se eliminó la segunda llamada duplicada idéntica 'drawTexturedModalRect(x, y, 0, 74, barWidth, 5);' para evitar sobrecarga y emisión de vértices redundantes en el Tessellator.
	if (filled > 0)
		drawTexturedModalRect(x, y, 0, 79, filled, 5);

	const std::string name = uiText("Boss health");
	fontRenderer->drawStringWithShadow(name, screenWidth / 2 - fontRenderer->getStringWidth(name) / 2, y - 10, 0xff00ff);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
}

void GuiIngame::renderPlayerStatusHudGeometry(int_t sw, int_t sh, Tessellator *captureTessellator)
{
#if defined(CTR_PLATFORM)
	// The whole status cluster emits into ONE batch on this platform: the
	// old code did startDrawingQuads()/draw() around every single icon,
	// which is ~60 individual backend submits per frame on the uncached
	// path -- and in capture mode each start reset the record, so a cached
	// static mesh kept only the last icon. Emission now appends; the
	// direct caller opens/closes the batch here, the static-mesh callers
	// own theirs.
	Tessellator *const ctrStatusBatch = captureTessellator != nullptr ? captureTessellator : &Tessellator::instance;
	const bool ctrOwnsBatch = captureTessellator == nullptr;
	if (ctrOwnsBatch)
		ctrStatusBatch->startDrawingQuads();
#endif
	auto emitRect = [&](int_t x, int_t y, int_t texX, int_t texY, int_t w, int_t h)
	{
		if (captureTessellator != nullptr)
		{
			appendTexturedModalRect(*captureTessellator, zLevel, x, y, texX, texY, w, h);
			return;
		}
		drawTexturedModalRect(x, y, texX, texY, w, h);
	};

#if defined(CTR_PLATFORM)
	// The status glyphs draw larger on this platform: the 9 px icons scaled
	// to 14 px with the vanilla UVs, appended to the batch above (the
	// uncached path has icons.png bound by the caller).
	auto emitIcon = [&](int_t x, int_t y, int_t texX, int_t texY)
	{
		constexpr float INV = 1.0f / 256.0f;
		const float u0 = static_cast<float_t>(texX) * INV;
		const float v0 = static_cast<float_t>(texY) * INV;
		const float u1 = u0 + 9.0f * INV;
		const float v1 = v0 + 9.0f * INV;
		ctrStatusBatch->setColorOpaque_I(0xffffff);
		ctrStatusBatch->addVertexWithUV(x, y + 14.0f, zLevel, u0, v1);
		ctrStatusBatch->addVertexWithUV(x + 14.0f, y + 14.0f, zLevel, u1, v1);
		ctrStatusBatch->addVertexWithUV(x + 14.0f, y, zLevel, u1, v0);
		ctrStatusBatch->addVertexWithUV(x, y, zLevel, u0, v0);
	};
	constexpr int_t ICON_PITCH = 14;
#else
	auto emitIcon = [&](int_t x, int_t y, int_t texX, int_t texY)
	{
		emitRect(x, y, texX, texY, 9, 9);
	};
	constexpr int_t ICON_PITCH = 8;
#endif

	bool flashHearts = (mc->thePlayer->heartsLife / 3) % 2 == 1;
	if (mc->thePlayer->heartsLife < 10)
		flashHearts = false;
	const int_t health = mc->thePlayer->health;
	const int_t prevHealth = mc->thePlayer->prevHealth;
	const int_t seed = JavaArithmetic::intFromBits(static_cast<uint_t>(updateCounter) * 0x4c627u);
	rand->setSeed(static_cast<long_t>(seed));
	const int_t left = sw / 2 - 91;
	const int_t right = sw / 2 + 91;
#if defined(CTR_PLATFORM)
	// Dual-screen gameplay, take three (owner calls): the XP strip runs
	// across the very top of the panel at full width; the hearts sit just
	// under it on the LEFT and the food bar just under it on the RIGHT,
	// both drawn ~30% larger (12 px glyphs from the 9 px source); armor
	// rides under the hearts, air bubbles under the food. The XP number
	// keeps the centre of that icon row. The hotbar lives on the touch
	// panel.
	const int_t xpY = 0;
	const int_t heartsLeft = 4;
	const int_t foodRight = sw - 4;
	const int_t healthY = 12;
	const int_t foodY = 12;
	const int_t armorLeft = 4;
	const int_t armorY = healthY + 16;
	const int_t airLeft = sw - 4;
	const int_t airY = foodY + 16;
	(void)left;
	(void)right;
#else
	const int_t heartsLeft = left;
	const int_t foodRight = right;
	const int_t armorLeft = left;
	const int_t airLeft = right;
	const int_t xpY = sh - 32 + 3;
	const int_t healthY = sh - 39;
	const int_t foodY = healthY;
	const int_t armorY = healthY - 10;
	const int_t airY = armorY;
#endif
	const int_t xpCap = mc->thePlayer->xpBarCap();
	if (xpCap > 0)
	{
#if defined(CTR_PLATFORM)
		// Full-width strip: the 182 px bar texture stretched across the
		// whole top edge and thickened to 8 px; the fill keeps its
		// proportional UVs. Appended to the shared status batch.
		constexpr float INV = 1.0f / 256.0f;
		const int_t fillW = static_cast<int_t>(mc->thePlayer->experience * static_cast<float_t>(sw));
		const float fillU = mc->thePlayer->experience * 182.0f * INV;
		ctrStatusBatch->setColorOpaque_I(0xffffff);
		ctrStatusBatch->addVertexWithUV(0, xpY + 8, zLevel, 0.0f, 69.0f * INV);
		ctrStatusBatch->addVertexWithUV(sw, xpY + 8, zLevel, 182.0f * INV, 69.0f * INV);
		ctrStatusBatch->addVertexWithUV(sw, xpY, zLevel, 182.0f * INV, 64.0f * INV);
		ctrStatusBatch->addVertexWithUV(0, xpY, zLevel, 0.0f, 64.0f * INV);
		if (fillW > 0)
		{
			ctrStatusBatch->addVertexWithUV(0, xpY + 8, zLevel, 0.0f, 74.0f * INV);
			ctrStatusBatch->addVertexWithUV(fillW, xpY + 8, zLevel, fillU, 74.0f * INV);
			ctrStatusBatch->addVertexWithUV(fillW, xpY, zLevel, fillU, 69.0f * INV);
			ctrStatusBatch->addVertexWithUV(0, xpY, zLevel, 0.0f, 69.0f * INV);
		}
#else
		constexpr int_t XP_BAR_WIDTH = 182;
		const int_t filled = static_cast<int_t>(mc->thePlayer->experience * static_cast<float_t>(XP_BAR_WIDTH + 1));
		emitRect(left, xpY, 0, 64, XP_BAR_WIDTH, 5);
		if (filled > 0)
			emitRect(left, xpY, 0, 69, filled, 5);
#endif
	}
	const int_t armor = mc->thePlayer->getPlayerArmorValue();
	const int_t regenerationHeart = mc->thePlayer->isPotionActive(Potion::regeneration) ? updateCounter % 25 : -1;
	const bool hardcore = mc->theWorld != nullptr && mc->theWorld->getWorldInfo() != nullptr
		&& mc->theWorld->getWorldInfo()->isHardcoreModeEnabled();

	for (int_t index = 0; index < 10; ++index)
	{
		if (armor > 0)
		{
			const int_t armorX = armorLeft + index * ICON_PITCH;
			if (index * 2 + 1 < armor)  emitIcon(armorX, armorY, 34, 9);
			if (index * 2 + 1 == armor) emitIcon(armorX, armorY, 25, 9);
			if (index * 2 + 1 > armor)  emitIcon(armorX, armorY, 16, 9);
		}

		int_t heartTextureX = 16;
		if (mc->thePlayer->isPotionActive(Potion::poison))
			heartTextureX += 36;
		const int_t flash = flashHearts ? 1 : 0;
		const int_t hx = heartsLeft + index * ICON_PITCH;
		int_t hy = healthY;
		if (health <= 4)
			hy += rand->nextInt(2);
		if (index == regenerationHeart)
			hy -= 2;
		const int_t hardcoreRow = hardcore ? 5 : 0;

		emitIcon(hx, hy, 16 + flash * 9, 9 * hardcoreRow);
		if (flashHearts)
		{
			if (index * 2 + 1 < prevHealth)  emitIcon(hx, hy, heartTextureX + 54, 9 * hardcoreRow);
			if (index * 2 + 1 == prevHealth) emitIcon(hx, hy, heartTextureX + 63, 9 * hardcoreRow);
		}
		if (index * 2 + 1 < health)  emitIcon(hx, hy, heartTextureX + 36, 9 * hardcoreRow);
		if (index * 2 + 1 == health) emitIcon(hx, hy, heartTextureX + 45, 9 * hardcoreRow);
	}

	FoodStats *foodStats = mc->thePlayer->getFoodStats();
	const int_t foodLevel = foodStats != nullptr ? foodStats->getFoodLevel() : 20;
	const float_t saturation = foodStats != nullptr ? foodStats->getSaturationLevel() : 5.0f;
	for (int_t index = 0; index < 10; ++index)
	{
		int_t fy = foodY;
		int_t foodTextureX = 16;
		int_t backgroundOffset = 0;
		if (mc->thePlayer->isPotionActive(Potion::hunger))
		{
			foodTextureX += 36;
			backgroundOffset = 13;
		}
		if (saturation <= 0.0f && updateCounter % (foodLevel * 3 + 1) == 0)
			fy = foodY + (rand->nextInt(3) - 1);
		const int_t fx = foodRight - index * ICON_PITCH - ICON_PITCH;
		emitIcon(fx, fy, 16 + backgroundOffset * 9, 27);
		if (index * 2 + 1 < foodLevel)  emitIcon(fx, fy, foodTextureX + 36, 27);
		if (index * 2 + 1 == foodLevel) emitIcon(fx, fy, foodTextureX + 45, 27);
	}

	if (mc->thePlayer->isInsideOfMaterial(Material::water))
	{
		const int_t air = mc->thePlayer->getAir();
		const int_t full = JavaArithmetic::floatToInt(std::ceil((static_cast<float_t>(air - 2) * 10.0f) / 300.0f));
		const int_t empty = JavaArithmetic::floatToInt(std::ceil((static_cast<float_t>(air) * 10.0f) / 300.0f)) - full;
		for (int_t index = 0; index < full + empty; ++index)
		{
			const int_t ax = airLeft - index * ICON_PITCH - ICON_PITCH;
			if (index < full)
				emitIcon(ax, airY, 16, 18);
			else
				emitIcon(ax, airY, 25, 18);
		}
	}
#if defined(CTR_PLATFORM)
	if (ctrOwnsBatch)
		ctrStatusBatch->draw();
#endif
}

void GuiIngame::renderPlayerStatusHudUncached(int_t sw, int_t sh)
{
	// The status bars sample /gui/icons.png, but nothing else in the frame
	// guarantees that bind: the crosshair block that owns it sits inside
	// "if (!showDebug)" and is skipped while any GuiScreen is open, and the
	// boss bar only re-binds when a boss is actually on screen. The
	// hotbar's gui.png used to leak through instead, so opening the pause
	// menu or the inventory drew hearts and food from the wrong atlas --
	// garbled bars, hidden behind the menu on a single screen but in plain
	// sight once the menus moved to the 3DS bottom panel. Bind here so
	// every caller (direct, display list, static-mesh fallback) is covered.
	renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
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

#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
void GuiIngame::ps2RenderHotbarFrame(int_t sw, int_t sh, int_t currentItem)
{
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		zLevel = -90.0f;
		drawTexturedModalRect(sw / 2 - 91, sh - 22, 0, 0, 182, 22);
		drawTexturedModalRect((sw / 2 - 91 - 1) + currentItem * 20, sh - 23, 0, 22, 24, 22);
		return;
	}

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
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		zLevel = -90.0f;
		drawTexturedModalRect(sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
		return;
	}

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
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		renderPlayerStatusHudUncached(sw, sh);
		return;
	}

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

	// The cached mesh replays geometry only, and the icons.png bind belongs
	// to the crosshair block that an open screen skips -- see
	// renderPlayerStatusHudUncached. Bind it here too.
	renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
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

#if defined(CTR_PLATFORM)
	// Dual-screen gameplay: the bottom panel carries the touch layer --
	// hotbar, coordinates and the inventory/crafting/pause buttons. Only
	// while no GuiScreen is open: any screen (pause, inventory, chat) owns
	// the panel through EntityRenderer's wrapper, and a second pass here
	// would clear it away.
	if (mc->currentScreen == nullptr && renderBottomPanelBegin())
	{
		renderGameplayBottomPanel(partialTick);
		renderBottomPanelEnd();
	}
#endif

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
	const bool isSplit = mc->isSplitScreenActive();
	const int_t hudBottomInset = mc->gameSettings->legacyUI ? legacyHudBottomInset(isSplit) : 0;
	const int_t hudHeight = sh - hudBottomInset;
#if PLATFORM_PC_LEGACY
	pcLegacyRenderHotbarFrame(sw, hudHeight, inv->currentItem);
#elif defined(PS2_PLATFORM)
	ps2RenderHotbarFrame(sw, hudHeight, inv->currentItem);
#else
#if !defined(CTR_PLATFORM)
	// Dual-screen: the hotbar lives on the touch panel (see
	// renderGameplayBottomPanel); the top screen keeps only the crosshair
	// and the status cluster.
	zLevel = -90.0f;
	drawTexturedModalRect(sw / 2 - 91, hudHeight - 22, 0,  0, 182, 22);
	drawTexturedModalRect((sw / 2 - 91 - 1) + inv->currentItem * 20, hudHeight - 22 - 1, 0, 22, 24, 22);
#endif
#endif

	if (!showDebug)
	{
		renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
		renderEnable(RenderCapability::Blend);
		renderBlendFunc(RenderBlendFactor::OneMinusDstColor, RenderBlendFactor::OneMinusSrcColor);
#if PLATFORM_PC_LEGACY
		pcLegacyRenderCrosshair(sw, sh);
#elif defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
		ps2RenderCrosshair(sw, sh);
#else
		drawTexturedModalRect(sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
#endif
		renderDisable(RenderCapability::Blend);
		renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
		renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	renderBossHealth();

	// [FIX DEFENSIVO] Verificación contra nullptr en mc->playerController antes de consultar shouldDrawHUD()
	if (mc->playerController != nullptr && mc->playerController->shouldDrawHUD())
	{
#if PLATFORM_PC_LEGACY
		pcLegacyRenderPlayerStatusHud(sw, hudHeight);
#elif defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
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
#if !defined(CTR_PLATFORM)
	for (int_t l1 = 0; l1 < 9; l1++)
	{
		int_t ix = (sw / 2 - 90) + l1 * 20 + 2;
		int_t iy = hudHeight - 16 - 3;
		renderInventorySlot(l1, ix, iy, partialTick);
	}
#endif
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

	// [FIX DEFENSIVO] Verificación contra nullptr en mc->playerController
	if (mc->playerController != nullptr && mc->playerController->func_35642_f() && mc->thePlayer->experienceLevel > 0)
	{
		const std::string level = std::to_string(mc->thePlayer->experienceLevel);
		const int_t color = 0x80ff20;
#if defined(CTR_PLATFORM)
		// The level number rides the icon row at 2x the font -- the same
		// size class as the coordinates strip -- keeping vanilla's outline
		// (the +/-1 local units scale to +/-2 world pixels).
		renderPushMatrix();
		renderTranslate(static_cast<float_t>(sw) * 0.5f, 0.0f, 0.0f);
		renderScale(2.0f, 2.0f, 1.0f);
		const int_t lx = -fr->getStringWidth(level) / 2;
		fr->drawString(level, lx + 1, 6, 0);
		fr->drawString(level, lx - 1, 6, 0);
		fr->drawString(level, lx, 6 + 1, 0);
		fr->drawString(level, lx, 6 - 1, 0);
		fr->drawString(level, lx, 6, color);
		renderPopMatrix();
#else
		const int_t x = (sw - fr->getStringWidth(level)) / 2;
		const int_t y = hudHeight - 35;
		fr->drawString(level, x + 1, y, 0);
		fr->drawString(level, x - 1, y, 0);
		fr->drawString(level, x, y + 1, 0);
		fr->drawString(level, x, y - 1, 0);
		fr->drawString(level, x, y, color);
#endif
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

	// Two passes so every line shares ONE text batch on FONT_IMMEDIATE
	// backends: the per-line interleave (background rect, then string) can
	// never batch. Rows do not overlap, so "all backgrounds then all strings"
	// blends to the same pixels in the same order per pixel.
	int_t lineAlpha[20];
	for (int_t i5 = 0; i5 < chatLines; ++i5)
	{
		lineAlpha[i5] = 0;
		if (i5 + chatScroll >= (int_t)chatMessageList.size())
			continue;
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
		if (msgAlpha <= 0)
			continue;
		lineAlpha[i5] = msgAlpha;
		drawRect(2, -i5 * 9 - 1, 2 + 320, -i5 * 9 + 8, JavaArithmetic::intShl(msgAlpha / 2, 24));
	}
	fr->beginTextBatch();
	for (int_t i5 = 0; i5 < chatLines; ++i5)
	{
		if (lineAlpha[i5] <= 0)
			continue;
		fr->drawStringWithShadow(chatMessageList[i5 + chatScroll]->message, 2, -i5 * 9,
			JavaArithmetic::intAdd(0xffffff, JavaArithmetic::intShl(lineAlpha[i5], 24)));
	}
	fr->endTextBatch();
	renderPopMatrix();

	EntityClientPlayerMP *clientPlayer = dynamic_cast<EntityClientPlayerMP *>(mc->thePlayer);
	if (clientPlayer != nullptr && mc->gameSettings->keyBindPlayerList != nullptr &&
	    mc->gameSettings->keyBindPlayerList->pressed && clientPlayer->sendQueue != nullptr)
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

		// Three passes, same reason as the chat: the interleaved
		// rect/text/icon order per row cannot share one text batch, and the
		// rows never overlap, so the resequencing blends identically.
		for (int_t index = 0; index < maxPlayers; ++index)
		{
			const int_t x = left + (index % columns) * columnWidth;
			const int_t y = top + (index / columns) * 9;
			drawRect(x, y, x + columnWidth - 1, y + 8, 0x20ffffff);
		}
		renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		renderEnable(RenderCapability::AlphaTest);

		fr->beginTextBatch();
		for (int_t index = 0; index < maxPlayers && index < (int_t)players.size(); ++index)
		{
			if (players[index] == nullptr)
				continue;
			fr->drawStringWithShadow(players[index]->name,
				left + (index % columns) * columnWidth,
				top + (index / columns) * 9, 0xffffff);
		}
		fr->endTextBatch();

		renderBindTexture(mc->renderEngine->getTexture("/gui/icons.png"));
		for (int_t index = 0; index < maxPlayers && index < (int_t)players.size(); ++index)
		{
			if (players[index] == nullptr)
				continue;
			GuiPlayerInfo *info = players[index];
			int_t pingIcon = 0;
			if (info->responseTime < 0) pingIcon = 5;
			else if (info->responseTime < 150) pingIcon = 0;
			else if (info->responseTime < 300) pingIcon = 1;
			else if (info->responseTime < 600) pingIcon = 2;
			else if (info->responseTime < 1000) pingIcon = 3;
			else pingIcon = 4;

			zLevel += 100.0f;
			drawTexturedModalRect(left + (index % columns) * columnWidth + columnWidth - 12,
				top + (index / columns) * 9, 0, 176 + pingIcon * 8, 10, 8);
			zLevel -= 100.0f;
		}
	}

#if PLATFORM_PROFILE_RENDER_PHASES
	platformProfileRenderPhaseEnd(cycHudText, PlatformRenderPhase::HudText);
	const std::uint32_t cycHudHints = platformProfileRenderPhaseBegin();
#endif
	LegacyControlTooltipHud::render(mc, sw, sh);
	if (!mc->isSplitScreenActive())
		LegacyTipHud::render(mc, sw, sh);
#if PLATFORM_PROFILE_RENDER_PHASES
	platformProfileRenderPhaseEnd(cycHudHints, PlatformRenderPhase::HudHints);
#endif
	ModManager::getInstance().onRenderGameOverlay(this, sw, sh, partialTick);
	finishOverlayGLState();
}

#if defined(CTR_PLATFORM)
void GuiIngame::renderGameplayBottomPanel(float_t partialTick)
{
	// The crafting button's request lands here (DsInput only flags it --
	// a fixed key code reached OptiFine's zoom on real settings, and the
	// crafting binding itself may be anything): open the Legacy crafting
	// screen directly, in its 2x2 inventory mode, under exactly the
	// conditions Minecraft's own key handler uses.
	if (platformConsumeTouchCraftRequest() && mc->gameSettings != nullptr &&
	    mc->gameSettings->legacyCrafting && mc->gameSettings->legacyUI &&
	    mc->playerController != nullptr && !mc->playerController->isInCreativeMode())
	{
		mc->displayGuiScreen(new LegacyCraftingScreen(mc->thePlayer->inventory,
			mc->theWorld, 0, 0, 0, true, mc->thePlayer));
		return;
	}

	// Called inside renderBottomPanelBegin/End, whose projection is this
	// panel's own 320x240 canvas -- every constant below is a panel pixel
	// from TouchHudLayout.h. The rest of the panel stays the camera pad.
	//
	// Everything draws at zLevel 0: the panel's depth buffer is cleared to
	// its near plane every pass, and every widget that has always rendered
	// here (the menus, the containers) draws at z >= 0. The first cut drew
	// at -90 like the top HUD, and the depth test rejected every textured
	// quad -- only the z=0 rects showed, which is exactly how the bug
	// looked on hardware.
	zLevel = 0.0f;

	// Pocket-Edition-style pad tap (see DsInput): a short touch on the
	// camera pad. On a block it places/interacts (button 1); on air it
	// swings/hits (button 0). The USE path (eat food, draw bow, block)
	// is the HOLD gesture, not the tap — Pocket Edition's split.
	if (platformConsumeTouchPadTap())
	{
		const bool targetsBlock = mc->objectMouseOver != nullptr &&
			mc->objectMouseOver->entityHit == nullptr;
		const int_t tapButton = targetsBlock ? 1 : 0;
		lwjgl::Mouse::detail::pushButton(tapButton, true, 0, 0);
		lwjgl::Mouse::detail::pushButton(tapButton, false, 0, 0);
	}

	// Tell DsInput what the crosshair targets so the pad-hold gesture can
	// route break (button 0, on a block) vs use-item (button 1, on air —
	// eat food, draw bow, block with sword).
	platformSetCrosshairTargetsBlock(mc->objectMouseOver != nullptr);

	// The classic menu backdrop: the dirt texture tiled at 32 px and
	// darkened, the same surface GuiScreen::drawBackground lays under the
	// Java menus ("gray like the original", owner call).
	{
		Tessellator *bgTess = &Tessellator::instance;
		constexpr float BG_TILE = 32.0f;
		renderDisable(RenderCapability::Lighting);
		renderDisable(RenderCapability::Fog);
		renderBindTexture(mc->renderEngine->getTexture("/gui/background.png"));
		renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		bgTess->startDrawingQuads();
		bgTess->setColorOpaque_I(0x404040);
		bgTess->addVertexWithUV(0, static_cast<float_t>(touchHud::PANEL_HEIGHT), 0.0f, 0.0f,
			static_cast<float_t>(touchHud::PANEL_HEIGHT) / BG_TILE);
		bgTess->addVertexWithUV(static_cast<float_t>(touchHud::PANEL_WIDTH),
			static_cast<float_t>(touchHud::PANEL_HEIGHT), 0.0f,
			static_cast<float_t>(touchHud::PANEL_WIDTH) / BG_TILE,
			static_cast<float_t>(touchHud::PANEL_HEIGHT) / BG_TILE);
		bgTess->addVertexWithUV(static_cast<float_t>(touchHud::PANEL_WIDTH), 0.0f, 0.0f,
			static_cast<float_t>(touchHud::PANEL_WIDTH) / BG_TILE, 0.0f);
		bgTess->addVertexWithUV(0, 0.0f, 0.0f, 0.0f, 0.0f);
		bgTess->draw();
	}

	InventoryPlayer *inv = mc->thePlayer->inventory;

	// Touch hotbar: the vanilla 182x22 strip stretched across the panel's
	// full width -- nine equal slots, wide enough for fingers.
	renderBindTexture(mc->renderEngine->getTexture("/gui/gui.png"));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	{
		Tessellator *tess = &Tessellator::instance;
		constexpr float INV = 1.0f / 256.0f;
		const int_t slotW = touchHud::HOTBAR_W / touchHud::HOTBAR_SLOTS;
		const int_t selX = touchHud::HOTBAR_X + inv->currentItem * slotW;

		tess->startDrawingQuads();
		tess->setColorOpaque_I(0xffffff);
		tess->addVertexWithUV(touchHud::HOTBAR_X, touchHud::HOTBAR_Y + touchHud::HOTBAR_H, zLevel, 0.0f, 22.0f * INV);
		tess->addVertexWithUV(touchHud::HOTBAR_X + touchHud::HOTBAR_W, touchHud::HOTBAR_Y + touchHud::HOTBAR_H, zLevel, 182.0f * INV, 22.0f * INV);
		tess->addVertexWithUV(touchHud::HOTBAR_X + touchHud::HOTBAR_W, touchHud::HOTBAR_Y, zLevel, 182.0f * INV, 0.0f);
		tess->addVertexWithUV(touchHud::HOTBAR_X, touchHud::HOTBAR_Y, zLevel, 0.0f, 0.0f);
		// The selection frame, stretched over the selected slot.
		tess->addVertexWithUV(selX - 1, touchHud::HOTBAR_Y - 1 + touchHud::HOTBAR_H, zLevel, 0.0f, 44.0f * INV);
		tess->addVertexWithUV(selX + slotW + 1, touchHud::HOTBAR_Y - 1 + touchHud::HOTBAR_H, zLevel, 24.0f * INV, 44.0f * INV);
		tess->addVertexWithUV(selX + slotW + 1, touchHud::HOTBAR_Y - 1, zLevel, 24.0f * INV, 22.0f * INV);
		tess->addVertexWithUV(selX - 1, touchHud::HOTBAR_Y - 1, zLevel, 0.0f, 22.0f * INV);
		tess->draw();
	}

	// Player coordinates, centred on a translucent black strip a little
	// below the hotbar (the same strip style the title's "A Select" row
	// uses), at 2x the UI font (owner call: +200%). The OptiCraft Options
	// "Touch Coords" toggle hides the strip entirely.
	if (mc->gameSettings == nullptr || mc->gameSettings->touchCoords)
	{
		const std::string coords = "X:"
			+ std::to_string(MathHelper::floor_double(mc->thePlayer->posX)) + " Y:"
			+ std::to_string(MathHelper::floor_double(mc->thePlayer->posY)) + " Z:"
			+ std::to_string(MathHelper::floor_double(mc->thePlayer->posZ));
		drawRect(0, touchHud::COORDS_BAR_TOP, touchHud::PANEL_WIDTH, touchHud::COORDS_BAR_BOTTOM,
			static_cast<int_t>(0x88000000u));
		const int_t textX = (touchHud::PANEL_WIDTH - mc->fontRenderer->getStringWidth(coords) * 2) / 2;
		renderPushMatrix();
		renderTranslate(static_cast<float_t>(textX), static_cast<float_t>(touchHud::COORDS_TEXT_Y), 0.0f);
		renderScale(2.0f, 2.0f, 1.0f);
		mc->fontRenderer->drawStringWithShadow(coords, 0, 0, 0xffffff);
		renderPopMatrix();
	}

	// Map slot: ReiMinimap's renderer, driven natively (no mod registration,
	// no mod menu — the class is compiled into the binary and called
	// directly). The OptiCraft Options "Touch Map" toggle hides the slot.
	if (mc->gameSettings == nullptr || mc->gameSettings->touchMap)
	{
		ReiMinimap &minimap = ReiMinimap::getInstance();
		minimap.init(mc);
		minimap.setEnabled(true);
		minimap.update();
		// ReiMinimap anchors its render at (screenWidth-64-6, 6). Pass a
		// fake screenWidth of 70 so the map lands at (0, 6) in local
		// space, then translate + scale into the slot. The 64 px map
		// stretches to the slot's full size; the cardinal-direction
		// labels and the border scale with it.
		const float_t mapScale = static_cast<float_t>(touchHud::MINIMAP_SIZE) / 64.0f;
		renderPushMatrix();
		renderTranslate(static_cast<float_t>(touchHud::MINIMAP_X),
			static_cast<float_t>(touchHud::MINIMAP_Y) - 6.0f * mapScale, 0.0f);
		renderScale(mapScale, mapScale, 1.0f);
		minimap.render(this, 70, touchHud::PANEL_HEIGHT, partialTick);
		renderPopMatrix();
	}

	// The hotbar's item icons: the same path the containers use, centred
	// in each stretched slot.
	RenderHelper::enableGUIStandardItemLighting();
	for (int_t slot = 0; slot < 9; ++slot)
	{
		const int_t slotW = touchHud::HOTBAR_W / touchHud::HOTBAR_SLOTS;
		renderTouchHotbarSlot(slot, touchHud::HOTBAR_X + slot * slotW + (slotW - 16) / 2,
			touchHud::HOTBAR_Y + (touchHud::HOTBAR_H - 16) / 2, partialTick);
	}
	RenderHelper::disableStandardItemLighting();

	// Action buttons down the right edge: inventory (chest front), crafting
	// (workbench top), pause (procedural bars).
	drawTouchHudButton(touchHud::BUTTON_INVENTORY_Y, "/terrain.png", 27);
	// Creative cannot craft (Minecraft's own key handler refuses the
	// screen there), so the button hides with the mode.
	if (mc->playerController == nullptr || !mc->playerController->isInCreativeMode())
		drawTouchHudButton(touchHud::BUTTON_CRAFTING_Y, "/terrain.png", 43);
	drawTouchHudButton(touchHud::BUTTON_PAUSE_Y, nullptr, 0);
}

void GuiIngame::renderTouchChestIcon(int_t x, int_t y)
{
	// 2D front-face icon cropped straight from /item/chest.png (64x64),
	// never a 3D/tile-entity render on the HUD pass. Two stacked quads
	// filling the full 24x24 icon cell (same size as the terrain-tile
	// buttons, e.g. the crafting table), using the same ModelChest UVs
	// the 3D model uses:
	//   lid front  = chestLid  front face, u 14..28 / v 14..19 (5px -> 8)
	//   body front = chestBelow front face, u 14..28 / v 33..43 (10px -> 16)
	//   latch      = chestKnob front face, u 1..3 / v 1..5 (silver, on top)
	// -> the classic closed-chest look at 24x24, matching the other icons.
	// Java-convention UVs, same convention the terrain icons above use.
	if (mc == nullptr || mc->renderEngine == nullptr)
		return;
	renderBindTexture(mc->renderEngine->getTexture("/item/chest.png"));
	Tessellator *tess = &Tessellator::instance;
	const float_t inv = 1.0f / 64.0f;

	tess->startDrawingQuads();
	tess->setColorOpaque_I(0xffffff);
	tess->addVertexWithUV(x, y + 8, zLevel, 14.0f * inv, 19.0f * inv);
	tess->addVertexWithUV(x + 24, y + 8, zLevel, 28.0f * inv, 19.0f * inv);
	tess->addVertexWithUV(x + 24, y, zLevel, 28.0f * inv, 14.0f * inv);
	tess->addVertexWithUV(x, y, zLevel, 14.0f * inv, 14.0f * inv);
	tess->draw();

	tess->startDrawingQuads();
	tess->setColorOpaque_I(0xffffff);
	tess->addVertexWithUV(x, y + 24, zLevel, 14.0f * inv, 43.0f * inv);
	tess->addVertexWithUV(x + 24, y + 24, zLevel, 28.0f * inv, 43.0f * inv);
	tess->addVertexWithUV(x + 24, y + 8, zLevel, 28.0f * inv, 33.0f * inv);
	tess->addVertexWithUV(x, y + 8, zLevel, 14.0f * inv, 33.0f * inv);
	tess->draw();

	// chestKnob latch front face: silver pixels at u 1..3 / v 1..5 (2x4),
	// centred horizontally and straddling the lid seam (row 8), the same
	// proportion the 3D model gives it (2/14 of the width).
	tess->startDrawingQuads();
	tess->setColorOpaque_I(0xffffff);
	tess->addVertexWithUV(x + 10, y + 11, zLevel, 1.0f * inv, 5.0f * inv);
	tess->addVertexWithUV(x + 13, y + 11, zLevel, 3.0f * inv, 5.0f * inv);
	tess->addVertexWithUV(x + 13, y + 5, zLevel, 3.0f * inv, 1.0f * inv);
	tess->addVertexWithUV(x + 10, y + 5, zLevel, 1.0f * inv, 1.0f * inv);
	tess->draw();
}

void GuiIngame::drawTouchHudButton(int_t y, const char *iconTexture, int_t iconTile)
{
	zLevel = 0.0f;
	// A 40x40 tile: translucent black fill, a light frame, and a 24x24 icon
	// drawn from the game's own terrain atlas -- the same convention the
	// dual-screen main-menu button icons use.
	drawRect(touchHud::BUTTON_X, y, touchHud::BUTTON_X + touchHud::BUTTON_W, y + touchHud::BUTTON_H,
		static_cast<int_t>(0xB0000000u));
	drawRect(touchHud::BUTTON_X, y, touchHud::BUTTON_X + touchHud::BUTTON_W, y + 1, 0x80ffffff);
	drawRect(touchHud::BUTTON_X, y + touchHud::BUTTON_H - 1,
		touchHud::BUTTON_X + touchHud::BUTTON_W, y + touchHud::BUTTON_H, 0x80000000);

	const int_t iconX = touchHud::BUTTON_X + (touchHud::BUTTON_W - 24) / 2;
	const int_t iconY = y + (touchHud::BUTTON_H - 24) / 2;
	if (iconTexture == nullptr)
	{
		// Pause: the universal two-bar glyph, plain quads.
		drawRect(iconX + 6, iconY + 2, iconX + 10, iconY + 22, 0xffffffff);
		drawRect(iconX + 14, iconY + 2, iconX + 18, iconY + 22, 0xffffffff);
		return;
	}

	// The chest button icon (terrain tile 27) is the one cell modern packs
	// blank out on purpose -- a 1.x+ pack keeps the chest OUT of the
	// terrain atlas (it is a tile entity with its own sheet), so the
	// terrain-sampled icon renders as an empty cell. Crop the chest FRONT
	// from /item/chest.png instead: pack-proof (the sheet always exists,
	// incl. the vanilla fallback) and no tile-entity render on the HUD pass.
	if (iconTile == 27 && iconTexture != nullptr &&
	    std::strcmp(iconTexture, "/terrain.png") == 0)
	{
		renderTouchChestIcon(iconX, iconY);
		return;
	}

	renderBindTexture(mc->renderEngine->getTexture(iconTexture));
	Tessellator *tess = &Tessellator::instance;
	const float u0 = static_cast<float>(iconTile % 16) / 16.0f;
	const float v0 = static_cast<float>(iconTile / 16) / 16.0f;
	tess->startDrawingQuads();
	tess->setColorOpaque_I(0xffffff);
	tess->addVertexWithUV(iconX, iconY + 24, zLevel, u0, v0 + 1.0f / 16.0f);
	tess->addVertexWithUV(iconX + 24, iconY + 24, zLevel, u0 + 1.0f / 16.0f, v0 + 1.0f / 16.0f);
	tess->addVertexWithUV(iconX + 24, iconY, zLevel, u0 + 1.0f / 16.0f, v0);
	tess->addVertexWithUV(iconX, iconY, zLevel, u0, v0);
	tess->draw();
}
#endif

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

#if defined(CTR_PLATFORM)
void GuiIngame::renderTouchHotbarSlot(int_t slot, int_t x, int_t y, float_t partialTick)
{
	ItemStack *stack = mc->thePlayer->inventory->mainInventory[slot];

	// Live-only slots: the pickup pop animation scales on the partial tick
	// and the enchantment glint slides its UVs on the wall clock
	// (RenderItem::renderGuiItemGlint) -- both must draw every frame, so
	// they bypass the list cache entirely.
	const bool dynamic = stack != nullptr &&
		((static_cast<float_t>(stack->animationsToGo) - partialTick) > 0.0f || stack->hasEffect());

	if (ctrHotbarListBase == 0)
		ctrHotbarListBase = renderGenerateDisplayLists(9);
	const int_t list = ctrHotbarListBase + slot;

	if (dynamic)
	{
		// Not cached this frame AND force a re-record once it settles.
		ctrHotbarSignature[slot] = -1;
		renderInventorySlot(slot, x, y, partialTick);
		return;
	}

	// Content signature: everything renderItemIntoGUI / the overlay can
	// ever draw derives from these three fields (the icon and damage bar
	// come from itemID+damage, the stack count text from stackSize).
	long_t signature = 0;
	if (stack != nullptr)
	{
		signature = 1;
		signature = signature * 1000003 + stack->itemID;
		signature = signature * 1000003 + stack->getItemDamage();
		signature = signature * 1000003 + stack->stackSize;
	}

	if (ctrHotbarSignature[slot] != signature)
	{
		// Re-record into the slot's display list: on this backend the
		// capture is linear-resident and replays with zero per-frame
		// staging (see DisplayListEntry).
		renderBeginDisplayList(list);
		if (stack != nullptr)
		{
			itemRenderer->renderItemIntoGUI(mc->fontRenderer, mc->renderEngine, stack, x, y);
			renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
			itemRenderer->renderItemOverlayIntoGUI(mc->fontRenderer, mc->renderEngine, stack, x, y);
			renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
		}
		renderEndDisplayList();
		ctrHotbarSignature[slot] = signature;
	}
	if (stack != nullptr)
		renderCallDisplayList(list);
}
#endif

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
	recordPlaying = uiText("Now playing: ") + record;
	recordPlayingUpFor = 60;
	field_22065_l = true;
}

void GuiIngame::addChatMessageTranslate(const std::string &key)
{
	StringTranslate *tr = StringTranslate::getInstance();
	addChatMessage(tr->translateKey(key));
}
