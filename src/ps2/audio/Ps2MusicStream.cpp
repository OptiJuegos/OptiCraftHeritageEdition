#ifdef PS2_PLATFORM

#include "ps2/audio/Ps2MusicStream.h"

#include "ps2/audio/Ps2AdpcmStreamDecoder.h"
#include "ps2/audio/Ps2StreamFile.h"
#include "ps2/system/Ps2ThreadPriority.h"
#include "platform/Log.h"
#include "platform/audio/AudioAssetFormat.h"

#include <audsrv.h>
#include <delaythread.h>
#include <kernel.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdint>
#include <cstring>

namespace
{
constexpr int kBlocksPerChunk = 64;
constexpr int kMaxFramesPerChunk = kBlocksPerChunk * Ps2AdpcmStream::kSamplesPerBlock;
constexpr int kMaxPcmSamples = kMaxFramesPerChunk * 2;
constexpr int kAudsrvWriteBytes = 2048;
// Sleep between ring-buffer polls. The ring frees kAudsrvWriteBytes every
// ~46 ms at 22 kHz mono and every ~11 ms at 48 kHz stereo.
constexpr int kRingPollDelayUs = 4000;
constexpr int kRawPcmSampleRate = 22050;
constexpr int kRawPcmChannels = 1;
constexpr int kThreadPriority = Ps2ThreadPriority::kMusicStream;
constexpr int kThreadStackBytes = 32 * 1024;

struct WavStreamInfo
{
    int channels = 0;
    int sampleRate = 0;
    long dataOffset = 0;
    std::uint32_t dataBytes = 0;
};

alignas(16) unsigned char s_threadStack[kThreadStackBytes];
volatile bool s_stopRequested = false;
volatile bool s_active = false;
int s_threadId = -1;
int s_doneSema = -1;
std::string s_path;
float s_volume = 1.0f;
// Static so the read buffers stay off the 32 KB thread stack. Only the
// stream thread touches them while it runs.
Ps2StreamFile s_streamFile;
Ps2StreamFile s_rightStreamFile;

float clampVolume(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

std::uint16_t readLe16(const std::uint8_t *data)
{
    return static_cast<std::uint16_t>(data[0]) |
           (static_cast<std::uint16_t>(data[1]) << 8);
}

std::uint32_t readLe32(const std::uint8_t *data)
{
    return static_cast<std::uint32_t>(data[0]) |
           (static_cast<std::uint32_t>(data[1]) << 8) |
           (static_cast<std::uint32_t>(data[2]) << 16) |
           (static_cast<std::uint32_t>(data[3]) << 24);
}

// Header probes run both on the main thread (validStreamFile, before stop()
// has joined the previous stream) and on the stream thread (musicThread), so
// each side passes the file object it owns: this one for the main thread,
// s_streamFile for the thread.
Ps2StreamFile s_probeFile;

bool readAdpcmHeader(const std::string &path, Ps2AdpcmStream::Header &header, long &fileSize,
                     Ps2StreamFile &file)
{
    if (!file.open(path.c_str()))
        return false;

    std::array<std::uint8_t, Ps2AdpcmStream::kHeaderBytes> bytes{};
    const bool readOk = file.readExact(bytes.data(), static_cast<int>(bytes.size()));
    fileSize = file.size();
    file.close();
    if (!readOk || fileSize < Ps2AdpcmStream::kHeaderBytes)
        return false;

    if (!Ps2AdpcmStream::parseHeader(bytes.data(), bytes.size(), header))
        return false;

    const std::uint64_t blocks =
        (static_cast<std::uint64_t>(header.samplesPerChannel) + Ps2AdpcmStream::kSamplesPerBlock - 1u) /
        Ps2AdpcmStream::kSamplesPerBlock;
    const std::uint64_t required = Ps2AdpcmStream::kHeaderBytes +
        blocks * Ps2AdpcmStream::kBlockBytes * static_cast<std::uint64_t>(header.channels);
    return required <= static_cast<std::uint64_t>(fileSize);
}

bool readWavHeader(const std::string &path, WavStreamInfo &info, Ps2StreamFile &file)
{
    if (!file.open(path.c_str()))
        return false;

    std::array<std::uint8_t, 12> riff{};
    if (!file.readExact(riff.data(), static_cast<int>(riff.size())) ||
        std::memcmp(riff.data(), "RIFF", 4) != 0 ||
        std::memcmp(riff.data() + 8, "WAVE", 4) != 0)
    {
        file.close();
        return false;
    }

    // Chunk walk by absolute offsets: every chunk header is read from
    // `cursor`, so the position is known without a tell().
    long cursor = static_cast<long>(riff.size());
    bool haveFormat = false;
    bool haveData = false;
    while (!haveData)
    {
        std::array<std::uint8_t, 8> chunk{};
        if (!file.seek(cursor) || !file.readExact(chunk.data(), static_cast<int>(chunk.size())))
            break;

        const std::uint32_t chunkSize = readLe32(chunk.data() + 4);
        const long payloadOffset = cursor + static_cast<long>(chunk.size());

        if (std::memcmp(chunk.data(), "fmt ", 4) == 0)
        {
            if (chunkSize < 16)
                break;

            std::array<std::uint8_t, 16> format{};
            if (!file.readExact(format.data(), static_cast<int>(format.size())))
                break;

            const int encoding = static_cast<int>(readLe16(format.data()));
            info.channels = static_cast<int>(readLe16(format.data() + 2));
            info.sampleRate = static_cast<int>(readLe32(format.data() + 4));
            const int blockAlign = static_cast<int>(readLe16(format.data() + 12));
            const int bits = static_cast<int>(readLe16(format.data() + 14));
            haveFormat = encoding == 1 && bits == 16 &&
                (info.channels == 1 || info.channels == 2) &&
                blockAlign == info.channels * static_cast<int>(sizeof(std::int16_t));
            if (!haveFormat)
                break;
        }
        else if (std::memcmp(chunk.data(), "data", 4) == 0)
        {
            info.dataOffset = payloadOffset;
            info.dataBytes = chunkSize;
            haveData = true;
        }

        cursor = payloadOffset + static_cast<long>(chunkSize) +
            static_cast<long>(chunkSize & 1u);
    }

    file.close();
    return haveFormat && haveData && info.sampleRate > 0 && info.dataBytes > 0;
}

bool configurePcmOutput(int sampleRate, int channels)
{
    audsrv_fmt_t format{};
    format.bits = 16;
    format.freq = sampleRate;
    format.channels = channels;

    audsrv_stop_audio();
    const int result = audsrv_set_format(&format);
    if (result != 0)
    {
        MC_LOG_WARN("audio", "unsupported PS2 PCM stream rate=%d channels=%d err=%d %s\n",
                    sampleRate, channels, result, audsrv_get_error_string());
        return false;
    }

    audsrv_set_volume(static_cast<int>(clampVolume(s_volume) * MAX_VOLUME + 0.5f));
    return true;
}

bool feedBytes(const void *data, int byteCount)
{
    const unsigned char *bytes = static_cast<const unsigned char *>(data);
    int remaining = byteCount;
    while (remaining > 0 && !s_stopRequested)
    {
        // Poll instead of audsrv_wait_audio(). That call blocks inside the
        // audsrv RPC while holding the EE library's single completion
        // semaphore, and the IOP handler is single-threaded, so every SFX
        // call from the main thread stalls behind it for up to one piece.
        const int piece = std::min(remaining, kAudsrvWriteBytes);
        const int available = audsrv_available();
        if (available < 0)
            return false;
        if (available < piece)
        {
            DelayThread(kRingPollDelayUs);
            continue;
        }

        const int sent = audsrv_play_audio(
            reinterpret_cast<const char *>(bytes), std::min(remaining, available));
        if (sent < 0)
            return false;
        if (sent == 0)
        {
            DelayThread(kRingPollDelayUs);
            continue;
        }

        bytes += sent;
        remaining -= sent;
    }
    return !s_stopRequested;
}

bool feedPcm(const std::int16_t *samples, int sampleCount)
{
    return feedBytes(samples, sampleCount * static_cast<int>(sizeof(std::int16_t)));
}

void waitForQueuedAudio(int sampleRate, int channels);

bool streamRawPcm()
{
    Ps2StreamFile &file = s_streamFile;
    if (!file.open(s_path.c_str()))
        return false;

    if (!configurePcmOutput(kRawPcmSampleRate, kRawPcmChannels))
    {
        file.close();
        return false;
    }

    MC_LOG_DEBUG("audio", "PS2 raw PCM stream start %s rate=%d channels=%d\n",
                 s_path.c_str(), kRawPcmSampleRate, kRawPcmChannels);

    bool ok = true;
    while (!s_stopRequested)
    {
        const int got = file.fill();
        if (got < 0)
        {
            ok = false;
            break;
        }
        if (got == 0)
            break;
        if (!feedBytes(file.data(), got))
        {
            ok = false;
            break;
        }
        file.consume(got);
    }

    file.close();
    if (ok && !s_stopRequested)
        waitForQueuedAudio(kRawPcmSampleRate, kRawPcmChannels);
    return ok;
}

bool readBlock(Ps2StreamFile &file, std::uint8_t *block)
{
    return file.isOpen() && file.readExact(block, Ps2AdpcmStream::kBlockBytes);
}

void waitForQueuedAudio(int sampleRate, int channels)
{
    const int bytesPerSecond = sampleRate * channels * static_cast<int>(sizeof(std::int16_t));
    if (bytesPerSecond <= 0 || s_stopRequested)
        return;

    const int queued = audsrv_queued();
    if (queued <= 0)
        return;

    long delayUs = static_cast<long>((static_cast<std::int64_t>(queued) * 1000000LL) / bytesPerSecond);
    delayUs += 20000;
    while (delayUs > 0 && !s_stopRequested)
    {
        const int step = static_cast<int>(std::min<long>(delayUs, 10000));
        DelayThread(step);
        delayUs -= step;
    }
}

bool streamWav(const WavStreamInfo &info)
{
    Ps2StreamFile &file = s_streamFile;
    if (!file.open(s_path.c_str()))
        return false;
    if (!file.seek(info.dataOffset))
    {
        file.close();
        return false;
    }
    if (!configurePcmOutput(info.sampleRate, info.channels))
    {
        file.close();
        return false;
    }

    MC_LOG_DEBUG("audio", "PS2 PCM stream start %s rate=%d channels=%d bytes=%u\n",
                 s_path.c_str(), info.sampleRate, info.channels,
                 static_cast<unsigned int>(info.dataBytes));

    std::uint32_t remaining = info.dataBytes;
    bool ok = true;
    while (!s_stopRequested && remaining > 0)
    {
        const int buffered = file.fill();
        if (buffered <= 0)
        {
            ok = false;
            break;
        }
        const int got = static_cast<int>(std::min<std::uint32_t>(
            remaining, static_cast<std::uint32_t>(buffered)));
        if (!feedBytes(file.data(), got))
        {
            ok = false;
            break;
        }
        file.consume(got);
        remaining -= static_cast<std::uint32_t>(got);
    }

    file.close();
    if (ok && !s_stopRequested)
        waitForQueuedAudio(info.sampleRate, info.channels);
    return ok;
}

bool streamAdpcm(const Ps2AdpcmStream::Header &header)
{
    Ps2StreamFile &left = s_streamFile;
    Ps2StreamFile &right = s_rightStreamFile;
    const bool stereo = header.channels == 2;
    if (!left.open(s_path.c_str()) || (stereo && !right.open(s_path.c_str())))
    {
        left.close();
        right.close();
        return false;
    }

    const std::uint32_t totalBlocks =
        (header.samplesPerChannel + Ps2AdpcmStream::kSamplesPerBlock - 1u) /
        Ps2AdpcmStream::kSamplesPerBlock;
    const long leftOffset = Ps2AdpcmStream::kHeaderBytes;
    const long rightOffset = leftOffset +
        static_cast<long>(totalBlocks) * Ps2AdpcmStream::kBlockBytes;
    if (!left.seek(leftOffset) || (stereo && !right.seek(rightOffset)))
    {
        left.close();
        right.close();
        return false;
    }

    if (!configurePcmOutput(header.sampleRate, header.channels))
    {
        left.close();
        right.close();
        return false;
    }

    Ps2AdpcmStream::ChannelState leftState{};
    Ps2AdpcmStream::ChannelState rightState{};
    std::array<std::int16_t, kMaxPcmSamples> pcm{};
    std::array<std::uint8_t, Ps2AdpcmStream::kBlockBytes> leftBlock{};
    std::array<std::uint8_t, Ps2AdpcmStream::kBlockBytes> rightBlock{};
    std::array<std::int16_t, Ps2AdpcmStream::kSamplesPerBlock> leftPcm{};
    std::array<std::int16_t, Ps2AdpcmStream::kSamplesPerBlock> rightPcm{};

    std::uint32_t blockIndex = 0;
    std::uint32_t emittedFrames = 0;
    bool decodeOk = true;
    while (!s_stopRequested && blockIndex < totalBlocks && emittedFrames < header.samplesPerChannel)
    {
        int chunkFrames = 0;
        for (int chunkBlock = 0;
             chunkBlock < kBlocksPerChunk && blockIndex < totalBlocks &&
             emittedFrames + chunkFrames < header.samplesPerChannel;
             ++chunkBlock, ++blockIndex)
        {
            if (!readBlock(left, leftBlock.data()) ||
                !Ps2AdpcmStream::decodeBlock(leftBlock.data(), leftState, leftPcm.data()))
            {
                decodeOk = false;
                break;
            }
            if (stereo &&
                (!readBlock(right, rightBlock.data()) ||
                 !Ps2AdpcmStream::decodeBlock(rightBlock.data(), rightState, rightPcm.data())))
            {
                decodeOk = false;
                break;
            }

            const std::uint32_t framesRemaining = header.samplesPerChannel - (emittedFrames + chunkFrames);
            const int blockFrames = static_cast<int>(std::min<std::uint32_t>(
                Ps2AdpcmStream::kSamplesPerBlock, framesRemaining));
            for (int frame = 0; frame < blockFrames; ++frame)
            {
                if (header.channels == 2)
                {
                    pcm[static_cast<std::size_t>((chunkFrames + frame) * 2)] = leftPcm[static_cast<std::size_t>(frame)];
                    pcm[static_cast<std::size_t>((chunkFrames + frame) * 2 + 1)] = rightPcm[static_cast<std::size_t>(frame)];
                }
                else
                {
                    pcm[static_cast<std::size_t>(chunkFrames + frame)] = leftPcm[static_cast<std::size_t>(frame)];
                }
            }
            chunkFrames += blockFrames;
        }

        if (!decodeOk || chunkFrames <= 0)
            break;
        if (!feedPcm(pcm.data(), chunkFrames * header.channels))
        {
            decodeOk = false;
            break;
        }
        emittedFrames += static_cast<std::uint32_t>(chunkFrames);
    }

    left.close();
    right.close();
    if (decodeOk && !s_stopRequested)
        waitForQueuedAudio(header.sampleRate, header.channels);
    return decodeOk;
}

void finishThread()
{
    audsrv_stop_audio();
    s_active = false;
    if (s_doneSema >= 0)
        SignalSema(s_doneSema);
    ExitThread();
}

void musicThread(void *)
{
    if (audioPathHasExtension(s_path, ".pcm"))
    {
        if (!streamRawPcm())
            MC_LOG_WARN("audio", "PS2 raw PCM stream failed %s\n", s_path.c_str());
        finishThread();
        return;
    }

    WavStreamInfo wav{};
    if (readWavHeader(s_path, wav, s_streamFile))
    {
        if (!streamWav(wav))
            MC_LOG_WARN("audio", "PS2 PCM stream failed %s\n", s_path.c_str());
        finishThread();
        return;
    }

    Ps2AdpcmStream::Header adpcm{};
    long fileSize = 0;
    if (readAdpcmHeader(s_path, adpcm, fileSize, s_streamFile))
    {
        if (!streamAdpcm(adpcm))
            MC_LOG_WARN("audio", "PS2 ADPCM stream failed %s\n", s_path.c_str());
        finishThread();
        return;
    }

    MC_LOG_WARN("audio", "invalid PS2 music/stream file %s\n", s_path.c_str());
    finishThread();
}

void cleanupFinishedThread()
{
    if (s_threadId < 0)
        return;
    if (s_doneSema >= 0)
        WaitSema(s_doneSema);
    DeleteThread(s_threadId);
    if (s_doneSema >= 0)
        DeleteSema(s_doneSema);
    s_threadId = -1;
    s_doneSema = -1;
}

bool validStreamFile(const std::string &path)
{
    if (audioPathHasExtension(path, ".pcm"))
    {
        Ps2StreamFile &file = s_probeFile;
        if (!file.open(path.c_str()))
            return false;
        const long fileSize = file.size();
        file.close();
        return fileSize > 0 && (fileSize & 1L) == 0;
    }

    WavStreamInfo wav{};
    if (readWavHeader(path, wav, s_probeFile))
        return true;

    Ps2AdpcmStream::Header adpcm{};
    long fileSize = 0;
    return readAdpcmHeader(path, adpcm, fileSize, s_probeFile);
}
}

namespace Ps2MusicStream
{
bool start(const std::string &path, float volume)
{
    if (!validStreamFile(path))
        return false;

    stop();

    ee_sema_t semaphore{};
    semaphore.init_count = 0;
    semaphore.max_count = 1;
    semaphore.option = 0;
    s_doneSema = CreateSema(&semaphore);
    if (s_doneSema < 0)
        return false;

    s_path = path;
    s_volume = clampVolume(volume);
    s_stopRequested = false;
    s_active = true;

    ee_thread_t thread{};
    thread.func = reinterpret_cast<void *>(musicThread);
    thread.stack = s_threadStack;
    thread.stack_size = sizeof(s_threadStack);
    thread.gp_reg = &_gp;
    thread.initial_priority = kThreadPriority;
    s_threadId = CreateThread(&thread);
    if (s_threadId < 0 || StartThread(s_threadId, nullptr) < 0)
    {
        if (s_threadId >= 0)
            DeleteThread(s_threadId);
        DeleteSema(s_doneSema);
        s_threadId = -1;
        s_doneSema = -1;
        s_active = false;
        return false;
    }
    return true;
}

void stop()
{
    if (s_threadId < 0)
    {
        s_active = false;
        return;
    }
    s_stopRequested = true;
    cleanupFinishedThread();
    s_stopRequested = false;
    s_active = false;
}

bool active()
{
    return s_active;
}

void setVolume(float volume)
{
    s_volume = clampVolume(volume);
    if (s_active)
        audsrv_set_volume(static_cast<int>(s_volume * MAX_VOLUME + 0.5f));
}
}

#endif // PS2_PLATFORM
