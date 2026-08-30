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
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <unistd.h>
#include <malloc.h>

#include <asndlib.h>
#include <ogc/cache.h>
#include <ogc/lwp.h>
#include <ogc/lwp_watchdog.h>

#include "pc/external/stb_vorbis.h"

#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/EntityLiving.h"
#include "net/minecraft/src/SoundPoolEntry.h"
#include "wii/WiiEarlyInit.h"

namespace
{

// ---- One-shot sounds: decoded-PCM cache over voices 1..MAX_SND_VOICES-1 ----
//
// Voice 0 is reserved for music/streaming below. Bounded because the shipped
// sound/+newsound/ set is 3.3 MB of OGG and short-transient Vorbis decodes to
// roughly 4-8x that in 16-bit PCM -- caching all of it at once (15-25 MB)
// would compete directly with world data on a ~60 MB heap. 4 MB holds dozens
// of distinct sounds at once (typical decoded SFX is tens of KB), which is
// enough that the working set of a busy scene should stay resident; least-
// recently-used entries are evicted to make room for the rest. Buffers evicted
// while ASND still reads them remain part of this budget until their voices
// finish. Starting point taken from the same arithmetic WiiTuning.h uses
// elsewhere, not yet measured on hardware.
constexpr size_t WII_SFX_CACHE_BUDGET_BYTES = 4 * 1024 * 1024;

struct WiiSfxSample
{
    void  *pcm        = nullptr;   // memalign(32, ...), 16-bit PCM, native rate
    int    sizeBytes  = 0;
    int    format     = VOICE_MONO_16BIT;
    int    sampleRate = 44100;
    u64    lastUsed   = 0;
};

struct WiiPendingSfxFree
{
    void   *pcm       = nullptr;
    size_t  allocBytes = 0;
};

std::unordered_map<std::string, WiiSfxSample> s_wiiSfxCache;
std::unordered_set<std::string> s_wiiRejectedSfx;
size_t s_wiiSfxCacheBytes = 0;
size_t s_wiiSfxResidentBytes = 0;
std::vector<WiiPendingSfxFree> s_wiiPendingFree;
u64 s_wiiVoiceLastUsed[MAX_SND_VOICES] = {};

size_t wiiAlign32(size_t n) { return (n + 31u) & ~size_t(31u); }

bool wiiSfxBufferIsPlaying(void *ptr)
{
    for (int v = 1; v < MAX_SND_VOICES; v++)
        if (ASND_StatusVoice(v) != SND_UNUSED && ASND_TestPointer(v, ptr))
            return true;
    return false;
}

// A cache entry can be evicted while its voice is still ringing out (a later,
// unrelated sound triggers the eviction). Its buffer can't be freed until no
// voice is still reading it -- ASND_TestPointer() is exactly the primitive
// for that -- so evicted buffers wait here instead of being freed outright.
void wiiReclaimPendingFree()
{
    for (size_t i = 0; i < s_wiiPendingFree.size(); )
    {
        WiiPendingSfxFree &pending = s_wiiPendingFree[i];
        if (wiiSfxBufferIsPlaying(pending.pcm))
        {
            i++;
            continue;
        }
        free(pending.pcm);
        s_wiiSfxResidentBytes -= pending.allocBytes;
        s_wiiPendingFree[i] = s_wiiPendingFree.back();
        s_wiiPendingFree.pop_back();
    }
}

void wiiEvictOneSfx()
{
    if (s_wiiSfxCache.empty())
        return;

    auto oldest = s_wiiSfxCache.begin();
    for (auto it = s_wiiSfxCache.begin(); it != s_wiiSfxCache.end(); ++it)
        if (it->second.lastUsed < oldest->second.lastUsed)
            oldest = it;

    const size_t allocBytes = wiiAlign32((size_t)oldest->second.sizeBytes);
    s_wiiSfxCacheBytes -= allocBytes;
    if (wiiSfxBufferIsPlaying(oldest->second.pcm))
        s_wiiPendingFree.push_back({ oldest->second.pcm, allocBytes });
    else
    {
        free(oldest->second.pcm);
        s_wiiSfxResidentBytes -= allocBytes;
    }
    s_wiiSfxCache.erase(oldest);
}

bool wiiHasSfxRoom(size_t allocBytes)
{
    return s_wiiSfxResidentBytes <= WII_SFX_CACHE_BUDGET_BYTES &&
           allocBytes <= WII_SFX_CACHE_BUDGET_BYTES - s_wiiSfxResidentBytes;
}

bool wiiMakeSfxRoom(size_t allocBytes)
{
    wiiReclaimPendingFree();
    while (!wiiHasSfxRoom(allocBytes) && !s_wiiSfxCache.empty())
    {
        wiiEvictOneSfx();
        wiiReclaimPendingFree();
    }
    return wiiHasSfxRoom(allocBytes);
}

void wiiRememberRejectedSfx(const std::string &path)
{
    try
    {
        s_wiiRejectedSfx.insert(path);
    }
    catch (const std::bad_alloc &)
    {
        MC_LOG_INFO("wii", "[WII][AUDIO] could not memoize rejected SFX path=%s\n",
            path.c_str());
    }
}

void wiiClearSfxCache()
{
    for (int v = 1; v < MAX_SND_VOICES; v++)
        ASND_StopVoice(v);
    for (auto &kv : s_wiiSfxCache)
        free(kv.second.pcm);
    s_wiiSfxCache.clear();
    s_wiiRejectedSfx.clear();
    s_wiiSfxCacheBytes = 0;
    for (const WiiPendingSfxFree &pending : s_wiiPendingFree)
        free(pending.pcm);
    s_wiiPendingFree.clear();
    s_wiiSfxResidentBytes = 0;
}

bool wiiProbeOgg(const std::string &path, size_t &decodedBytes)
{
    int error = 0;
    stb_vorbis *v = stb_vorbis_open_filename(path.c_str(), &error, nullptr);
    if (!v)
        return false;

    stb_vorbis_info info = stb_vorbis_get_info(v);
    unsigned int frames = stb_vorbis_stream_length_in_samples(v);
    stb_vorbis_close(v);

    if (frames == 0 || info.channels <= 0)
        return false;

    const size_t values = (size_t)frames * (size_t)info.channels;
    if (values / (size_t)info.channels != (size_t)frames ||
        values > (size_t)-1 / sizeof(short))
        return false;

    decodedBytes = values * sizeof(short);
    return true;
}

WiiSfxSample *wiiGetSfxSample(const std::string &path)
{
    auto it = s_wiiSfxCache.find(path);
    if (it != s_wiiSfxCache.end())
    {
        it->second.lastUsed = gettime();
        return &it->second;
    }

    if (s_wiiRejectedSfx.find(path) != s_wiiRejectedSfx.end())
        return nullptr;

    size_t probedBytes = 0;
    if (!wiiProbeOgg(path, probedBytes))
    {
        MC_LOG_INFO("wii", "[WII][AUDIO] failed to inspect SFX %s\n", path.c_str());
        return nullptr;
    }
    if (probedBytes > WII_SFX_CACHE_BUDGET_BYTES)
    {
        MC_LOG_INFO("wii", "[WII][AUDIO] skipped oversized SFX decoded=%uKB budget=%uKB path=%s\n",
            (unsigned)(probedBytes / 1024u),
            (unsigned)(WII_SFX_CACHE_BUDGET_BYTES / 1024u), path.c_str());
        wiiRememberRejectedSfx(path);
        return nullptr;
    }

    const size_t probedAllocBytes = wiiAlign32(probedBytes);
    if (!wiiMakeSfxRoom(probedAllocBytes))
    {
        MC_LOG_INFO("wii", "[WII][AUDIO] dropped SFX: cache busy request=%uKB resident=%uKB "
               "cached=%uKB pending=%u path=%s\n",
            (unsigned)(probedAllocBytes / 1024u),
            (unsigned)(s_wiiSfxResidentBytes / 1024u),
            (unsigned)(s_wiiSfxCacheBytes / 1024u),
            (unsigned)s_wiiPendingFree.size(), path.c_str());
        return nullptr;
    }

    short *pcm = nullptr;
    int channels = 0, sampleRate = 0;
    int frames = stb_vorbis_decode_filename(
        path.c_str(), &channels, &sampleRate, &pcm);
    if (frames <= 0 || !pcm || channels <= 0)
    {
        free(pcm);
        MC_LOG_INFO("wii", "[WII][AUDIO] failed to decode SFX %s\n", path.c_str());
        return nullptr;
    }

    const size_t values = (size_t)frames * (size_t)channels;
    if (values / (size_t)channels != (size_t)frames ||
        values > (size_t)-1 / sizeof(short))
    {
        free(pcm);
        MC_LOG_INFO("wii", "[WII][AUDIO] invalid decoded SFX size path=%s\n", path.c_str());
        wiiRememberRejectedSfx(path);
        return nullptr;
    }

    size_t rawBytes = values * sizeof(short);
    if (rawBytes > WII_SFX_CACHE_BUDGET_BYTES)
    {
        free(pcm);
        MC_LOG_INFO("wii", "[WII][AUDIO] skipped oversized SFX after decode=%uKB budget=%uKB path=%s\n",
            (unsigned)(rawBytes / 1024u),
            (unsigned)(WII_SFX_CACHE_BUDGET_BYTES / 1024u), path.c_str());
        wiiRememberRejectedSfx(path);
        return nullptr;
    }
    size_t allocBytes = wiiAlign32(rawBytes);

    if (!wiiMakeSfxRoom(allocBytes))
    {
        free(pcm);
        MC_LOG_INFO("wii", "[WII][AUDIO] dropped decoded SFX: cache busy request=%uKB "
               "resident=%uKB cached=%uKB pending=%u path=%s\n",
            (unsigned)(allocBytes / 1024u),
            (unsigned)(s_wiiSfxResidentBytes / 1024u),
            (unsigned)(s_wiiSfxCacheBytes / 1024u),
            (unsigned)s_wiiPendingFree.size(), path.c_str());
        return nullptr;
    }

    void *buf = memalign(32, allocBytes);
    if (!buf)
    {
        free(pcm);
        MC_LOG_INFO("wii", "[WII][AUDIO] SFX cache allocation failed bytes=%u resident=%u "
               "cached=%u path=%s\n",
            (unsigned)allocBytes, (unsigned)s_wiiSfxResidentBytes,
            (unsigned)s_wiiSfxCacheBytes, path.c_str());
        return nullptr;
    }
    std::memset(buf, 0, allocBytes);
    std::memcpy(buf, pcm, rawBytes);
    free(pcm);
    // ASND/DSP reads through DMA, not the CPU cache. Flush the entire padded
    // allocation once after decoding; cached SFX are immutable afterwards.
    DCFlushRange(buf, allocBytes);

    WiiSfxSample *entry = nullptr;
    try
    {
        auto inserted = s_wiiSfxCache.emplace(path, WiiSfxSample{});
        if (!inserted.second)
        {
            free(buf);
            inserted.first->second.lastUsed = gettime();
            return &inserted.first->second;
        }
        entry = &inserted.first->second;
    }
    catch (const std::bad_alloc &)
    {
        free(buf);
        MC_LOG_INFO("wii", "[WII][AUDIO] SFX cache metadata allocation failed path=%s\n",
            path.c_str());
        return nullptr;
    }
    entry->pcm = buf;
    entry->sizeBytes = (int)rawBytes;
    entry->format = channels >= 2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
    entry->sampleRate = sampleRate;
    entry->lastUsed = gettime();
    s_wiiSfxCacheBytes += allocBytes;
    s_wiiSfxResidentBytes += allocBytes;
    return entry;
}

// First unused voice among 1..15; if all 15 are busy, steal whichever was
// triggered longest ago rather than drop the new sound. Voice 0 is never a
// candidate -- it belongs to music/streaming below.
int wiiPickSfxVoice()
{
    for (int v = 1; v < MAX_SND_VOICES; v++)
        if (ASND_StatusVoice(v) == SND_UNUSED)
            return v;

    int oldest = 1;
    for (int v = 2; v < MAX_SND_VOICES; v++)
        if (s_wiiVoiceLastUsed[v] < s_wiiVoiceLastUsed[oldest])
            oldest = v;
    ASND_StopVoice(oldest);
    return oldest;
}

void wiiPlaySfx(const std::string &path, float volume)
{
    WiiSfxSample *sample = wiiGetSfxSample(path);
    if (!sample)
        return;

    int voice = wiiPickSfxVoice();
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    int vol = (int)(volume * (float)MAX_VOLUME + 0.5f);

    int r = ASND_SetVoice(voice, sample->format, sample->sampleRate, 0,
        sample->pcm, sample->sizeBytes, vol, vol, nullptr);
#if MC_LOG_LEVEL >= 2
    //MC_LOG_INFO("wii", "[WII][AUDIO] sfx '%s' voice=%d fmt=%d rate=%d bytes=%d vol=%d -> %d\n",
        //path.c_str(), voice, sample->format, sample->sampleRate, sample->sizeBytes, vol, r);
#else
    (void)r;
#endif
    s_wiiVoiceLastUsed[voice] = gettime();
}

// ---- Streaming: background music and the jukebox share voice 0 -------------
//
// Music/streaming tracks run up to ~1.4 MB of OGG (several MB decoded to PCM
// for the longest record tracks) -- too large to decode whole and hold
// resident the way the SFX cache does. This double-buffers ~4096-frame
// chunks (roughly 100 ms at 44.1 kHz) through a background LWP thread
// modelled on devkitPro's own oggplayer example
// (examples/wii/audio/oggplayer), adapted from Tremor/ov_read to
// stb_vorbis_get_samples_short_interleaved since that is the decoder already
// vendored in this port. Simplified relative to that example: a 1 ms poll
// instead of an LWP_ThreadSleep/Signal rendezvous -- easier to reason about
// without hardware to test the handshake on, at the cost of the producer
// thread waking slightly more often than strictly necessary.
constexpr int WII_STREAM_FRAMES = 4096;

alignas(32) short s_wiiStreamBuf[2][WII_STREAM_FRAMES * 2];
volatile int  s_wiiStreamFrames[2] = { 0, 0 };
volatile int  s_wiiStreamBytes[2]  = { 0, 0 };
volatile bool s_wiiStreamReady[2]  = { false, false };
stb_vorbis *s_wiiStreamVorbis      = nullptr;
std::string s_wiiStreamPath;
int   s_wiiStreamChannels          = 0;
int   s_wiiStreamSampleRate        = 44100;
float s_wiiStreamVolume            = 1.0f;
volatile bool s_wiiStreamEof       = false;
volatile bool s_wiiStreamRunning   = false;
volatile u32  s_wiiStreamStarves   = 0;
volatile u32  s_wiiStreamAddBusy   = 0;

lwp_t  s_wiiStreamThread = LWP_THREAD_NULL;
// stb_vorbis_open_filename now runs on this worker too; leave headroom for its
// setup path as well as the steady-state decode loop.
u8     s_wiiStreamStack[16384] __attribute__((aligned(8)));

void wiiStreamCallback(s32 voice)
{
    if (!s_wiiStreamRunning)
    {
        ASND_StopVoice(voice);
        return;
    }
    for (int slot = 0; slot < 2; slot++)
    {
        if (s_wiiStreamReady[slot])
        {
            const int result = ASND_AddVoice(voice, s_wiiStreamBuf[slot],
                s_wiiStreamBytes[slot]);
            if (result == SND_OK)
                s_wiiStreamReady[slot] = false;
            else if (result == SND_BUSY)
                ++s_wiiStreamAddBusy;
            return;
        }
    }
    // Producer fell behind (or the track just ended): nothing to hand off.
    // With a callback ASND remains in SND_WAITING and calls us again; the worker
    // can therefore publish the next buffer without restarting the whole OGG.
    if (!s_wiiStreamEof)
        ++s_wiiStreamStarves;
}

void *wiiStreamThreadMain(void *)
{
    int error = 0;
    stb_vorbis *v = stb_vorbis_open_filename(s_wiiStreamPath.c_str(), &error, nullptr);
    if (!v)
    {
        MC_LOG_INFO("wii", "[WII][AUDIO] failed to open stream %s (err=%d)\n",
               s_wiiStreamPath.c_str(), error);
        s_wiiStreamRunning = false;
        return nullptr;
    }

    s_wiiStreamVorbis = v;
    stb_vorbis_info info = stb_vorbis_get_info(v);
    s_wiiStreamChannels = info.channels;
    s_wiiStreamSampleRate = (int)info.sample_rate;
    MC_LOG_INFO("wii", "[WII][AUDIO] streaming %s chans=%d rate=%d vol=%.2f\n",
           s_wiiStreamPath.c_str(), s_wiiStreamChannels,
           s_wiiStreamSampleRate, s_wiiStreamVolume);

    while (s_wiiStreamRunning)
    {
        // ready=false only means the buffer was submitted. ASND owns it until
        // TestPointer says otherwise; this is the ownership test used by the
        // official libogc oggplayer and prevents decoding over live DMA data.
        for (int slot = 0; slot < 2; ++slot)
        {
            if (!s_wiiStreamReady[slot] && s_wiiStreamFrames[slot] > 0
                && ASND_TestPointer(0, s_wiiStreamBuf[slot]) == 0)
            {
                s_wiiStreamFrames[slot] = 0;
                s_wiiStreamBytes[slot] = 0;
            }
        }

        for (int fillSlot = 0; fillSlot < 2 && !s_wiiStreamEof; ++fillSlot)
        {
            if (s_wiiStreamReady[fillSlot] || s_wiiStreamFrames[fillSlot] != 0)
                continue;

            int got = 0;
            while (got < WII_STREAM_FRAMES)
            {
                int n = stb_vorbis_get_samples_short_interleaved(
                    s_wiiStreamVorbis, s_wiiStreamChannels,
                    s_wiiStreamBuf[fillSlot] + (size_t)got * s_wiiStreamChannels,
                    (WII_STREAM_FRAMES - got) * s_wiiStreamChannels);
                if (n <= 0)
                    break;
                got += n;
            }

            if (got == 0)
            {
                s_wiiStreamEof = true;
            }
            else
            {
                const size_t rawBytes = (size_t)got * (size_t)s_wiiStreamChannels * sizeof(short);
                const size_t dmaBytes = wiiAlign32(rawBytes);
                if (dmaBytes > rawBytes)
                    std::memset(reinterpret_cast<byte_t *>(s_wiiStreamBuf[fillSlot]) + rawBytes,
                                0, dmaBytes - rawBytes);
                DCFlushRange(s_wiiStreamBuf[fillSlot], dmaBytes);
                s_wiiStreamFrames[fillSlot] = got;
                s_wiiStreamBytes[fillSlot] = (int)dmaBytes;
                s_wiiStreamReady[fillSlot] = true;
            }
        }

        // Initial start and recovery from an actually stopped voice happen here.
        // Normal starvation leaves a callback voice in SND_WAITING; once a
        // buffer becomes ready the repeatedly invoked callback chains it.
        if (ASND_StatusVoice(0) == SND_UNUSED && s_wiiStreamRunning)
        {
            for (int slot = 0; slot < 2; ++slot)
            {
                if (!s_wiiStreamReady[slot])
                    continue;
                int fmt = s_wiiStreamChannels >= 2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
                int vol = (int)(s_wiiStreamVolume * (float)MAX_VOLUME + 0.5f);
                ASND_SetVoice(0, fmt, s_wiiStreamSampleRate, 0,
                    s_wiiStreamBuf[slot], s_wiiStreamBytes[slot],
                    vol, vol, wiiStreamCallback);
                s_wiiStreamReady[slot] = false;
                break;
            }
        }

        if (s_wiiStreamEof && !s_wiiStreamReady[0] && !s_wiiStreamReady[1]
            && s_wiiStreamFrames[0] == 0 && s_wiiStreamFrames[1] == 0)
        {
            // A callback voice intentionally remains SND_WAITING when starved;
            // at true EOF no buffer will ever arrive, so end it explicitly.
            ASND_StopVoice(0);
            s_wiiStreamRunning = false;
            break;
        }

        usleep(1000);
    }

    if (s_wiiStreamVorbis)
    {
        stb_vorbis_close(s_wiiStreamVorbis);
        s_wiiStreamVorbis = nullptr;
    }
    return nullptr;
}

void wiiStopStream()
{
    if (!s_wiiStreamRunning && s_wiiStreamThread == LWP_THREAD_NULL)
        return;

    s_wiiStreamRunning = false;
    ASND_StopVoice(0);

    if (s_wiiStreamThread != LWP_THREAD_NULL)
    {
        LWP_JoinThread(s_wiiStreamThread, nullptr);
        s_wiiStreamThread = LWP_THREAD_NULL;
    }
    s_wiiStreamReady[0] = false;
    s_wiiStreamReady[1] = false;
    s_wiiStreamFrames[0] = 0;
    s_wiiStreamFrames[1] = 0;
    s_wiiStreamBytes[0] = 0;
    s_wiiStreamBytes[1] = 0;
    s_wiiStreamPath.clear();
}

bool wiiStartStream(const std::string &path, float volume)
{
    wiiStopStream();

    s_wiiStreamPath       = path;
    s_wiiStreamVorbis     = nullptr;
    s_wiiStreamVolume     = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
    s_wiiStreamEof        = false;
    s_wiiStreamReady[0]  = false;
    s_wiiStreamReady[1]  = false;
    s_wiiStreamFrames[0] = 0;
    s_wiiStreamFrames[1] = 0;
    s_wiiStreamBytes[0] = 0;
    s_wiiStreamBytes[1] = 0;
    s_wiiStreamRunning    = true;

    // Match devkitPro's oggplayer priority. The previous 64 could pre-empt the
    // game more aggressively while opening/decoding a track and merely move the
    // visible startup hitch from the main thread to a higher-priority worker.
    if (LWP_CreateThread(&s_wiiStreamThread, wiiStreamThreadMain, nullptr,
            s_wiiStreamStack, sizeof(s_wiiStreamStack), 80) == -1)
    {
        MC_LOG_INFO("wii", "[WII][AUDIO] failed to start stream thread\n");
        s_wiiStreamRunning = false;
        s_wiiStreamPath.clear();
        return false;
    }
    return true;
}

bool wiiStreamIsActive()
{
    // Opening/priming is active too. Requiring an already-running ASND voice
    // made the next game tick stop and reopen a stream whose first buffer was
    // still being decoded.
    return s_wiiStreamRunning;
}

} // namespace

