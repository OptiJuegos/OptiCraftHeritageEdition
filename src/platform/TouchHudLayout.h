#pragma once

#include "java/Type.h"

// Dual-screen gameplay HUD (3DS): the widget geometry of the bottom panel's
// touch layer. GuiIngame draws every widget from these constants and
// DsInput hit-tests the same rects, so the drawn and the tappable surfaces
// can never drift apart. The panel's canvas is 320x240.

namespace touchHud
{
constexpr int_t PANEL_WIDTH = 320;
constexpr int_t PANEL_HEIGHT = 240;

// Touch hotbar: the vanilla strip stretched across the panel's full width
// and made taller so the stretch does not read, nine equal touch slots.
constexpr int_t HOTBAR_X = 0;
constexpr int_t HOTBAR_Y = 4;
constexpr int_t HOTBAR_W = PANEL_WIDTH;
constexpr int_t HOTBAR_H = 36;
constexpr int_t HOTBAR_SLOTS = 9;

inline int hotbarSlotAt(int_t x)
{
	int slot = (x - HOTBAR_X) * HOTBAR_SLOTS / HOTBAR_W;
	if (slot < 0) slot = 0;
	if (slot > HOTBAR_SLOTS - 1) slot = HOTBAR_SLOTS - 1;
	return slot;
}

// Player coordinates (2x font), centred on a translucent black strip a
// little below the hotbar.
constexpr int_t COORDS_BAR_TOP = 46;
constexpr int_t COORDS_BAR_BOTTOM = 64;
constexpr int_t COORDS_TEXT_Y = 48;

// Map slot: the HELD MAP when one is equipped, otherwise ReiMinimap's
// native renderer. Centred in the space between the panel's left edge and
// the action buttons' column.
constexpr int_t MINIMAP_X = 34;
constexpr int_t MINIMAP_Y = 68;
constexpr int_t MINIMAP_SIZE = 168;

// Action buttons down the right edge, below the hotbar and the coordinates
// strip: inventory (chest), crafting, pause.
constexpr int_t BUTTON_X = 236;
constexpr int_t BUTTON_W = 80;
constexpr int_t BUTTON_H = 48;
constexpr int_t BUTTON_INVENTORY_Y = 72;
constexpr int_t BUTTON_CRAFTING_Y = 128;
constexpr int_t BUTTON_PAUSE_Y = 184;

enum class Widget
{
	None,
	Hotbar,
	Inventory,
	Crafting,
	Pause,
};

struct WidgetHit
{
	Widget widget = Widget::None;
	int slot = 0;
};

inline WidgetHit hitTest(int_t x, int_t y)
{
	if (x >= HOTBAR_X && x < HOTBAR_X + HOTBAR_W && y >= HOTBAR_Y && y < HOTBAR_Y + HOTBAR_H)
		return {Widget::Hotbar, hotbarSlotAt(x)};
	if (x >= BUTTON_X && x < BUTTON_X + BUTTON_W)
	{
		if (y >= BUTTON_INVENTORY_Y && y < BUTTON_INVENTORY_Y + BUTTON_H)
			return {Widget::Inventory, 0};
		if (y >= BUTTON_CRAFTING_Y && y < BUTTON_CRAFTING_Y + BUTTON_H)
			return {Widget::Crafting, 0};
		if (y >= BUTTON_PAUSE_Y && y < BUTTON_PAUSE_Y + BUTTON_H)
			return {Widget::Pause, 0};
	}
	return {Widget::None, 0};
}
} // namespace touchHud
