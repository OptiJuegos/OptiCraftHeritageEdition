#include "net/minecraft/src/SoundManager.h"
#include "platform/Log.h"

#if defined(NO_SOUND)


void *SoundManager::sndSystem = nullptr;
bool  SoundManager::loaded    = false;

SoundManager::SoundManager()
    : soundPoolSounds(), soundPoolStreaming(), soundPoolMusic(), soundVolume(0),
      options(nullptr), rand(), ticksBeforeMusic(0)
{
}

void SoundManager::loadSoundSettings(GameSettings *gamesettings) { options = gamesettings; loaded = false; }
void SoundManager::onSoundOptionsChanged() {}
void SoundManager::closeMinecraft() { loaded = false; }
void SoundManager::addSound(const jstring &s, const std::string &file) { soundPoolSounds.addSound(s, file); }
void SoundManager::addStreaming(const jstring &s, const std::string &file) { soundPoolStreaming.addSound(s, file); }
void SoundManager::addMusic(const jstring &s, const std::string &file) { soundPoolMusic.addSound(s, file); }
void SoundManager::playRandomMusicIfReady() {}
void SoundManager::setListenerPosition(EntityLiving *, float) {}
void SoundManager::playStreaming(const jstring &, float, float, float, float, float) {}
void SoundManager::playSound(const jstring &, float, float, float, float, float) {}
void SoundManager::playSoundFX(const jstring &, float, float) {}
void SoundManager::tryToSetLibraryAndCodecs() {}


#else


#include "net/minecraft/src/SoundManager.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <unordered_map>

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <audsrv.h>

#include "platform/Resources.h"
#include "ps2/system/Ps2IrxLoader.h"

#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/EntityLiving.h"
#include "net/minecraft/src/SoundPoolEntry.h"

