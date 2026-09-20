#include "net/minecraft/src/UiStrings.h"
#include "LegacyControlsBinding.h"

#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/KeyBinding.h"
#include "platform/PlatformConfig.h"

#if PLATFORM_WII
#include "wii/input/WiiPadKeyCodes.h"
#endif

namespace
{
#if PLATFORM_WII
enum LegacyWiiAction
{
    ACTION_JUMP = 0,
    ACTION_SNEAK,
    ACTION_ATTACK,
    ACTION_USE,
    ACTION_DROP,
    ACTION_INVENTORY,
    ACTION_THIRDPERSON,
    ACTION_COUNT
};

std::string actionLabel(int_t action)
{
    switch (action)
    {
        case ACTION_JUMP: return uiText("Jump");
        case ACTION_SNEAK: return uiText("Sneak");
        case ACTION_ATTACK: return uiText("Attack");
        case ACTION_USE: return uiText("Use");
        case ACTION_DROP: return uiText("Drop");
        case ACTION_INVENTORY: return uiText("Inventory");
        case ACTION_THIRDPERSON: return "3rd Person";
        default: return "";
    }
}

int_t *fieldFor(GameSettings *settings, int_t action, LegacyControlsWiiFamily family)
{
    if (settings == nullptr)
        return nullptr;

    switch (action)
    {
        case ACTION_JUMP:
            if (family == LegacyControlsWiiFamily::GameCube) return &settings->wiiGcJump;
            if (family == LegacyControlsWiiFamily::Wiimote) return &settings->wiiWmJump;
            if (family == LegacyControlsWiiFamily::Classic) return &settings->wiiCcJump;
            break;
        case ACTION_SNEAK:
            if (family == LegacyControlsWiiFamily::GameCube) return &settings->wiiGcSneak;
            if (family == LegacyControlsWiiFamily::Wiimote) return &settings->wiiWmSneak;
            if (family == LegacyControlsWiiFamily::Classic) return &settings->wiiCcSneak;
            break;
        case ACTION_ATTACK:
            if (family == LegacyControlsWiiFamily::GameCube) return &settings->wiiGcAttack;
            if (family == LegacyControlsWiiFamily::Wiimote) return &settings->wiiWmAttack;
            if (family == LegacyControlsWiiFamily::Classic) return &settings->wiiCcAttack;
            break;
        case ACTION_USE:
            if (family == LegacyControlsWiiFamily::GameCube) return &settings->wiiGcUse;
            if (family == LegacyControlsWiiFamily::Wiimote) return &settings->wiiWmUse;
            if (family == LegacyControlsWiiFamily::Classic) return &settings->wiiCcUse;
            break;
        case ACTION_DROP:
            if (family == LegacyControlsWiiFamily::GameCube) return &settings->wiiGcDrop;
            if (family == LegacyControlsWiiFamily::Wiimote) return &settings->wiiWmDrop;
            if (family == LegacyControlsWiiFamily::Classic) return &settings->wiiCcDrop;
            break;
        case ACTION_INVENTORY:
            if (family == LegacyControlsWiiFamily::GameCube) return &settings->wiiGcInventory;
            if (family == LegacyControlsWiiFamily::Wiimote) return &settings->wiiWmInventory;
            if (family == LegacyControlsWiiFamily::Classic) return &settings->wiiCcInventory;
            break;
        case ACTION_THIRDPERSON:
            if (family == LegacyControlsWiiFamily::GameCube) return &settings->wiiGcThirdPerson;
            if (family == LegacyControlsWiiFamily::Wiimote) return &settings->wiiWmThirdPerson;
            if (family == LegacyControlsWiiFamily::Classic) return &settings->wiiCcThirdPerson;
            break;
        default:
            break;
    }
    return nullptr;
}

WiiPadFamily toWiiPadFamily(LegacyControlsWiiFamily family)
{
    if (family == LegacyControlsWiiFamily::GameCube)
        return WiiPadFamily::GameCube;
    if (family == LegacyControlsWiiFamily::Wiimote)
        return WiiPadFamily::Wiimote;
    return WiiPadFamily::Classic;
}

std::string familySuffix(LegacyControlsWiiFamily family)
{
    if (family == LegacyControlsWiiFamily::GameCube)
        return "GC";
    if (family == LegacyControlsWiiFamily::Wiimote)
        return "Wiimote";
    return uiText("Classic");
}
#endif
}

