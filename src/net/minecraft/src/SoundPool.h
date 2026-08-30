#pragma once

#include <vector>
#include <map>
#include <string>
#include "java/Type.h"
#include "java/Random.h"
#include "java/String.h"
#include "SoundPoolEntry.h"

// net.minecraft.src.SoundPool
class SoundPool
{
public:
	SoundPool();
	~SoundPool();

	SoundPoolEntry *addSound(const jstring &s, const std::string &file);
	SoundPoolEntry *getRandomSound();
	SoundPoolEntry *getRandomSoundFromSoundPool(const jstring &s);

	Random rand;
	std::map<jstring, std::vector<SoundPoolEntry*>> nameToSoundPoolEntriesMapping;
	std::vector<SoundPoolEntry*> allSoundPoolEntries;
	int_t numberOfSoundPoolEntries;
	bool motionX; // isRemovableDigitsFromName (true for sounds, false for streaming)
};
