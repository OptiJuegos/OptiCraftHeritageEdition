#include "platform/LegacyControlPromptBackend.h"

#ifdef CTR_PLATFORM
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/KeyBinding.h"

namespace
{
// Same shape as the PS2 helper, but with no pad-name table of its own: pad
// codes resolve inside lwjgl::Keyboard::getKeyName(), which consults
// dsPadKeyName() first (Keyboard_ps2 does the same with ps2PadKeyName), so a
// bound pad button already comes back as "A"/"Y"/"SELECT" down this path.
std::string bindingLabel(const KeyBinding *binding)
{
    if (binding == nullptr)
        return std::string();
    return GameSettings::getKeyDisplayString(binding->keyCode);
}
}

std::string legacyControlPromptLabel(const GameSettings &settings, LegacyControlAction action)
{
    switch (action)
    {
    case LegacyControlAction::Inventory: return bindingLabel(settings.keyBindInventory);
    // No button in the decided layout is spare for a drop, so nothing on this
    // console emits one. Report no label at all rather than keyBindDrop's
    // keyboard default, which would promise a button that does nothing. See
    // GameSettingsBackend_3DS.cpp.
    case LegacyControlAction::Drop: return std::string();
    case LegacyControlAction::Jump: return bindingLabel(settings.keyBindJump);
    // Attack and Use are mouse pseudo-binds (-100 / -99) rather than pad
    // codes, so there is no binding to look a name up in -- hence the literal,
    // for the same reason the PS2 hardcodes "R2"/"L2".
    case LegacyControlAction::Attack: return "X";
    case LegacyControlAction::Use: return "B";
    }
    return std::string();
}
#endif