std::vector<LegacyControlsBindingRow> legacyControlsRows(GameSettings *settings)
{
    std::vector<LegacyControlsBindingRow> rows;
    if (settings == nullptr)
        return rows;

#if PLATFORM_WII
    const LegacyControlsWiiFamily families[] = {
        LegacyControlsWiiFamily::GameCube,
        LegacyControlsWiiFamily::Wiimote,
        LegacyControlsWiiFamily::Classic
    };
    rows.reserve(ACTION_COUNT * 3);
    for (int_t action = 0; action < ACTION_COUNT; ++action)
    {
        for (LegacyControlsWiiFamily family : families)
        {
            LegacyControlsBindingRow row;
            row.kind = LegacyControlsBindingKind::WiiFamilyBinding;
            row.label = std::string(actionLabel(action)) + " (" + familySuffix(family) + ")";
            row.action = action;
            row.wiiFamily = family;
            row.wiiField = fieldFor(settings, action, family);
            if (row.wiiField != nullptr)
                rows.push_back(row);
        }
    }
#else
    rows.reserve(settings->keyBindings.size());
    for (int_t i = 0; i < static_cast<int_t>(settings->keyBindings.size()); ++i)
    {
        LegacyControlsBindingRow row;
        row.kind = LegacyControlsBindingKind::KeyBinding;
        row.label = settings->getKeyBindingDescription(i);
        row.bindingIndex = i;
        rows.push_back(row);
    }
#endif
    return rows;
}

bool legacyControlsBindingConflicts(GameSettings *settings, const LegacyControlsBindingRow &row)
{
    if (settings == nullptr)
        return false;

    if (row.kind == LegacyControlsBindingKind::KeyBinding)
    {
        if (row.bindingIndex < 0 || row.bindingIndex >= static_cast<int_t>(settings->keyBindings.size()))
            return false;
        const int_t code = settings->keyBindings[row.bindingIndex]->keyCode;
        for (int_t i = 0; i < static_cast<int_t>(settings->keyBindings.size()); ++i)
        {
            if (i != row.bindingIndex && settings->keyBindings[i]->keyCode == code)
                return true;
        }
        return false;
    }

#if PLATFORM_WII
    if (row.wiiField == nullptr)
        return false;
    const std::vector<LegacyControlsBindingRow> rows = legacyControlsRows(settings);
    for (const LegacyControlsBindingRow &other : rows)
    {
        if (&other == &row || other.kind != LegacyControlsBindingKind::WiiFamilyBinding)
            continue;
        if (other.action == row.action && other.wiiFamily == row.wiiFamily)
            continue;
        if (other.wiiFamily == row.wiiFamily && other.wiiField != nullptr && *other.wiiField == *row.wiiField)
            return true;
    }
#endif
    return false;
}

std::string legacyControlsBindingLabel(GameSettings *settings, const LegacyControlsBindingRow &row)
{
    if (settings == nullptr)
        return "?";

    if (row.kind == LegacyControlsBindingKind::KeyBinding)
    {
        if (row.bindingIndex < 0 || row.bindingIndex >= static_cast<int_t>(settings->keyBindings.size()))
            return "?";
        return settings->getOptionDisplayString(row.bindingIndex);
    }

#if PLATFORM_WII
    if (row.wiiField == nullptr)
        return "?";
    const char *name = wiiPadButtonName(toWiiPadFamily(row.wiiFamily), static_cast<unsigned>(*row.wiiField));
    return name != nullptr ? name : "?";
#else
    return "?";
#endif
}

bool legacyControlsApplyCapturedKey(GameSettings *settings, const LegacyControlsBindingRow &row, int_t key)
{
    if (settings == nullptr)
        return false;

    if (row.kind == LegacyControlsBindingKind::KeyBinding)
    {
        if (row.bindingIndex < 0 || row.bindingIndex >= static_cast<int_t>(settings->keyBindings.size()))
            return false;
        settings->setKeyBinding(row.bindingIndex, key);
        return true;
    }

#if PLATFORM_WII
    const WiiPadFamily *family = wiiPadKeyFamily(key);
    if (family == nullptr || row.wiiField == nullptr)
        return false;
    if (*family != toWiiPadFamily(row.wiiFamily))
        return false;
    const unsigned mask = wiiPadKeyButtonMask(key);
    if (mask == 0)
        return false;
    *row.wiiField = static_cast<int_t>(mask);
    settings->syncControllerBindingsToPlatform();
    settings->saveOptions();
    return true;
#else
    (void)key;
    return false;
#endif
}
