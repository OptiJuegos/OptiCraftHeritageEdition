#pragma once

#include "Gui.h"
#include <vector>
#include <string>

class Minecraft;
class RenderItem;
class ChatLine;
class Random;
class FontRenderer;
class ChatClickData;

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
	void renderBossHealth();

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

public:
	float_t damageGuiPartialTime;
	float_t prevVignetteBrightness;
};
