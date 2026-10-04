#pragma once

#include "Gui.h"
#include "platform/PlatformConfig.h"
#include <vector>
#include <string>

class Minecraft;
class RenderItem;
class ChatLine;
class Random;
class FontRenderer;
class ChatClickData;
class Tessellator;
// The 3DS reuses the PS2's static-mesh HUD cache for the crosshair/status
// cluster -- its hotbar stays on the touch panel, so ps2RenderHotbarFrame is
// never called on CTR (the cache entry it would fill simply stays invalid).
#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
struct Ps2HudCache;
#endif

// net.minecraft.src.GuiIngame
class GuiIngame : public Gui
{
public:
	GuiIngame(Minecraft *minecraft);
	~GuiIngame();

	void renderGameOverlay(float_t partialTick, bool showDebug, int_t mouseX, int_t mouseY);

private:
	void renderPumpkinBlur(int_t w, int_t h);
	void renderVignette(float_t brightness, int_t w, int_t h);
	void renderPortalOverlay(float_t intensity, int_t w, int_t h);
	void renderInventorySlot(int_t slot, int_t x, int_t y, float_t partialTick);
	void renderDebugOverlay(FontRenderer *fontRenderer, int_t screenWidth);
	void renderFpsOverlay(FontRenderer *fontRenderer);
	void renderBossHealth();
	void renderPlayerStatusHudGeometry(int_t sw, int_t sh, Tessellator *captureTessellator);
	void renderPlayerStatusHudUncached(int_t sw, int_t sh);
#if PLATFORM_PC_LEGACY
	void pcLegacyEnsureHudCacheLists(int_t sw, int_t sh);
	void pcLegacyRenderHotbarFrame(int_t sw, int_t sh, int_t currentItem);
	void pcLegacyRenderCrosshair(int_t sw, int_t sh);
	void pcLegacyRenderPlayerStatusHud(int_t sw, int_t sh);
#endif
#if defined(CTR_PLATFORM)
	void renderGameplayBottomPanel(float_t partialTick);
	void drawTouchHudButton(int_t y, const char *iconTexture, int_t iconTile);

	// The inventory button's chest glyph as a 2D front-face crop of
	// /item/chest.png (pack-proof; see drawTouchHudButton's comment for
	// why terrain tile 27 cannot be trusted).
	void renderTouchChestIcon(int_t x, int_t y);
	// Bottom-panel hotbar slot cache: the untouched-slot case replays a
	// recorded display list (linear-resident on this backend) instead of
	// re-tessellating the icon every frame.
	void renderTouchHotbarSlot(int_t slot, int_t x, int_t y, float_t partialTick);
#endif
#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
	void ps2RenderHotbarFrame(int_t sw, int_t sh, int_t currentItem);
	void ps2RenderCrosshair(int_t sw, int_t sh);
	void ps2RenderPlayerStatusHud(int_t sw, int_t sh);
#endif

public:
	void updateTick();
	void clearChatMessages();
	void addChatMessage(const std::string &msg);
	void setRecordPlayingMessage(const std::string &record);
	void addChatMessageTranslate(const std::string &key);
	const std::vector<std::string> &getSentMessages() const { return sentMessages; }
	std::vector<std::string> &getSentMessages() { return sentMessages; }
	void resetChatScroll();
	void scrollChat(int_t amount);
	bool isChatOpen() const;
	ChatClickData *getChatClickData(int_t rawMouseX, int_t rawMouseY);

	// MCP 1.2.5 compatibility names.
	const std::vector<std::string> &func_50013_c() const { return getSentMessages(); }
	std::vector<std::string> &func_50013_c() { return getSentMessages(); }
	void func_50014_d() { resetChatScroll(); }
	void func_50011_a(int_t amount) { scrollChat(amount); }
	ChatClickData *func_50012_a(int_t x, int_t y) { return getChatClickData(x, y); }

private:
	static RenderItem *itemRenderer;
	std::vector<ChatLine *> chatMessageList;
	std::vector<std::string> sentMessages;
	Random *rand;
	Minecraft *mc;

public:
	std::string field_933_a;   // tab-completion target

private:
	int_t updateCounter;
	std::string recordPlaying;
	int_t recordPlayingUpFor;
	bool field_22065_l;        // record playing with color effect
	int_t chatScroll;
	bool isScrolled;
#if PLATFORM_PC_LEGACY
	int_t pcLegacyHudDisplayLists;
	int_t pcLegacyHudWidth;
	int_t pcLegacyHudHeight;
	int_t pcLegacyHotbarItem;
	bool pcLegacyHotbarValid;
	bool pcLegacyCrosshairValid;
	bool pcLegacyStatusValid;
	unsigned long long pcLegacyStatusSignature;
#endif
#if defined(PS2_PLATFORM) || defined(CTR_PLATFORM)
	Ps2HudCache *ps2HudCache;
#endif
#if defined(CTR_PLATFORM)
	// Per-slot hotbar display lists on the bottom panel; 0 until lazily
	// generated, signature -1 forces re-record (dynamic/animated slots).
	int_t ctrHotbarListBase = 0;
	long_t ctrHotbarSignature[9] = {};
#endif

public:
	float_t damageGuiPartialTime;
	float_t prevVignetteBrightness;
};
