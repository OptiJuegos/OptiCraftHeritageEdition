#include "platform/GameSettingsBackend.h"

#include "net/minecraft/src/GameSettings.h"

void platformGameSettingsInitialize(GameSettings&) {}
int_t platformGameSettingsDefaultChunkUpdates() { return 1; }
int_t platformGameSettingsDefaultConnectedTextures() { return 2; }
int_t platformGameSettingsCycleRenderDistance(int_t current, int_t delta) { return (current + delta) & 3; }
int_t platformGameSettingsClampRenderDistance(int_t value) { return value; }
int_t platformGameSettingsClampFineRenderDistance(int_t value) { return value; }
void platformGameSettingsUpdateRenderDistanceFromFine(int_t fineDistance, int_t& renderDistance)
{
	renderDistance = 3;
	if (fineDistance > 32) renderDistance = 2;
	if (fineDistance > 64) renderDistance = 1;
	if (fineDistance > 128) renderDistance = 0;
}
bool platformGameSettingsAnaglyphValue(bool, bool requested) { return requested; }
bool platformGameSettingsLoadOption(GameSettings&, const std::string&, const std::string&) { return false; }
void platformGameSettingsFinalizeLoad(GameSettings&) {}
void platformGameSettingsSyncControllerBindings(const GameSettings&) {}
void platformGameSettingsAddKnownKeys(std::unordered_set<std::string>&) {}
void platformGameSettingsWriteOptions(const GameSettings&, std::ostream&) {}
