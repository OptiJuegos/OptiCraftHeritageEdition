#include "platform/GameSettingsBackend.h"

#include <algorithm>
#include <ostream>
#include "ps2/storage/save/Ps2SaveStorage.h"
#include "lwjgl/Keyboard.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/KeyBinding.h"
#include "platform/PlatformTuning.h"
#include "ps2/input/Ps2PadKeyCodes.h"

namespace
{
void migrateKey(KeyBinding* binding, int_t fallback)
{
	if (binding->keyCode < lwjgl::Keyboard::KEY_MAX)
		binding->keyCode = fallback;
}
}

void platformGameSettingsInitialize(GameSettings& settings)
{
	settings.keyBindForward->keyCode = PS2_KEY_DPAD_UP;
	settings.keyBindLeft->keyCode = PS2_KEY_DPAD_LEFT;
	settings.keyBindBack->keyCode = PS2_KEY_DPAD_DOWN;
	settings.keyBindRight->keyCode = PS2_KEY_DPAD_RIGHT;
	settings.keyBindJump->keyCode = PS2_KEY_CROSS;
	settings.keyBindInventory->keyCode = PS2_KEY_SQUARE;
	settings.keyBindDrop->keyCode = PS2_KEY_TRIANGLE;
	settings.keyBindSneak->keyCode = PS2_KEY_L3;
}

void platformGameSettingsResetControlBindings(GameSettings& settings)
{
	settings.keyBindForward->keyCode = PS2_KEY_DPAD_UP;
	settings.keyBindLeft->keyCode = PS2_KEY_DPAD_LEFT;
	settings.keyBindBack->keyCode = PS2_KEY_DPAD_DOWN;
	settings.keyBindRight->keyCode = PS2_KEY_DPAD_RIGHT;
	settings.keyBindJump->keyCode = PS2_KEY_CROSS;
	settings.keyBindInventory->keyCode = PS2_KEY_SQUARE;
	settings.keyBindDrop->keyCode = PS2_KEY_TRIANGLE;
	settings.keyBindSneak->keyCode = PS2_KEY_L3;
}

int_t platformGameSettingsDefaultChunkUpdates() { return (int_t)PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME; }
int_t platformGameSettingsDefaultConnectedTextures() { return 3; }
int_t platformGameSettingsCycleRenderDistance(int_t, int_t) { return PLATFORM_DEFAULT_RENDER_DISTANCE; }
int_t platformGameSettingsClampRenderDistance(int_t) { return PLATFORM_DEFAULT_RENDER_DISTANCE; }
int_t platformGameSettingsClampFineRenderDistance(int_t value)
{
	return value < 32 ? 32 : (value > PLATFORM_VISIBLE_CHUNK_RADIUS * 16 ? PLATFORM_VISIBLE_CHUNK_RADIUS * 16 : value);
}
void platformGameSettingsUpdateRenderDistanceFromFine(int_t, int_t&) {}
bool platformGameSettingsAnaglyphValue(bool, bool requested) { return requested; }
bool platformGameSettingsLoadOption(GameSettings&, const std::string& key, const std::string& value)
{
    if (key != "worldStorage") return false;
    Ps2SaveStorage::setTarget(value == "usb" ? Ps2SaveStorage::Target::MassStorage
                                           : Ps2SaveStorage::Target::MemoryCard);
    return true;
}

void platformGameSettingsFinalizeLoad(GameSettings& settings)
{
	settings.ofChunkUpdates = std::max(settings.ofChunkUpdates, (int_t)PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME);
	migrateKey(settings.keyBindForward, PS2_KEY_DPAD_UP);
	migrateKey(settings.keyBindLeft, PS2_KEY_DPAD_LEFT);
	migrateKey(settings.keyBindBack, PS2_KEY_DPAD_DOWN);
	migrateKey(settings.keyBindRight, PS2_KEY_DPAD_RIGHT);
	migrateKey(settings.keyBindJump, PS2_KEY_CROSS);
	migrateKey(settings.keyBindInventory, PS2_KEY_SQUARE);
	migrateKey(settings.keyBindDrop, PS2_KEY_TRIANGLE);
	migrateKey(settings.keyBindSneak, PS2_KEY_L3);
}

void platformGameSettingsSyncControllerBindings(const GameSettings&) {}
void platformGameSettingsAddKnownKeys(std::unordered_set<std::string>& keys) { keys.insert("worldStorage"); }
void platformGameSettingsWriteOptions(const GameSettings&, std::ostream& out)
{
    out << "worldStorage:" << (Ps2SaveStorage::target() == Ps2SaveStorage::Target::MassStorage ? "usb" : "mc") << "\n";
}
