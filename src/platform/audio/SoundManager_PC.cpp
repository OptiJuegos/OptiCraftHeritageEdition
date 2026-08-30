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

#include <cmath>
#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>

#include "SDL.h"

#include "pc/external/stb_vorbis.h"

#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/EntityLiving.h"
#include "net/minecraft/src/MathHelper.h"
#include "net/minecraft/src/SoundPoolEntry.h"

// ─── Internal audio mixer ────────────────────────────────────────────────────

static constexpr int SAMPLE_RATE   = 44100;
static constexpr int CHANNELS      = 2;      // stereo
static constexpr int MAX_SOUNDS    = 32;     // concurrent sound slots

enum class AudioBus
{
    Sound,
    Music,
    Streaming
};

struct AudioChannel
{
    std::vector<float> pcm;    // interleaved L/R samples, normalised [-1,1]
    size_t pos     = 0;
    bool   active  = false;
    bool   looping = false;
    float  volume  = 1.0f;
    float  pitch   = 1.0f;   // resampling not yet implemented; stored for future use
    AudioBus bus   = AudioBus::Sound;
};

static SDL_AudioDeviceID s_device   = 0;
static AudioChannel      s_channels[MAX_SOUNDS];
static std::mutex        s_mutex;
static float             s_soundVolume   = 1.0f;
static float             s_musicVolume   = 1.0f;
static int               s_musicChannel  = -1; // index into s_channels, or -1
static int               s_streamingChannel = -1;

// Decode an OGG file to float PCM (mono → stereo interleaved, 44100 Hz)
static std::vector<float> decodeOgg(const std::string &path, float pitch)
{
    int channels, sampleRate;
    short *pcmShort = nullptr;
    int samples = stb_vorbis_decode_filename(path.c_str(), &channels, &sampleRate, &pcmShort);
    if (samples <= 0 || pcmShort == nullptr)
        return {};

    // Resample + convert to float stereo
    std::vector<float> out;
    out.reserve((size_t)samples * 2);
    for (int i = 0; i < samples; i++)
    {
        float l = pcmShort[i * channels + 0] / 32768.0f;
        float r = (channels > 1) ? pcmShort[i * channels + 1] / 32768.0f : l;
        out.push_back(l);
        out.push_back(r);
    }
    free(pcmShort);
    return out;
}

// Decode a WAV file to float PCM stereo
static std::vector<float> decodeWav(const std::string &path)
{
    SDL_AudioSpec spec;
    Uint8 *buf = nullptr;
    Uint32 len = 0;
    if (!SDL_LoadWAV(path.c_str(), &spec, &buf, &len))
        return {};

    SDL_AudioCVT cvt;
    SDL_BuildAudioCVT(&cvt,
        spec.format, spec.channels, spec.freq,
        AUDIO_F32SYS, CHANNELS, SAMPLE_RATE);

    std::vector<Uint8> cvtBuf(len * (cvt.len_mult ? cvt.len_mult : 1));
    std::memcpy(cvtBuf.data(), buf, len);
    SDL_FreeWAV(buf);

    cvt.buf = cvtBuf.data();
    cvt.len = (int)len;
    SDL_ConvertAudio(&cvt);

    size_t floatSamples = cvt.len_cvt / sizeof(float);
    std::vector<float> out(floatSamples);
    std::memcpy(out.data(), cvtBuf.data(), cvt.len_cvt);
    return out;
}

static std::vector<float> loadSound(const std::string &path, float pitch = 1.0f)
{
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".ogg")
        return decodeOgg(path, pitch);
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".wav")
        return decodeWav(path);
    return {};
}

