#include "platform/GameSettingsBackend.h"

#include <algorithm>
#include <ostream>
#include <stdexcept>
#include "java/String.h"
#include "net/minecraft/src/GameSettings.h"
#include "platform/PlatformTuning.h"
#include "platform/PlatformUserSettings.h"
#include "wii/input/WiiButtonBindings.h"

namespace
{
int_t parseInt(const std::string& value)
{
	int_t parsed = 0;
	if (!String::tryParseInt(value, parsed))
		throw std::invalid_argument("Invalid Wii controller binding");
	return parsed;
}

void copyDefaults(GameSettings& settings, const WiiButtonBindings::Snapshot& snapshot)
{
	settings.wiiGcJump = snapshot.gameCube.jump; settings.wiiGcSneak = snapshot.gameCube.sneak;
	settings.wiiGcDrop = snapshot.gameCube.drop; settings.wiiGcInventory = snapshot.gameCube.inventory;
	settings.wiiGcAttack = snapshot.gameCube.attack; settings.wiiGcUse = snapshot.gameCube.use; settings.wiiGcThirdPerson = snapshot.gameCube.thirdPerson;
	settings.wiiWmJump = snapshot.wiimote.jump; settings.wiiWmSneak = snapshot.wiimote.sneak;
	settings.wiiWmDrop = snapshot.wiimote.drop; settings.wiiWmInventory = snapshot.wiimote.inventory;
	settings.wiiWmAttack = snapshot.wiimote.attack; settings.wiiWmUse = snapshot.wiimote.use; settings.wiiWmThirdPerson = snapshot.wiimote.thirdPerson;
	settings.wiiCcJump = snapshot.classic.jump; settings.wiiCcSneak = snapshot.classic.sneak;
	settings.wiiCcDrop = snapshot.classic.drop; settings.wiiCcInventory = snapshot.classic.inventory;
	settings.wiiCcAttack = snapshot.classic.attack; settings.wiiCcUse = snapshot.classic.use; settings.wiiCcThirdPerson = snapshot.classic.thirdPerson;
}
}

void platformGameSettingsInitialize(GameSettings& settings)
{
	copyDefaults(settings, WiiButtonBindings::get());
}

int_t platformGameSettingsDefaultChunkUpdates() { return (int_t)PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME; }
int_t platformGameSettingsDefaultConnectedTextures() { return 2; }
// SHORT and TINY only. FAR was already out because nothing on this heap can hold
// its grid; NORMAL is out because PLATFORM_CHUNK_CACHE_RADIUS is sized for SHORT
// (see WiiWorldTuning.h) and a wider grid would mesh renderers the chunk cache
// cannot serve, which draws as black terrain holes rather than as distance.
int_t platformGameSettingsCycleRenderDistance(int_t current, int_t delta)
{
	const int_t span = 3 - (int_t)PLATFORM_DEFAULT_RENDER_DISTANCE + 1;
	current += delta;
	while (current > 3) current -= span;
	while (current < (int_t)PLATFORM_DEFAULT_RENDER_DISTANCE) current += span;
	return current;
}
int_t platformGameSettingsClampRenderDistance(int_t value)
{
	if (value < (int_t)PLATFORM_DEFAULT_RENDER_DISTANCE) return (int_t)PLATFORM_DEFAULT_RENDER_DISTANCE;
	return value > 3 ? 3 : value;
}
int_t platformGameSettingsClampFineRenderDistance(int_t value)
{
	return value < 32 ? 32 : (value > PLATFORM_VISIBLE_CHUNK_RADIUS * 16 ? PLATFORM_VISIBLE_CHUNK_RADIUS * 16 : value);
}
void platformGameSettingsUpdateRenderDistanceFromFine(int_t, int_t&) {}
bool platformGameSettingsAnaglyphValue(bool, bool) { return false; }

bool platformGameSettingsLoadOption(GameSettings& settings, const std::string& key, const std::string& value)
{
	int_t* destination = nullptr;
	if (key == "wiiGcJump") destination = &settings.wiiGcJump;
	else if (key == "wiiGcSneak") destination = &settings.wiiGcSneak;
	else if (key == "wiiGcDrop") destination = &settings.wiiGcDrop;
	else if (key == "wiiGcInventory") destination = &settings.wiiGcInventory;
	else if (key == "wiiGcAttack") destination = &settings.wiiGcAttack;
	else if (key == "wiiGcUse") destination = &settings.wiiGcUse;
	else if (key == "wiiGcThirdPerson") destination = &settings.wiiGcThirdPerson;
	else if (key == "wiiWmJump") destination = &settings.wiiWmJump;
	else if (key == "wiiWmSneak") destination = &settings.wiiWmSneak;
	else if (key == "wiiWmDrop") destination = &settings.wiiWmDrop;
	else if (key == "wiiWmInventory") destination = &settings.wiiWmInventory;
	else if (key == "wiiWmAttack") destination = &settings.wiiWmAttack;
	else if (key == "wiiWmUse") destination = &settings.wiiWmUse;
	else if (key == "wiiWmThirdPerson") destination = &settings.wiiWmThirdPerson;
	else if (key == "wiiCcJump") destination = &settings.wiiCcJump;
	else if (key == "wiiCcSneak") destination = &settings.wiiCcSneak;
	else if (key == "wiiCcDrop") destination = &settings.wiiCcDrop;
	else if (key == "wiiCcInventory") destination = &settings.wiiCcInventory;
	else if (key == "wiiCcAttack") destination = &settings.wiiCcAttack;
	else if (key == "wiiCcUse") destination = &settings.wiiCcUse;
	else if (key == "wiiCcThirdPerson") destination = &settings.wiiCcThirdPerson;
	if (destination == nullptr)
		return false;
	*destination = parseInt(value);
	return true;
}