extern "C" void wiiAudioMemoryStats(u32 *cachedBytes, u32 *residentBytes,
                                     u32 *pendingBuffers, u32 *rejectedSounds,
                                     u32 *streamStarves, u32 *streamAddBusy)
{
    if (cachedBytes)
        *cachedBytes = (u32)s_wiiSfxCacheBytes;
    if (residentBytes)
        *residentBytes = (u32)s_wiiSfxResidentBytes;
    if (pendingBuffers)
        *pendingBuffers = (u32)s_wiiPendingFree.size();
    if (rejectedSounds)
        *rejectedSounds = (u32)s_wiiRejectedSfx.size();
    if (streamStarves)
        *streamStarves = s_wiiStreamStarves;
    if (streamAddBusy)
        *streamAddBusy = s_wiiStreamAddBusy;
}

void *SoundManager::sndSystem = nullptr;
bool SoundManager::loaded = false;

SoundManager::SoundManager()
    : soundPoolSounds(), soundPoolStreaming(), soundPoolMusic(), soundVolume(0),
      options(nullptr), rand(), ticksBeforeMusic(rand.nextInt(12000))
{
}

void SoundManager::tryToSetLibraryAndCodecs()
{
    if (loaded)
        return;

    ASND_Init();
    ASND_Pause(0); // ASND_Init() starts paused
    s_wiiPendingFree.reserve(MAX_SND_VOICES - 1);

    loaded = true;
    MC_LOG_INFO("wii", "[WII][AUDIO] ASND ready\n");
}