// Play decoded PCM on the next free channel; returns channel index or -1
static int playPCM(std::vector<float> pcm, float volume, bool looping, AudioBus bus)
{
    if (pcm.empty()) return -1;
    std::lock_guard<std::mutex> lock(s_mutex);
    for (int i = 0; i < MAX_SOUNDS; i++)
    {
        if (!s_channels[i].active)
        {
            s_channels[i].pcm     = std::move(pcm);
            s_channels[i].pos     = 0;
            s_channels[i].active  = true;
            s_channels[i].looping = looping;
            s_channels[i].volume  = volume;
            s_channels[i].bus     = bus;
            return i;
        }
    }
    return -1;
}

static void stopChannel(int idx, AudioBus expectedBus)
{
    if (idx < 0 || idx >= MAX_SOUNDS) return;
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_channels[idx].active || s_channels[idx].bus != expectedBus)
        return;
    s_channels[idx].active = false;
    s_channels[idx].pcm.clear();
    s_channels[idx].pos = 0;
}

// SDL2 audio callback – mixes all active channels into float32 stereo
static void audioCallback(void * /*userdata*/, Uint8 *stream, int len)
{
    float *out   = reinterpret_cast<float *>(stream);
    int   frames = len / (int)(sizeof(float) * CHANNELS);

    std::memset(stream, 0, len);

    std::lock_guard<std::mutex> lock(s_mutex);
    for (auto &ch : s_channels)
    {
        if (!ch.active) continue;
        float busVolume = ch.bus == AudioBus::Music ? s_musicVolume : s_soundVolume;
        float vol = ch.volume * busVolume;

        for (int f = 0; f < frames; f++)
        {
            if (ch.pos + 1 >= ch.pcm.size())
            {
                if (ch.looping)
                    ch.pos = 0;
                else
                {
                    ch.active = false;
                    ch.pcm.clear();
                    break;
                }
            }
            out[f * 2 + 0] += ch.pcm[ch.pos++] * vol;
            out[f * 2 + 1] += ch.pcm[ch.pos++] * vol;
        }
    }

    // Soft clip to [-1, 1]
    for (int i = 0; i < frames * CHANNELS; i++)
    {
        if      (out[i] >  1.0f) out[i] =  1.0f;
        else if (out[i] < -1.0f) out[i] = -1.0f;
    }
}

// ─── SoundManager static members ─────────────────────────────────────────────

void *SoundManager::sndSystem = nullptr;
bool  SoundManager::loaded    = false;

// ─── SoundManager implementation ─────────────────────────────────────────────

SoundManager::SoundManager() :
    soundPoolSounds(),
    soundPoolStreaming(),
    soundPoolMusic(),
    soundVolume(0),
    options(nullptr),
    rand(),
    ticksBeforeMusic(rand.nextInt(12000))
{
}

void SoundManager::loadSoundSettings(GameSettings *gamesettings)
{
    soundPoolStreaming.motionX = false;
    options = gamesettings;

    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_soundVolume = gamesettings ? gamesettings->soundVolume : 1.0f;
        s_musicVolume = gamesettings ? gamesettings->musicVolume : 1.0f;
    }

    if (!loaded && (gamesettings == nullptr || gamesettings->soundVolume != 0.0f || gamesettings->musicVolume != 0.0f))
        tryToSetLibraryAndCodecs();
}

void SoundManager::tryToSetLibraryAndCodecs()
{
    if (loaded) return;

    SDL_AudioSpec want{}, have{};
    want.freq     = SAMPLE_RATE;
    want.format   = AUDIO_F32SYS;
    want.channels = CHANNELS;
    want.samples  = 1024;
    want.callback = audioCallback;

    s_device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (s_device == 0)
    {
        MC_LOG_ERROR("audio", "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return;
    }
    SDL_PauseAudioDevice(s_device, 0); // start playback
    loaded = true;
}

void SoundManager::onSoundOptionsChanged()
{
    if (!loaded && options && (options->soundVolume != 0.0f || options->musicVolume != 0.0f))
        tryToSetLibraryAndCodecs();

    if (!loaded) return;

    float musicVolume = options ? options->musicVolume : 1.0f;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_soundVolume = options ? options->soundVolume : 1.0f;
        s_musicVolume = musicVolume;
    }

    // Match Java: setting Music to zero stops only ambient background music.
    // Records/streaming remain on the Sound bus.
    if (musicVolume == 0.0f && s_musicChannel >= 0)
        stopChannel(s_musicChannel, AudioBus::Music);
}

