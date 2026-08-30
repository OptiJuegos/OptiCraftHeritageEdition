#pragma once

#include "GuiScreen.h"
#include <string>
#include <vector>

class GuiTextField;

// net.minecraft.src.GuiChat
class GuiChat : public GuiScreen
{
public:
	GuiChat();
	explicit GuiChat(const std::string &initialMessage);
	~GuiChat() override;

	void initGui() override;
	void onGuiClosed() override;
	void updateScreen() override;
	void handleMouseInput() override;
	void confirmClicked(bool confirmed, int_t id) override;
	void completePlayerName();
	void getSentHistory(int_t direction);

protected:
	void keyTyped(char_t c, int_t key) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
	std::string getMessage() const;
	void setMessage(const std::string &message);

private:
	std::string historyBuffer;
	int_t sentHistoryCursor;
	bool playerNamesFound;
	int_t autocompleteIndex;
	std::vector<std::string> autocompleteNames;
	std::string pendingUrl;
	GuiTextField *messageField;
	std::string initialMessage;
};
