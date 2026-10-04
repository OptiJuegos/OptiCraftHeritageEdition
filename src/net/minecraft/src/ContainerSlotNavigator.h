#pragma once

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM) || defined(CTR_PLATFORM)

#include "java/Type.h"
#include "platform/Input.h"

class GuiContainer;
class Slot;

class ContainerSlotNavigator
{
public:
    struct Layout
    {
        int_t guiLeft = 0;
        int_t guiTop = 0;
        int_t screenWidth = 0;
        int_t screenHeight = 0;
        int_t displayWidth = 0;
        int_t displayHeight = 0;
    };

    static ContainerSlotNavigator& instance(int padPort = -1);

    int getPadPort() const { return m_padPort; }
    void setPadPort(int port) { m_padPort = port; }

    void notifyOpen(GuiContainer *screen, const Layout &layout);
    void notifyClosed(const GuiContainer *screen);
    bool isActive() const { return screen != nullptr; }

    void tick();
    void tickWithInput(const PlatformTextInputSnapshot &pad);

    Slot *selectedSlot() const { return selected; }
    bool controllerSelectionActive() const { return controllerActive && selected != nullptr; }
    void notePointerActivity();
    // Slot under the platform pointer this frame (null when none or when the
    // pointer is not an active input). Seeds the D-pad selection.
    void notePointerSlot(Slot *slot);
    void activateControllerSelection();
    bool consumePrimaryClick();
    bool consumeSecondaryClick();
    // Quick-move (shift-click equivalent): captured from the pad's SHIFT bit
    // (PS2 Triangle, Wii Z/2, 3DS R) and consumed like the primary/secondary
    // clicks. Inactive on platforms whose menu maps carry no shift button.
    bool consumeShiftMoveClick();

private:
    ContainerSlotNavigator() = default;

    void repairSelection();
    void clearControllerSelection();
    void moveMenuCursorToSelection();
    Slot *pickSlot(int_t originX, int_t originY, int_t dirX, int_t dirY) const;

    GuiContainer *screen = nullptr;
    Layout layout;
    Slot *selected = nullptr;
    Slot *pointerSlot = nullptr;
    bool controllerActive = false;
    bool ignorePointerMotionOnce = false;
    bool pendingPrimary = false;
    bool pendingSecondary = false;
    bool pendingShiftMove = false;
    int nextRepeatMs = 0;
    int m_padPort = 0;
};

#endif // PS2_PLATFORM || WII_PLATFORM || CTR_PLATFORM
