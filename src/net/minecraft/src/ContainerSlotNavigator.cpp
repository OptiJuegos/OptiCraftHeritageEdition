#include "ContainerSlotNavigator.h"

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM) || defined(CTR_PLATFORM)

#include <algorithm>

#include "Container.h"
#include "GuiContainer.h"
#include "Slot.h"
#include "Minecraft.h"
#include "platform/ConsoleInputClock.h"
#include "platform/Input.h"

namespace
{
const int_t SLOT_CENTER = 8;
const int_t PERPENDICULAR_WEIGHT = 4;
const int REPEAT_DELAY_MS = 250;
const int REPEAT_INTERVAL_MS = 90;

int_t absInt(int_t value)
{
    return value < 0 ? -value : value;
}
}

ContainerSlotNavigator& ContainerSlotNavigator::instance(int padPort)
{
    static ContainerSlotNavigator s_navigators[2];
    if (padPort < 0)
        padPort = platformMenuPad();
    if (padPort < 0 || padPort >= 2)
        padPort = 0;
    s_navigators[padPort].m_padPort = padPort;
    return s_navigators[padPort];
}

void ContainerSlotNavigator::notifyOpen(GuiContainer *guiContainer, const Layout &guiLayout)
{
    if (guiContainer == nullptr)
        return;

    platformSetContainerNavigationActive(true);
    if (screen != guiContainer)
    {
        screen = guiContainer;
        selected = nullptr;
        pointerSlot = nullptr;
        controllerActive = false;
        pendingPrimary = false;
        pendingSecondary = false;
        nextRepeatMs = consoleInputNowMs() + REPEAT_DELAY_MS;
    }
    layout = guiLayout;
    repairSelection();
}

void ContainerSlotNavigator::notifyClosed(const GuiContainer *guiContainer)
{
    if (screen != guiContainer)
        return;

    screen = nullptr;
    selected = nullptr;
    pointerSlot = nullptr;
    controllerActive = false;
    ignorePointerMotionOnce = false;
    pendingPrimary = false;
    pendingSecondary = false;
    nextRepeatMs = 0;
    if (!instance(0).isActive() && !instance(1).isActive())
        platformSetContainerNavigationActive(false);
}

void ContainerSlotNavigator::repairSelection()
{
    if (screen == nullptr || screen->inventorySlots == nullptr)
    {
        selected = nullptr;
        return;
    }

    const std::vector<Slot *> &slots = screen->inventorySlots->slots;
    if (slots.empty())
    {
        selected = nullptr;
        return;
    }

    if (selected != nullptr && std::find(slots.begin(), slots.end(), selected) != slots.end())
        return;

    selected = nullptr;
    for (Slot *slot : slots)
    {
        if (slot != nullptr)
        {
            selected = slot;
            break;
        }
    }
}

void ContainerSlotNavigator::clearControllerSelection()
{
    controllerActive = false;
    pendingPrimary = false;
    pendingSecondary = false;
}

void ContainerSlotNavigator::notePointerActivity()
{
    // The motion event produced by moveMenuCursorToSelection() is not the
    // player reaching for the pointer.
    if (ignorePointerMotionOnce)
    {
        ignorePointerMotionOnce = false;
        return;
    }
    clearControllerSelection();
}

void ContainerSlotNavigator::moveMenuCursorToSelection()
{
    // Keep the hidden platform pointer on the D-pad slot too. GameCube and
    // Classic can then hand control to the stick without the cursor jumping
    // back to its previous position.
    if (selected == nullptr || layout.screenWidth <= 0 || layout.screenHeight <= 0)
        return;

    const int_t guiX = layout.guiLeft + selected->xDisplayPosition + SLOT_CENTER;
    const int_t guiY = layout.guiTop + selected->yDisplayPosition + SLOT_CENTER;
    ignorePointerMotionOnce = platformMenuPointerActive();
    if (m_padPort == 0)
    {
        platformSetMenuCursor(guiX * layout.displayWidth / layout.screenWidth,
                              guiY * layout.displayHeight / layout.screenHeight);
    }
    Minecraft *mc = Minecraft::getMinecraft();
    if (mc != nullptr)
    {
        mc->setPlayerCursor(m_padPort, static_cast<float>(guiX), static_cast<float>(guiY));
    }
}

void ContainerSlotNavigator::notePointerSlot(Slot *slot)
{
    pointerSlot = slot;
}

void ContainerSlotNavigator::activateControllerSelection()
{
    repairSelection();
    // Taking over from the pointer starts at the slot it is hovering rather
    // than at the first slot of the screen (or wherever the D-pad last was).
    if (!controllerActive && pointerSlot != nullptr)
        selected = pointerSlot;
    controllerActive = selected != nullptr;
}

bool ContainerSlotNavigator::consumePrimaryClick()
{
    const bool value = pendingPrimary;
    pendingPrimary = false;
    return value;
}

bool ContainerSlotNavigator::consumeSecondaryClick()
{
    const bool value = pendingSecondary;
    pendingSecondary = false;
    return value;
}

bool ContainerSlotNavigator::consumeShiftMoveClick()
{
    const bool value = pendingShiftMove;
    pendingShiftMove = false;
    return value;
}

