#pragma once

#include <string>
#include "java/Type.h"
#include "java/Random.h"
#include "java/String.h"
#include "SoundPool.h"

class GameSettings;
class EntityLiving;

// net.minecraft.src.SoundManager
class SoundManager
{
public:
	SoundManager();

	void loadSoundSettings(GameSettings *gamesettings);
	void onSoundOptionsChanged();
	void closeMinecraft();
	void addSound(const jstring &s, const std::string &file);
	void addStreaming(const jstring &s, const std::string &file);
	void addMusic(const jstring &s, const std::string &file);
	void playRandomMusicIfReady();
	void setListenerPosition(EntityLiving *entityliving, float f);
	void playStreaming(const jstring &s, float f, float f1, float f2, float f3, float f4);
	void playSound(const jstring &s, float f, float f1, float f2, float f3, float f4);
	void playSoundFX(const jstring &s, float f, float f1);

private:
	void tryToSetLibraryAndCodecs();

	static void *sndSystem; // SDL2_mixer / OpenAL handle, opaque pointer
	SoundPool soundPoolSounds;
	SoundPool soundPoolStreaming;
	SoundPool soundPoolMusic;
	int_t soundVolume; // soundCounter / nextSoundIndex
	GameSettings *options;
	static bool loaded;
	Random rand;
	int_t ticksBeforeMusic;
};
