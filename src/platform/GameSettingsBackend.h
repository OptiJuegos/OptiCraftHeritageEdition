#pragma once

#include <iosfwd>
#include <string>
#include <unordered_set>
#include "java/Type.h"

class GameSettings;

void platformGameSettingsInitialize(GameSettings& settings);
int_t platformGameSettingsDefaultChunkUpdates();
int_t platformGameSettingsDefaultConnectedTextures();
int_t platformGameSettingsCycleRenderDistance(int_t current, int_t delta);
// Applied to a render distance read back from options.txt. The cycle above only
// governs what the options screen can reach; a file written by another build, or
// by this one before its bounds changed, reaches renderDistance without it.
int_t platformGameSettingsClampRenderDistance(int_t value);
int_t platformGameSettingsClampFineRenderDistance(int_t value);
void platformGameSettingsUpdateRenderDistanceFromFine(int_t fineDistance, int_t& renderDistance);
bool platformGameSettingsAnaglyphValue(bool current, bool requested);
bool platformGameSettingsLoadOption(GameSettings& settings, const std::string& key, const std::string& value);
void platformGameSettingsFinalizeLoad(GameSettings& settings);
void platformGameSettingsSyncControllerBindings(const GameSettings& settings);
void platformGameSettingsAddKnownKeys(std::unordered_set<std::string>& keys);
void platformGameSettingsWriteOptions(const GameSettings& settings, std::ostream& out);
