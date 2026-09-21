#pragma once

#include "GuiScreen.h"
#include "platform/PlatformConfig.h"

// net.minecraft.src.GuiIngameMenu
class GuiIngameMenu : public GuiScreen
{
public:
	GuiIngameMenu();

	void initGui() override;

protected:
	void handleSpecializedMenuInput() override;
	bool usesSpecializedMenuNavigation() const override;
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;

public:
	void updateScreen() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	void syncLegacySelection();
	void moveLegacySelection(int_t direction);
	void activateLegacySelection();
	void closeLegacyPause();

	int_t updateCounter2;
	int_t updateCounter;
	int_t selectedControlIndex;
	int_t hoveredControlIndex;
#if PLATFORM_PS2 || PLATFORM_WII
	long_t legacyPauseOpenedAtMillis;
#endif
#if PLATFORM_PS2
	bool ps2PauseStartReleaseLatch;
	bool ps2PauseActionReleaseLatch;
#endif
};