void platformGameSettingsFinalizeLoad(GameSettings& settings)
{
	if (settings.renderDistance < 1 || settings.renderDistance > 3)
		settings.renderDistance = PLATFORM_DEFAULT_RENDER_DISTANCE;
	settings.ofChunkUpdates = std::max(settings.ofChunkUpdates, (int_t)PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME);
	PlatformUserSettings::setAlternativeControls(settings.alternativeControllerLayout);
}

void platformGameSettingsSyncControllerBindings(const GameSettings& settings)
{
	WiiButtonBindings::Snapshot snapshot;
	snapshot.gameCube = { (unsigned)settings.wiiGcJump, (unsigned)settings.wiiGcSneak, (unsigned)settings.wiiGcDrop, (unsigned)settings.wiiGcInventory,
	                      (unsigned)settings.wiiGcAttack, (unsigned)settings.wiiGcUse, (unsigned)settings.wiiGcThirdPerson };
	snapshot.wiimote = { (unsigned)settings.wiiWmJump, (unsigned)settings.wiiWmSneak, (unsigned)settings.wiiWmDrop, (unsigned)settings.wiiWmInventory,
	                   (unsigned)settings.wiiWmAttack, (unsigned)settings.wiiWmUse, (unsigned)settings.wiiWmThirdPerson };
	snapshot.classic = { (unsigned)settings.wiiCcJump, (unsigned)settings.wiiCcSneak, (unsigned)settings.wiiCcDrop, (unsigned)settings.wiiCcInventory,
	                   (unsigned)settings.wiiCcAttack, (unsigned)settings.wiiCcUse, (unsigned)settings.wiiCcThirdPerson };
	WiiButtonBindings::set(snapshot);
}

void platformGameSettingsAddKnownKeys(std::unordered_set<std::string>& keys)
{
	static const char* names[] = {
		"wiiGcJump", "wiiGcSneak", "wiiGcDrop", "wiiGcInventory", "wiiGcAttack", "wiiGcUse", "wiiGcThirdPerson",
		"wiiWmJump", "wiiWmSneak", "wiiWmDrop", "wiiWmInventory", "wiiWmAttack", "wiiWmUse", "wiiWmThirdPerson",
		"wiiCcJump", "wiiCcSneak", "wiiCcDrop", "wiiCcInventory", "wiiCcAttack", "wiiCcUse", "wiiCcThirdPerson"
	};
	for (const char* name : names)
		keys.insert(name);
}

void platformGameSettingsWriteOptions(const GameSettings& settings, std::ostream& out)
{
	out << "wiiGcJump:" << settings.wiiGcJump << "\n";
	out << "wiiGcSneak:" << settings.wiiGcSneak << "\n";
	out << "wiiGcDrop:" << settings.wiiGcDrop << "\n";
	out << "wiiGcInventory:" << settings.wiiGcInventory << "\n";
	out << "wiiGcAttack:" << settings.wiiGcAttack << "\n";
	out << "wiiGcUse:" << settings.wiiGcUse << "\n";
	out << "wiiGcThirdPerson:" << settings.wiiGcThirdPerson << "\n";
	out << "wiiWmJump:" << settings.wiiWmJump << "\n";
	out << "wiiWmSneak:" << settings.wiiWmSneak << "\n";
	out << "wiiWmDrop:" << settings.wiiWmDrop << "\n";
	out << "wiiWmInventory:" << settings.wiiWmInventory << "\n";
	out << "wiiWmAttack:" << settings.wiiWmAttack << "\n";
	out << "wiiWmUse:" << settings.wiiWmUse << "\n";
	out << "wiiWmThirdPerson:" << settings.wiiWmThirdPerson << "\n";
	out << "wiiCcJump:" << settings.wiiCcJump << "\n";
	out << "wiiCcSneak:" << settings.wiiCcSneak << "\n";
	out << "wiiCcDrop:" << settings.wiiCcDrop << "\n";
	out << "wiiCcInventory:" << settings.wiiCcInventory << "\n";
	out << "wiiCcAttack:" << settings.wiiCcAttack << "\n";
	out << "wiiCcUse:" << settings.wiiCcUse << "\n";
	out << "wiiCcThirdPerson:" << settings.wiiCcThirdPerson << "\n";
}