Slot *ContainerSlotNavigator::pickSlot(int_t originX, int_t originY, int_t dirX, int_t dirY) const
{
    if (screen == nullptr || screen->inventorySlots == nullptr)
        return nullptr;

    const std::vector<Slot *> &slots = screen->inventorySlots->slots;
    Slot *forward = nullptr;
    int_t forwardScore = 0;
    Slot *wrapped = nullptr;
    int_t wrappedScore = 0;

    for (Slot *slot : slots)
    {
        if (slot == nullptr || slot == selected)
            continue;

        const int_t x = layout.guiLeft + slot->xDisplayPosition + SLOT_CENTER;
        const int_t y = layout.guiTop + slot->yDisplayPosition + SLOT_CENTER;
        const int_t along = (x - originX) * dirX + (y - originY) * dirY;
        const int_t perpendicular = dirX != 0 ? absInt(y - originY) : absInt(x - originX);

        if (along > 0)
        {
            const int_t score = along + perpendicular * PERPENDICULAR_WEIGHT;
            if (forward == nullptr || score < forwardScore)
            {
                forward = slot;
                forwardScore = score;
            }
        }
        else
        {
            const int_t score = (x * dirX + y * dirY) + perpendicular * PERPENDICULAR_WEIGHT;
            if (wrapped == nullptr || score < wrappedScore)
            {
                wrapped = slot;
                wrappedScore = score;
            }
        }
    }

    return forward != nullptr ? forward : wrapped;
}

void ContainerSlotNavigator::tick()
{
    if (!isActive() || screen == nullptr || screen->inventorySlots == nullptr || screen->inventorySlots->slots.empty())
        return;

    tickWithInput(platformTextInputSnapshot(m_padPort));
}

void ContainerSlotNavigator::tickWithInput(const PlatformTextInputSnapshot &pad)
{
    if (!isActive() || screen->inventorySlots == nullptr || screen->inventorySlots->slots.empty())
        return;

    repairSelection();
#ifdef WII_PLATFORM
    // Wii exposes pointer ownership explicitly. When a GameCube/Classic left
    // stick takes over, drop the D-pad highlight before consuming A/B so the
    // same frame is routed to the slot under the cursor. PS2 reports its menu
    // pointer as always active, so this handoff is intentionally Wii-only.
    if (platformMenuPointerActive())
        clearControllerSelection();
#endif

    if (!pad.connected)
        return;

    Minecraft *mc = Minecraft::getMinecraft();
    const bool splitScreen = (mc != nullptr && mc->isSplitScreenActive());

    const unsigned int keyLeft = PLATFORM_TEXT_LEFT;
    const unsigned int keyRight = PLATFORM_TEXT_RIGHT;
    const unsigned int keyUp = PLATFORM_TEXT_UP;
    const unsigned int keyDown = PLATFORM_TEXT_DOWN;

    // In splitscreen, clicks are processed directly in handleSplitscreenPlayerInput()
    // rather than deferred to drawScreen().
    if (!splitScreen)
    {
        const bool pointerOwnsClick = platformMenuPointerActive() && !controllerActive;
        if (!pointerOwnsClick && (pad.pressed & PLATFORM_TEXT_TYPE) != 0)
        {
            activateControllerSelection();
            if (controllerSelectionActive())
                pendingPrimary = true;
        }
        if (!pointerOwnsClick && (pad.pressed & PLATFORM_TEXT_BACK) != 0)
        {
            activateControllerSelection();
            if (controllerSelectionActive())
                pendingSecondary = true;
        }
        // Quick-move rides the pad's SHIFT bit; screens that never map one
        // simply never set this.
        if (!pointerOwnsClick && (pad.pressed & PLATFORM_TEXT_SHIFT) != 0)
        {
            activateControllerSelection();
            if (controllerSelectionActive())
                pendingShiftMove = true;
        }
    }

    int_t dirX = 0;
    int_t dirY = 0;
    if (pad.pressed & keyLeft) dirX = -1;
    else if (pad.pressed & keyRight) dirX = 1;
    else if (pad.pressed & keyUp) dirY = -1;
    else if (pad.pressed & keyDown) dirY = 1;

    const int now = consoleInputNowMs();
    const unsigned int heldDpad = pad.held & (keyLeft | keyRight | keyUp | keyDown);
    if (dirX == 0 && dirY == 0 && heldDpad != 0 && now >= nextRepeatMs)
    {
        if (pad.held & keyLeft) dirX = -1;
        else if (pad.held & keyRight) dirX = 1;
        else if (pad.held & keyUp) dirY = -1;
        else if (pad.held & keyDown) dirY = 1;
        nextRepeatMs = now + REPEAT_INTERVAL_MS;
    }
    else if ((pad.pressed & heldDpad) != 0)
    {
        nextRepeatMs = now + REPEAT_DELAY_MS;
    }

    if (dirX == 0 && dirY == 0)
        return;

    activateControllerSelection();
    if (selected == nullptr)
        return;

    const int_t originX = layout.guiLeft + selected->xDisplayPosition + SLOT_CENTER;
    const int_t originY = layout.guiTop + selected->yDisplayPosition + SLOT_CENTER;
    Slot *target = screen->getControllerNavigationTarget(selected, dirX, dirY);
    if (target == nullptr)
        target = pickSlot(originX, originY, dirX, dirY);
    if (target != nullptr)
        selected = target;
    moveMenuCursorToSelection();
}

#endif // PS2_PLATFORM || WII_PLATFORM