void SoundManager::closeMinecraft()
{
    if (s_device)
    {
        SDL_PauseAudioDevice(s_device, 1);
        SDL_CloseAudioDevice(s_device);
        s_device = 0;
    }

    {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (auto &ch : s_channels)
        {
            ch.active = false;
            ch.looping = false;
            ch.pos = 0;
            std::vector<float>().swap(ch.pcm);
        }
        s_musicChannel = -1;
        s_streamingChannel = -1;
    }

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

    // Check if music channel is still playing
    bool bgMusicPlaying = false;
    bool streamingPlaying = false;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        bgMusicPlaying = s_musicChannel >= 0 && s_channels[s_musicChannel].active
            && s_channels[s_musicChannel].bus == AudioBus::Music;
        streamingPlaying = s_streamingChannel >= 0 && s_channels[s_streamingChannel].active
            && s_channels[s_streamingChannel].bus == AudioBus::Streaming;
    }

    if (!bgMusicPlaying && !streamingPlaying)
    {
        if (ticksBeforeMusic > 0)
        {
            ticksBeforeMusic--;
            return;
        }
        SoundPoolEntry *entry = soundPoolMusic.getRandomSound();
        if (entry != nullptr)
        {
            ticksBeforeMusic = rand.nextInt(12000) + 12000;
            auto pcm = loadSound(entry->soundUrl);
            s_musicChannel = playPCM(std::move(pcm), 1.0f, false, AudioBus::Music);
        }
    }
}

void SoundManager::setListenerPosition(EntityLiving *entityliving, float f)
{
    if (!loaded || !options || options->soundVolume == 0.0f) return;
    if (entityliving == nullptr) return;
    // 3.0 audio position tracking — stored for future attenuation use
}

void SoundManager::playStreaming(const jstring &s, float f, float f1, float f2, float f3, float f4)
{
    if (!loaded || !options || options->soundVolume == 0.0f) return;

    // Java uses a distinct source named "streaming".
    if (s_streamingChannel >= 0)
        stopChannel(s_streamingChannel, AudioBus::Streaming);
    s_streamingChannel = -1;

    if (s.empty()) return;

    SoundPoolEntry *entry = soundPoolStreaming.getRandomSoundFromSoundPool(s);
    if (entry != nullptr && f3 > 0.0f)
    {
        // A record/stream temporarily replaces ambient background music.
        if (s_musicChannel >= 0)
            stopChannel(s_musicChannel, AudioBus::Music);
        s_musicChannel = -1;

        auto pcm = loadSound(entry->soundUrl, f4);
        s_streamingChannel = playPCM(std::move(pcm), 0.5f, false, AudioBus::Streaming);
    }
}

void SoundManager::playSound(const jstring &s, float f, float f1, float f2, float f3, float f4)
{
    if (!loaded || !options || options->soundVolume == 0.0f) return;

    SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
    if (entry != nullptr && f3 > 0.0f)
    {
        float vol = f3 > 1.0f ? 1.0f : f3;
        auto pcm = loadSound(entry->soundUrl, f4);
        playPCM(std::move(pcm), vol, false, AudioBus::Sound);
    }
}

void SoundManager::playSoundFX(const jstring &s, float f, float f1)
{
    if (!loaded || !options || options->soundVolume == 0.0f) return;

    SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
    if (entry != nullptr)
    {
        float vol = (f > 1.0f ? 1.0f : f) * 0.25f;
        auto pcm = loadSound(entry->soundUrl, f1);
        playPCM(std::move(pcm), vol, false, AudioBus::Sound);
    }
}

#endif