namespace
{
constexpr int PS2_MAX_CACHED_ADPCM_BYTES = 1024 * 1024;
constexpr int PS2_ADPCM_HEADER_BYTES = 16;
constexpr int PS2_ADPCM_BLOCK_BYTES = 16;
constexpr int PS2_ADPCM_FIRST_VOICE = 1;
constexpr int PS2_ADPCM_VOICE_COUNT = 24;

struct Ps2CachedSample
{
    audsrv_adpcm_t adpcm{};
    int size = 0;
    bool ready = false;
};

struct Ps2VoiceState
{
    Ps2CachedSample *sample = nullptr;
    bool active = false;
};

std::unordered_map<std::string, Ps2CachedSample> s_ps2Samples;
int s_ps2MusicChannel = -1;
Ps2CachedSample *s_ps2MusicSample = nullptr;
float s_ps2SoundVolume = 1.0f;
float s_ps2MusicVolume = 1.0f;
Ps2VoiceState s_ps2Voices[PS2_ADPCM_VOICE_COUNT] = {};
int s_ps2NextVoice = PS2_ADPCM_FIRST_VOICE;

static bool ps2EndsWith(const std::string &s, const char *suffix)
{
    std::size_t n = std::strlen(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

// audsrv ADPCM voice volume uses SPU2 units in the 0..MAX_VOLUME range.
static int ps2Volume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    return (int)(volume * (float)MAX_VOLUME + 0.5f);
}


static void ps2ResetVoiceTracking()
{
    for (int channel = 0; channel < PS2_ADPCM_VOICE_COUNT; ++channel)
        s_ps2Voices[channel] = Ps2VoiceState{};
    s_ps2NextVoice = PS2_ADPCM_FIRST_VOICE;
}

static int ps2FindFreeVoice()
{
    const int usableVoices = PS2_ADPCM_VOICE_COUNT - PS2_ADPCM_FIRST_VOICE;
    for (int offset = 0; offset < usableVoices; ++offset)
    {
        const int channel = PS2_ADPCM_FIRST_VOICE +
            ((s_ps2NextVoice - PS2_ADPCM_FIRST_VOICE + offset) % usableVoices);
        Ps2VoiceState &voice = s_ps2Voices[channel];
        if (!voice.active || voice.sample == nullptr ||
            audsrv_is_adpcm_playing(channel, &voice.sample->adpcm) != 1)
        {
            voice = Ps2VoiceState{};
            s_ps2NextVoice = channel + 1;
            if (s_ps2NextVoice >= PS2_ADPCM_VOICE_COUNT)
                s_ps2NextVoice = PS2_ADPCM_FIRST_VOICE;
            return channel;
        }
    }
    return -1;
}

static bool ps2ValidAdpcmFile(const unsigned char *data, unsigned int size)
{
    if (data == nullptr || size < PS2_ADPCM_HEADER_BYTES + PS2_ADPCM_BLOCK_BYTES)
        return false;
    return ((size - PS2_ADPCM_HEADER_BYTES) % PS2_ADPCM_BLOCK_BYTES) == 0;
}

// Force a silent, self-looping tail on the last two ADPCM blocks so the SPU2
// decoder sets ENDX and stops there instead of decoding past the sample into
// whatever garbage sits next in SPU RAM (heard as a stuck/buzzing beep after
// the sound "ends" -- e.g. after breaking a block). adpenc's own tail isn't
// guaranteed to carry this flag combination.
// SPU2 block flag byte bits: 0x01=End(sets ENDX), 0x02=Repeat(loop instead of
// release), 0x04=Loop Start(loop address = this block). 0x07 on the last
// block means: set ENDX, then loop back onto this same silent block forever.
static void ps2SanitizeOneShotTail(unsigned char *data, unsigned int size)
{
    const unsigned int blocks = (size - PS2_ADPCM_HEADER_BYTES) / PS2_ADPCM_BLOCK_BYTES;
    if (blocks == 0)
        return;

    if (blocks >= 2)
    {
        unsigned char *penult = data + PS2_ADPCM_HEADER_BYTES + (blocks - 2) * PS2_ADPCM_BLOCK_BYTES;
        std::memset(penult + 2, 0, 14);
        penult[1] = 0x00;
    }

    unsigned char *last = data + PS2_ADPCM_HEADER_BYTES + (blocks - 1) * PS2_ADPCM_BLOCK_BYTES;
    last[0] = 0;
    std::memset(last + 2, 0, 14);
    last[1] = 0x07; // End + Repeat + Loop Start -> silent self-loop, ENDX set
}

static Ps2CachedSample *ps2GetAdpcmSample(const std::string &path, bool allowLarge)
{
    auto it = s_ps2Samples.find(path);
    if (it != s_ps2Samples.end())
        return it->second.ready ? &it->second : nullptr;

    if (!ps2EndsWith(path, ".adp"))
    {
        MC_LOG_DEBUG("audio", "unsupported audio asset %s\n", path.c_str());
        return nullptr;
    }

    // The size gate runs off the directory entry, before the read: a rejected
    // sample must not cost a multi-megabyte allocation on a 32 MB console.
    const long fileBytes = PlatformResources::fileSize(path);
    if (fileBytes <= 0)
    {
        MC_LOG_DEBUG("audio", "missing audio %s\n", path.c_str());
        return nullptr;
    }

    if (!allowLarge && fileBytes > PS2_MAX_CACHED_ADPCM_BYTES)
    {
        MC_LOG_DEBUG("audio", "skip large ADPCM %s (%ld bytes)\n", path.c_str(), fileBytes);
        return nullptr;
    }

    unsigned int size = 0;
    unsigned char *data = PlatformResources::loadFile(path, &size);
    if (!ps2ValidAdpcmFile(data, size))
    {
        MC_LOG_WARN("audio", "invalid ADPCM asset %s size=%u\n", path.c_str(), size);
        std::free(data);
        return nullptr;
    }

    ps2SanitizeOneShotTail(data, size);

    // Writeback the EE data cache to main RAM before the SIF DMA to the
    // IOP/SPU2: ps2SanitizeOneShotTail()'s writes land in the EE dcache, and
    // audsrv_load_adpcm() DMAs straight from RAM. Without this flush, the SPU
    // gets stale, unsanitized bytes on real hardware (emulators bypass the
    // cache, so the bug never reproduces there).
    FlushCache(0);

    // audsrv tracks each sample by the address of the audsrv_adpcm_t passed to
    // audsrv_load_adpcm(). We must therefore load into the struct that LIVES in
    // the cache (a stable unordered_map node address) and play from that exact
    // struct — never a copy. Loading into a local and copying it into the map
    // (the previous bug) left audsrv unable to find the sample -> silence.
    Ps2CachedSample &slot = s_ps2Samples[path];
    int r = audsrv_load_adpcm(&slot.adpcm, (void *)data, (int)size);

    // audsrv_load_adpcm() uploads the sample to SPU2 memory, so the EE-side
    // bytes are dead the moment it returns -- this is the whole reason the
    // audio no longer lives in the ELF. The struct address stays the sample's
    // identity (see above), not the buffer it was read from.
    std::free(data);

    if (r < 0)
    {
        MC_LOG_DEBUG("audio", "audsrv_load_adpcm failed %s size=%u err=%d %s\n",
                    path.c_str(), size, r, audsrv_get_error_string());
        s_ps2Samples.erase(path);
        return nullptr;
    }

    slot.size = (int)size;
    slot.ready = true;
    return &slot;
}

static int ps2PlaySample(Ps2CachedSample *sample, float volume)
{
    if (sample == nullptr)
        return -1;

    const int channel = ps2FindFreeVoice();
    if (channel < PS2_ADPCM_FIRST_VOICE)
        return -1;

    const int playedChannel = audsrv_ch_play_adpcm(channel, &sample->adpcm);
    if (playedChannel != channel)
    {
        s_ps2Voices[channel] = Ps2VoiceState{};
        return -1;
    }

    // Set volume/pan only after the voice is confirmed playing. Setting it
    // beforehand had the requested volume silently lost/overwritten by KON
    // (audsrv_ch_play_adpcm()'s internal key-on), so the voice played back at
    // whatever volume that channel last had -- heard as sounds randomly too
    // loud or too quiet. This matches both the official ps2sdk playadpcm
    // sample and the PE-edition reference this file was ported from.
    audsrv_adpcm_set_volume_and_pan(playedChannel, ps2Volume(volume), 0);

    s_ps2Voices[channel].sample = sample;
    s_ps2Voices[channel].active = true;
    return channel;
}

static int ps2PlayAdpcm(const std::string &path, float volume, bool allowLarge)
{
    return ps2PlaySample(ps2GetAdpcmSample(path, allowLarge), volume);
}
}

void *SoundManager::sndSystem = nullptr;
bool SoundManager::loaded = false;

SoundManager::SoundManager()
    : soundPoolSounds(), soundPoolStreaming(), soundPoolMusic(), soundVolume(0),
      options(nullptr), rand(), ticksBeforeMusic(rand.nextInt(12000))
{
}

void SoundManager::loadSoundSettings(GameSettings *gamesettings)
{
    options = gamesettings;
    s_ps2SoundVolume = gamesettings ? gamesettings->soundVolume : 1.0f;
    s_ps2MusicVolume = gamesettings ? gamesettings->musicVolume : 1.0f;

    if (!loaded && (gamesettings == nullptr || gamesettings->soundVolume != 0.0f || gamesettings->musicVolume != 0.0f))
        tryToSetLibraryAndCodecs();
}

void SoundManager::tryToSetLibraryAndCodecs()
{
    if (loaded)
        return;

    sceSifInitRpc(0);
    SifLoadFileInit();

    SifLoadModule("rom0:LIBSD", 0, NULL);
    Ps2IrxLoader::load("irx/audsrv.irx", "host:audsrv.irx");

    int r = audsrv_init();
    if (r != 0)
    {
        MC_LOG_DEBUG("audio", "audsrv_init failed err=%d %s\n", r, audsrv_get_error_string());
        return;
    }

    r = audsrv_adpcm_init();
    if (r != 0)
    {
        MC_LOG_DEBUG("audio", "audsrv_adpcm_init failed err=%d %s\n", r, audsrv_get_error_string());
        audsrv_quit();
        return;
    }

    ps2ResetVoiceTracking();
    audsrv_set_volume(MAX_VOLUME);
    loaded = true;
    MC_LOG_DEBUG("audio", "audsrv ready\n");
}

void SoundManager::onSoundOptionsChanged()
{
    if (options)
    {
        s_ps2SoundVolume = options->soundVolume;
        s_ps2MusicVolume = options->musicVolume;
    }

    if (!loaded && options && (options->soundVolume != 0.0f || options->musicVolume != 0.0f))
        tryToSetLibraryAndCodecs();
}

void SoundManager::closeMinecraft()
{
    if (loaded)
    {
        audsrv_adpcm_init();
        audsrv_quit();
    }

    ps2ResetVoiceTracking();
    s_ps2Samples.clear();
    s_ps2MusicChannel = -1;
    s_ps2MusicSample = nullptr;
    loaded = false;
}

void SoundManager::addSound(const jstring &s, const std::string &file)
{
    soundPoolSounds.addSound(s, file);
}

void SoundManager::addStreaming(const jstring &s, const std::string &file)
{
    soundPoolStreaming.addSound(s, file);
}

void SoundManager::addMusic(const jstring &s, const std::string &file)
{
    soundPoolMusic.addSound(s, file);
}

void SoundManager::playRandomMusicIfReady()
{
    if (!loaded || !options || options->musicVolume == 0.0f)
        return;

    if (s_ps2MusicChannel >= 0 && s_ps2MusicSample &&
        audsrv_is_adpcm_playing(s_ps2MusicChannel, &s_ps2MusicSample->adpcm) == 1)
        return;

    s_ps2MusicChannel = -1;
    s_ps2MusicSample = nullptr;

    if (ticksBeforeMusic > 0)
    {
        ticksBeforeMusic--;
        return;
    }

    SoundPoolEntry *entry = soundPoolMusic.getRandomSound();
    ticksBeforeMusic = rand.nextInt(12000) + 12000;
    if (!entry)
        return;

    Ps2CachedSample *sample = ps2GetAdpcmSample(entry->soundUrl, false);
    if (!sample)
        return;

    const int channel = ps2PlaySample(sample, s_ps2MusicVolume);
    if (channel >= PS2_ADPCM_FIRST_VOICE)
    {
        s_ps2MusicChannel = channel;
        s_ps2MusicSample = sample;
    }
}

void SoundManager::setListenerPosition(EntityLiving *, float)
{
}

void SoundManager::playStreaming(const jstring &s, float, float, float, float volume, float)
{
    if (!loaded || !options || options->soundVolume == 0.0f || s.empty())
        return;

    SoundPoolEntry *entry = soundPoolStreaming.getRandomSoundFromSoundPool(s);
    if (entry)
        ps2PlayAdpcm(entry->soundUrl, volume * s_ps2SoundVolume, false);
}

void SoundManager::playSound(const jstring &s, float, float, float, float volume, float)
{
    if (!loaded || !options || options->soundVolume == 0.0f)
        return;

    SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
    if (entry && volume > 0.0f)
        ps2PlayAdpcm(entry->soundUrl, std::min(volume, 1.0f) * s_ps2SoundVolume, false);
}

void SoundManager::playSoundFX(const jstring &s, float volume, float)
{
    if (!loaded || !options || options->soundVolume == 0.0f)
        return;

    SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
    if (entry)
        ps2PlayAdpcm(entry->soundUrl, std::min(volume, 1.0f) * 0.25f * s_ps2SoundVolume, false);
}


#endif
