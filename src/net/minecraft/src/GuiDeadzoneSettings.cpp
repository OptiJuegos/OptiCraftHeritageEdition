#include "GuiDeadzoneSettings.h"

#include "GameSettings.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "platform/PlatformUserSettings.h"
#include "platform/PlatformConfig.h"
#include "platform/Input.h"

namespace
{
	const float MIN_DEADZONE = 0.05f;
	const float MAX_DEADZONE = 0.35f;
	const float DEADZONE_STEP = 0.05f;

	const int_t TITLE_Y = 30;
	const int_t PREVIEW_RADIUS = 24;
	const int_t PREVIEW_GAP = 20;
	const int_t PREVIEW_LABEL_GAP = 10;
}

GuiDeadzoneSettings::GuiDeadzoneSettings(GuiScreen *parent, GameSettings *options)
	: parentScreen(parent), settings(options), valueButton(nullptr)
{
}

int_t GuiDeadzoneSettings::getPreviewCenterY() const
{
	// Keep the preview between the title and the controls, but never let it
	// climb high enough to overlap the title or the stick labels above it.
	const int_t minCenterY = TITLE_Y + 8 + 8 + PREVIEW_LABEL_GAP + 8 + PREVIEW_RADIUS;
	const int_t centerY = height / 2 - 36;
	return centerY < minCenterY ? minCenterY : centerY;
}

int_t GuiDeadzoneSettings::getControlsY() const
{
	const int_t minControlsY = getPreviewCenterY() + PREVIEW_RADIUS + 14;
	const int_t controlsY = height / 2 + 12;
	return controlsY < minControlsY ? minControlsY : controlsY;
}

void GuiDeadzoneSettings::initGui()
{
	controlList.clear();
	const int_t controlsY = getControlsY();
	controlList.push_back(new GuiButton(201, width / 2 - 100, controlsY, 48, 20, "-"));
	valueButton = new GuiButton(202, width / 2 - 48, controlsY, 96, 20, "");
	valueButton->enabled = false;
	controlList.push_back(valueButton);
	controlList.push_back(new GuiButton(203, width / 2 + 52, controlsY, 48, 20, "+"));
	controlList.push_back(new GuiButton(204, width / 2 - 100, controlsY + 30, "Reset to 20%"));
	controlList.push_back(new GuiButton(200, width / 2 - 100, controlsY + 54, "Done"));
	updateValueButton();
}

void GuiDeadzoneSettings::changeDeadzone(float delta)
{
	settings->controllerDeadzone += delta;
	if (settings->controllerDeadzone < MIN_DEADZONE)
		settings->controllerDeadzone = MIN_DEADZONE;
	if (settings->controllerDeadzone > MAX_DEADZONE)
		settings->controllerDeadzone = MAX_DEADZONE;
	PlatformUserSettings::setControllerDeadzone(settings->controllerDeadzone);
	settings->saveOptions();
	updateValueButton();
}

void GuiDeadzoneSettings::updateValueButton()
{
	if (valueButton == nullptr)
		return;
	const int percent = (int)(settings->controllerDeadzone * 100.0f + 0.5f);
	valueButton->displayString = std::to_string(percent) + "%";
}

void GuiDeadzoneSettings::actionPerformed(GuiButton *button)
{
	if (button == nullptr || !button->enabled)
		return;
	if (button->id == 201)
		changeDeadzone(-DEADZONE_STEP);
	else if (button->id == 203)
		changeDeadzone(DEADZONE_STEP);
	else if (button->id == 204)
	{
		settings->controllerDeadzone = 0.20f;
		changeDeadzone(0.0f);
	}
	else if (button->id == 200)
		mc->displayGuiScreen(parentScreen);
}

void GuiDeadzoneSettings::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		mc->displayGuiScreen(parentScreen);
		return;
	}
	GuiScreen::keyTyped(c, key);
}

#if PLATFORM_HAS_CONTROLLER_CALIBRATION
void GuiDeadzoneSettings::drawStickPreview(int_t centerX, int_t centerY, float stickX, float stickY, bool connected)
{
	const int_t deadzoneRadius = (int_t)(PREVIEW_RADIUS * settings->controllerDeadzone);
	drawRect(centerX - PREVIEW_RADIUS, centerY - PREVIEW_RADIUS,
		centerX + PREVIEW_RADIUS, centerY + PREVIEW_RADIUS, 0xff202020);
	drawRect(centerX - deadzoneRadius, centerY - deadzoneRadius,
		centerX + deadzoneRadius + 1, centerY + deadzoneRadius + 1, 0xff505050);
	drawRect(centerX - 1, centerY - 1, centerX + 2, centerY + 2, 0xffffffff);
	if (connected)
	{
		const int_t markerX = centerX + (int_t)(stickX * PREVIEW_RADIUS);
#if defined(PS2_PLATFORM)
		const int_t markerY = centerY + (int_t)(stickY * PREVIEW_RADIUS);
#else
		const int_t markerY = centerY - (int_t)(stickY * PREVIEW_RADIUS);
#endif
		drawRect(markerX - 2, markerY - 2, markerX + 3, markerY + 3, 0xffffc040);
	}
}
#endif

void GuiDeadzoneSettings::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, "Deadzone Settings", width / 2, TITLE_Y, 0xffffff);

#if PLATFORM_HAS_CONTROLLER_CALIBRATION
	const PlatformGamepadSnapshot stick = platformRawGamepadSnapshot(0);
	const int_t previewCenterY = getPreviewCenterY();
	const int_t previewOffsetX = PREVIEW_RADIUS + PREVIEW_GAP / 2;
	const int_t moveCenterX = width / 2 - previewOffsetX;
	const int_t cameraCenterX = width / 2 + previewOffsetX;
	const int_t labelY = previewCenterY - PREVIEW_RADIUS - PREVIEW_LABEL_GAP;

	drawCenteredString(fontRenderer, "Move", moveCenterX, labelY, 0xa0a0a0);
	drawCenteredString(fontRenderer, "Camera", cameraCenterX, labelY, 0xa0a0a0);

	drawStickPreview(moveCenterX, previewCenterY, stick.leftX, stick.leftY, stick.connected);
	drawStickPreview(cameraCenterX, previewCenterY, stick.rightX, stick.rightY, stick.connected);

	if (!stick.connected)
	{
		drawCenteredString(fontRenderer, "Connect a controller to preview input",
			width / 2, previewCenterY + PREVIEW_RADIUS + 5, 0x808080);
	}
#endif
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