void SoundManager::loadSoundSettings(GameSettings *gamesettings)
{
    options = gamesettings;

    if (!loaded && (gamesettings == nullptr || gamesettings->soundVolume != 0.0f || gamesettings->musicVolume != 0.0f))
        tryToSetLibraryAndCodecs();
}

void SoundManager::onSoundOptionsChanged()
{
    if (!loaded && options && (options->soundVolume != 0.0f || options->musicVolume != 0.0f))
        tryToSetLibraryAndCodecs();
}

void SoundManager::closeMinecraft()
{
    if (loaded)
    {
        wiiStopStream();
        wiiClearSfxCache();
        ASND_Pause(1);
        ASND_End();
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

    // Shares voice 0 with the jukebox (playStreaming) below, so "is voice 0
    // busy" is the right question regardless of which of the two started it.
    if (wiiStreamIsActive())
        return;

    if (ticksBeforeMusic > 0)
    {
        ticksBeforeMusic--;
        return;
    }

    SoundPoolEntry *entry = soundPoolMusic.getRandomSound();
    ticksBeforeMusic = rand.nextInt(12000) + 12000;
    if (!entry)
    {
#if MC_LOG_LEVEL >= 2
        MC_LOG_INFO("wii", "[WII][AUDIO] playRandomMusicIfReady: music pool is empty\n");
#endif
        return;
    }

    wiiStartStream(entry->soundUrl, options->musicVolume);
}

void SoundManager::setListenerPosition(EntityLiving *, float)
{
}

void SoundManager::playStreaming(const jstring &s, float, float, float, float f3, float)
{
    if (!loaded || !options || options->soundVolume == 0.0f)
        return;

    if (s.empty())
    {
        wiiStopStream();
        return;
    }

    if (f3 <= 0.0f)
        return;

    SoundPoolEntry *entry = soundPoolStreaming.getRandomSoundFromSoundPool(s);
    if (entry)
        wiiStartStream(entry->soundUrl, 0.5f * options->soundVolume);
}

void SoundManager::playSound(const jstring &s, float, float, float, float f3, float)
{
    if (!loaded || !options || options->soundVolume == 0.0f)
        return;

    SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
#if MC_LOG_LEVEL >= 2
    if (!entry)
        MC_LOG_INFO("wii", "[WII][AUDIO] playSound: no entry for '%s'\n", s.c_str());
#endif
    if (entry && f3 > 0.0f)
        wiiPlaySfx(entry->soundUrl, std::min(f3, 1.0f) * options->soundVolume);
}

void SoundManager::playSoundFX(const jstring &s, float volume, float)
{
    if (!loaded || !options || options->soundVolume == 0.0f)
        return;

    SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
#if MC_LOG_LEVEL >= 2
    if (!entry)
        MC_LOG_DEBUG("audio", "playSoundFX: no entry for '%s'", s.c_str());
#endif
    if (entry)
        wiiPlaySfx(entry->soundUrl, std::min(volume, 1.0f) * 0.25f * options->soundVolume);
}

#endif
