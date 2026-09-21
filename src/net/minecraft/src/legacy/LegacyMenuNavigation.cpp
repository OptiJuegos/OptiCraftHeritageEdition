#include "LegacyMenuNavigation.h"

#include "platform/PlatformConfig.h"
#include "net/minecraft/src/GuiButton.h"
#include "platform/Input.h"

namespace
{
bool selectable(const GuiButton *button)
{
    return button != nullptr && button->enabled && button->enabled2;
}
}

int_t legacyFirstSelectableButton(const std::vector<GuiButton *> &buttons)
{
    for (int_t i = 0; i < static_cast<int_t>(buttons.size()); ++i)
    {
        if (selectable(buttons[i]))
            return i;
    }
    return -1;
}

int_t legacyNextSelectableButton(const std::vector<GuiButton *> &buttons, int_t currentIndex, int_t direction)
{
    if (buttons.empty() || direction == 0)
        return currentIndex;

    const int_t count = static_cast<int_t>(buttons.size());

    // Do not wrap around from top to bottom when pressing Up, or bottom to top when pressing Down.
    if (direction < 0 && currentIndex >= 0 && currentIndex <= legacyFirstSelectableButton(buttons))
        return currentIndex;

    if (direction > 0 && currentIndex >= 0)
    {
        int_t lastSelectable = -1;
        for (int_t i = count - 1; i >= 0; --i)
        {
            if (selectable(buttons[i]))
            {
                lastSelectable = i;
                break;
            }
        }
        if (currentIndex >= lastSelectable && lastSelectable >= 0)
            return currentIndex;
    }

    int_t index = currentIndex;
    if (index < 0 || index >= count)
        index = direction > 0 ? -1 : 0;

    for (int_t attempt = 0; attempt < count; ++attempt)
    {
        index = (index + (direction > 0 ? 1 : -1) + count) % count;
        if (selectable(buttons[index]))
            return index;
    }
    return currentIndex;
}

int_t legacyHoveredSelectableButton(const std::vector<GuiButton *> &buttons, int_t mouseX, int_t mouseY)
{
#if PLATFORM_PS2
    (void)buttons;
    (void)mouseX;
    (void)mouseY;
    return -1;
#else
#if PLATFORM_WII
    if (!platformMenuPointerActive())
        return -1;
#endif
    for (int_t i = 0; i < static_cast<int_t>(buttons.size()); ++i)
    {
        GuiButton *button = buttons[i];
        if (!selectable(button))
            continue;
        if (mouseX >= button->xPosition && mouseY >= button->yPosition &&
            mouseX < button->xPosition + button->getButtonWidth() &&
            mouseY < button->yPosition + button->getButtonHeight())
            return i;
    }
    return -1;
#endif
}

void legacyApplyMenuSelection(const std::vector<GuiButton *> &buttons, int_t selectedIndex)
{
    for (int_t i = 0; i < static_cast<int_t>(buttons.size()); ++i)
    {
        if (buttons[i] != nullptr)
            buttons[i]->setKeyboardSelected(i == selectedIndex);
    }
}

void legacyMoveMenuCursorToSelection(const std::vector<GuiButton *> &buttons, int_t selectedIndex)
{
#if PLATFORM_WII
    if (selectedIndex < 0 || selectedIndex >= static_cast<int_t>(buttons.size()))
        return;
    GuiButton *button = buttons[selectedIndex];
    if (!selectable(button))
        return;
    platformSetMenuCursor(button->xPosition + button->getButtonWidth() / 2,
        button->yPosition + button->getButtonHeight() / 2);
#else
    (void)buttons;
    (void)selectedIndex;
#endif
}
