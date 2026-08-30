#pragma once

#include "GuiScreen.h"
#include "platform/PlatformConfig.h"

class GameSettings;

class GuiDeadzoneSettings : public GuiScreen
{
public:
	GuiDeadzoneSettings(GuiScreen *parent, GameSettings *settings);

	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void keyTyped(char_t c, int_t key) override;
	void actionPerformed(GuiButton *button) override;

private:
	void changeDeadzone(float delta);
	void updateValueButton();
	int_t getPreviewCenterY() const;
	int_t getControlsY() const;
#if PLATFORM_HAS_CONTROLLER_CALIBRATION
	void drawStickPreview(int_t centerX, int_t centerY, float stickX, float stickY, bool connected);
#endif

	GuiScreen *parentScreen;
	GameSettings *settings;
	GuiButton *valueButton;
};
