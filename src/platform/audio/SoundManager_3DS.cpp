// SoundManager_3DS.cpp -- Nintendo 3DS ndsp backend (Phase 4).
//
// The 3DS consumes the same assets the PS2 backend does -- and that means the
// same three file shapes Ps2MusicStream probes by content, so a pack staged
// for the PS2 plays here unmodified:
//
//   * .adp  SPU2-ADPCM exactly as ps2sdk's adpenc writes it (the "APCM"
//     16-byte header plus 16-byte blocks from scripts/ogg to adp). The 3DS
//     has no SPU2 to decode that in hardware, so the shared portable decoder
//     the PS2 streamer uses (ps2/audio/Ps2AdpcmStreamDecoder.*, listed into
//     this target by cmake/3ds.cmake -- pure C++, no PS2 SDK dependency)
//     turns the blocks into 16-bit PCM.
//   * .pcm  raw s16le with no header (scripts/ogg to pcm): the shape the
//     PS2 pack's music/ and streaming/ folders actually carry. There is no
//     header to read a rate or channel count from, so the stream reads them
//     as 22050 mono -- the PS2's own interpretation (kRawPcmSampleRate /
//     kRawPcmChannels) and the only one the files' durations support (the
//     beta record "13" runs 2:58; 13.pcm at 44100 mono bytes/second lands
//     on exactly that).
//   * .wav  RIFF/WAVE with a real header (scripts/ogg to wav), so the rate
//     and channels come from the fmt chunk instead of a convention.
//
// Decoded 16-bit PCM lives in linear memory (the DSP DMAs from the linear
// heap), queued on ndsp channels:
//
//   * One-shot SFX (.adp only, like the PS2's addSound) decode once and
//     cache, LRU-evicted under a fixed byte budget (the Wii backend's
//     arithmetic: the set decodes to several times its compressed size,
//     and the heap has to keep a world too), behind the same 1 MB
//     per-asset gate the PS2 SFX path applies.
//   * Music/records stream on channel 0, at most one at a time -- the same
//     single-stream semantics the PS2 uses. Blocks (or raw frames) are
//     read and decoded incrementally into a ring of wave buffers
//     (DsStreamFile is the pak-aware sequential reader), so a minutes-long
//     track never has to sit decoded in RAM.
//
// The DSP firmware: ndspInit() loads romfs:/dspfirm.cdc or
// sdmc:/3ds/dspfirm.cdc. Real hardware needs the file; Azahar's HLE
// accepts any readable placeholder.
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
bool SoundManager::playMusicFileNow(const std::string &) { return false; }
void SoundManager::setListenerPosition(EntityLiving *, float) {}
void SoundManager::playStreaming(const jstring &, float, float, float, float, float) {}
void SoundManager::playSound(const jstring &, float, float, float, float, float) {}
void SoundManager::playSoundFX(const jstring &, float, float) {}
void SoundManager::tryToSetLibraryAndCodecs() {}

#else

#include <3ds.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>

#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/EntityLiving.h"
#include "net/minecraft/src/SoundPoolEntry.h"
#include "3ds/audio/DsStreamFile.h"
#include "ps2/audio/Ps2AdpcmStreamDecoder.h"
#include "platform/Resources.h"
#include "platform/audio/AudioAssetFormat.h"
#include "platform/audio/AudioSpatialization.h"

namespace
{

AudioListenerState s_listener;

constexpr int CTR_MUSIC_CHANNEL = 0;
constexpr int CTR_SFX_FIRST_CHANNEL = 1;
constexpr int CTR_SFX_CHANNEL_COUNT = 24;

// Decoded-PCM budget for one-shot SFX, mirroring the Wii backend's
// arithmetic: enough to keep a busy scene's working set resident without
// competing with world data on a 64 MB Old-3DS heap.
constexpr size_t CTR_SFX_CACHE_BUDGET_BYTES = 4 * 1024 * 1024;

// Compressed-ADP gate before a one-shot decode, the same 1 MB ceiling the
// PS2 SFX path applies (PS2_MAX_CACHED_ADPCM_BYTES): the only assets that
// big are the minutes-long ambient beds and loops, which that console never
// loads either -- and which would decode past the whole budget above anyway.
constexpr long CTR_SFX_MAX_FILE_BYTES = 1024 * 1024;

float s_soundVolume = 1.0f;
float s_musicVolume = 1.0f;
bool s_streamIsMusic = false; // false = record/streaming, true = music
bool s_streamActive = false;
int s_nextSfxChannel = CTR_SFX_FIRST_CHANNEL;

struct CachedSample
{
	std::int16_t *pcm = nullptr; // linearAlloc: DSP DMA reads this
	std::uint32_t samples = 0;   // frames per channel
	int channels = 1;
	int rate = 44100;
	std::uint64_t lastUsed = 0;
	std::size_t allocBytes = 0;
	// One wave buffer per ndsp channel: the same sample can be playing on
	// several channels at once (overlapping footsteps are the constant
	// case), and ndspWaveBuf is a live queue node -- re-adding the one
	// struct while it is still queued on another channel corrupts the
	// playlist and the speaker plays the chopped result (the "robotic"
	// report). Each play arms its own channel's buf below.
	ndspWaveBuf wave[CTR_SFX_CHANNEL_COUNT]{};
};

std::unordered_map<std::string, CachedSample> s_sfxCache;
std::unordered_map<std::string, bool> s_rejectedSfx;
std::size_t s_sfxCacheBytes = 0;
std::uint64_t s_cacheClock = 0;

// The one live stream's playback state (channel 0). Music and records are
// minutes of audio -- tens of MB decoded, which does not fit next to a
// loaded world in the 64 MB Old-3DS application region. So the ADP blocks
// are read and decoded incrementally into a ring of wave buffers, the same
// shape the PS2's stream thread takes over its own block reader.
constexpr int CTR_STREAM_BUFFER_COUNT = 6;
constexpr int CTR_STREAM_BUFFER_FRAMES = 8192; // ~186 ms stereo 44.1 kHz, ~371 ms at the ADP set's 22050

// Refills ran in ndsp's own sound-frame callback -- the canonical ndsp
// streaming shape, and immune to the game-thread starvation that a stream
// thread of our own suffered (the original "robotic music" report). But that
// callback fires on the DSP service thread, whose stack is a fraction of what
// a decoder wants: the decode overran it, which was both the corrupted
// "robotic with echo/saturation" audio the reports described and the menu
// exit the 2026-09-25 run died with ~3 s after the music started. So the
// refills now run on a thread of our own with a stack that fits the decode,
// at a priority ABOVE the game thread's own 0x30 -- the starvation the
// first attempt hit came from running it below the game thread -- and it
// sleeps between passes: the ring above holds over a second of audio, so a
// few ms of cadence is far under what one buffer drains in.
constexpr int CTR_STREAM_THREAD_PRIORITY = 0x2B;
constexpr size_t CTR_STREAM_THREAD_STACK_BYTES = 32 * 1024;

// Defined below, after startAudioStream; the stream start needs it first.
void streamRefillThreadMain(void *);

// The stream's source format: which of the PS2's three asset shapes the open
// file carries. `Adp` walks 16-byte blocks through the shared decoder; the
// other two are already 16-bit PCM on disk and a fill is a plain read.
enum class StreamFormat
{
	Adp,
	RawPcm,
	Wav
};

// Parsed from a RIFF/WAVE header, the same fields Ps2MusicStream's
// WavStreamInfo carries (and the same chunk walk the probe below performs).
struct WavStreamInfo
{
	int channels = 0;
	int sampleRate = 0;
	long dataOffset = 0;
	std::uint32_t dataBytes = 0;
};

// The PS2's raw-PCM interpretation: scripts/ogg to pcm writes headerless
// s16le, so the rate and channel count are conventions, not file contents.
// These are the constants Ps2MusicStream plays .pcm files with.
constexpr int CTR_RAW_PCM_SAMPLE_RATE = 22050;
constexpr int CTR_RAW_PCM_CHANNELS = 1;

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

struct StreamState
{
	// Sequential block readers: the left plane starts right after the
	// header, the right channel's plane (stereo only) after the left
	// plane's blocks -- the planar layout streamAdpcm() seeks to on the PS2.
	// For the raw-PCM and WAV formats only `left` is used, one stream of
	// interleaved frames.
	DsStreamFile left;
	DsStreamFile right;
	StreamFormat format = StreamFormat::Adp;
	WavStreamInfo wav{};
	Ps2AdpcmStream::Header header{};
	Ps2AdpcmStream::ChannelState leftState{};
	Ps2AdpcmStream::ChannelState rightState{};
	std::uint32_t framesDecoded = 0; // frames pulled through decodeStreamFrames
	std::uint32_t framesTotal = 0;   // raw/wav: whole frames the source holds
	std::int16_t *pcm[CTR_STREAM_BUFFER_COUNT] = {};
	ndspWaveBuf wave[CTR_STREAM_BUFFER_COUNT]{};
	std::string path;
	float gain = 1.0f;
	bool running = false;
	bool eof = false;
	// libctru's Thread is already a pointer type (Thread_tag*).
	Thread thread = nullptr;
};

StreamState s_stream;

void stopStream();

void setChannelMix(int channel, float gain)
{
	float mix[12] = {};
	mix[0] = gain; // front left
	mix[1] = gain; // front right
	ndspChnSetMix(channel, mix);
}

// ADP asset validation, shared by the SFX and stream paths: the header must
// parse (APCM magic, supported rate/channels) and the file must hold every
// block the header promises -- the same check readAdpcmHeader() performs on
// the PS2 streamer. `data` is the 16-byte header probe, `size` the whole
// file's.
bool validAdpHeader(const std::uint8_t *data, std::size_t size, Ps2AdpcmStream::Header &header)
{
	if (!Ps2AdpcmStream::parseHeader(data, size, header))
		return false;

	const std::uint64_t blocks =
		(static_cast<std::uint64_t>(header.samplesPerChannel) +
		 Ps2AdpcmStream::kSamplesPerBlock - 1u) /
		static_cast<std::uint64_t>(Ps2AdpcmStream::kSamplesPerBlock);
	const std::uint64_t required = Ps2AdpcmStream::kHeaderBytes +
		blocks * Ps2AdpcmStream::kBlockBytes * static_cast<std::uint64_t>(header.channels);
	return required <= static_cast<std::uint64_t>(size);
}

struct DecodedSfx
{
	std::int16_t *pcm = nullptr; // linearAlloc'd, caller takes ownership
	std::uint32_t frames = 0;   // per channel
	std::size_t bytes = 0;
};

// Whole-ADP decode for one-shot SFX. Stereo files are planar (each channel's
// blocks contiguous, left plane first) and come out interleaved, which is
// what ndspWaveBuf wants. The header's loop flag is ignored: a one-shot
// plays exactly samplesPerChannel frames, the software counterpart of the
// silent tail the PS2 writes so the SPU2's hardware decoder stops on ENDX.
bool decodeAdpSfx(const std::uint8_t *data, const Ps2AdpcmStream::Header &header, DecodedSfx &out)
{
	const bool stereo = header.channels == 2;
	const std::uint64_t bytes64 =
		static_cast<std::uint64_t>(header.samplesPerChannel) *
		static_cast<std::uint64_t>(header.channels) * sizeof(std::int16_t);
	// A sample that cannot fit the cache even empty is refused before the
	// allocation: evictFor() would churn the whole cache for it and still
	// land over budget.
	if (bytes64 > CTR_SFX_CACHE_BUDGET_BYTES)
		return false;

	std::int16_t *pcm = static_cast<std::int16_t *>(linearAlloc(static_cast<std::size_t>(bytes64)));
	if (pcm == nullptr)
		return false;

	const std::uint32_t totalBlocks =
		(header.samplesPerChannel + Ps2AdpcmStream::kSamplesPerBlock - 1u) /
		Ps2AdpcmStream::kSamplesPerBlock;
	Ps2AdpcmStream::ChannelState leftState{};
	Ps2AdpcmStream::ChannelState rightState{};
	std::array<std::int16_t, Ps2AdpcmStream::kSamplesPerBlock> leftPcm{};
	std::array<std::int16_t, Ps2AdpcmStream::kSamplesPerBlock> rightPcm{};

	std::uint32_t emitted = 0;
	bool ok = true;
	while (ok && emitted < header.samplesPerChannel)
	{
		const std::uint32_t blockIndex = emitted / Ps2AdpcmStream::kSamplesPerBlock;
		const std::uint8_t *leftBlock = data + Ps2AdpcmStream::kHeaderBytes +
			static_cast<std::size_t>(blockIndex) * Ps2AdpcmStream::kBlockBytes;
		if (!Ps2AdpcmStream::decodeBlock(leftBlock, leftState, leftPcm.data()))
		{
			ok = false;
			break;
		}
		if (stereo)
		{
			const std::uint8_t *rightBlock = data + Ps2AdpcmStream::kHeaderBytes +
				(static_cast<std::size_t>(totalBlocks) + blockIndex) * Ps2AdpcmStream::kBlockBytes;
			if (!Ps2AdpcmStream::decodeBlock(rightBlock, rightState, rightPcm.data()))
			{
				ok = false;
				break;
			}
		}

		const std::uint32_t framesLeft = header.samplesPerChannel - emitted;
		const std::uint32_t blockFrames =
			std::min<std::uint32_t>(Ps2AdpcmStream::kSamplesPerBlock, framesLeft);
		for (std::uint32_t frame = 0; frame < blockFrames; ++frame)
		{
			pcm[(emitted + frame) * header.channels] = leftPcm[frame];
			if (stereo)
				pcm[(emitted + frame) * header.channels + 1] = rightPcm[frame];
		}
		emitted += blockFrames;
	}

	if (!ok)
	{
		linearFree(pcm);
		return false;
	}

	out.pcm = pcm;
	out.frames = emitted;
	out.bytes = static_cast<std::size_t>(bytes64);
	return true;
}

void freeSfxEntry(std::unordered_map<std::string, CachedSample>::iterator it)
{
	// ndsp may still be DMA-reading a buffer queued on a previous play of
	// this sample; clear every channel that holds it before freeing.
	for (int channel = CTR_SFX_FIRST_CHANNEL; channel < CTR_SFX_CHANNEL_COUNT; ++channel)
		ndspChnWaveBufClear(channel);
	s_sfxCacheBytes -= it->second.allocBytes;
	linearFree(it->second.pcm);
	s_sfxCache.erase(it);
}

void evictFor(std::size_t incomingBytes)
{
	while (s_sfxCacheBytes + incomingBytes > CTR_SFX_CACHE_BUDGET_BYTES && !s_sfxCache.empty())
	{
		auto oldest = s_sfxCache.begin();
		for (auto it = s_sfxCache.begin(); it != s_sfxCache.end(); ++it)
			if (it->second.lastUsed < oldest->second.lastUsed)
				oldest = it;
		freeSfxEntry(oldest);
	}
}

CachedSample *getDecodedSfx(const std::string &path)
{
	const auto hit = s_sfxCache.find(path);
	if (hit != s_sfxCache.end())
	{
		hit->second.lastUsed = ++s_cacheClock;
		return &hit->second;
	}
	if (s_rejectedSfx.count(path) != 0)
		return nullptr;

	if (!audioPathHasExtension(path, ".adp"))
	{
		MC_LOG_DEBUG("audio", "unsupported audio asset %s\n", path.c_str());
		s_rejectedSfx[path] = true;
		return nullptr;
	}

	// The size gate runs off the directory entry, before the read -- the
	// PS2 path's rule -- so a rejected bed costs no allocation.
	const long fileBytes = PlatformResources::fileSize(path);
	if (fileBytes <= 0)
	{
		MC_LOG_DEBUG("audio", "missing audio %s\n", path.c_str());
		s_rejectedSfx[path] = true;
		return nullptr;
	}
	if (fileBytes > CTR_SFX_MAX_FILE_BYTES)
	{
		MC_LOG_DEBUG("audio", "skip large ADP %s (%ld bytes)\n", path.c_str(), fileBytes);
		s_rejectedSfx[path] = true;
		return nullptr;
	}

	unsigned int size = 0;
	unsigned char *data = PlatformResources::loadFile(path, &size);
	if (data == nullptr)
	{
		s_rejectedSfx[path] = true;
		return nullptr;
	}

	Ps2AdpcmStream::Header header;
	DecodedSfx decoded;
	if (!validAdpHeader(data, size, header) ||
	    !decodeAdpSfx(data, header, decoded))
	{
		MC_LOG_WARN("audio", "3ds: adp decode failed %s\n", path.c_str());
		std::free(data);
		s_rejectedSfx[path] = true;
		return nullptr;
	}
	std::free(data);

	evictFor(decoded.bytes);

	CachedSample &entry = s_sfxCache[path];
	entry.pcm = decoded.pcm;
	entry.samples = decoded.frames;
	entry.channels = header.channels;
	entry.rate = header.sampleRate;
	entry.lastUsed = ++s_cacheClock;
	entry.allocBytes = decoded.bytes;
	// The per-channel wave bufs stay zeroed here: each play arms its own
	// channel's node (see playSfxSample), exactly where the Wii arms
	// ASND_SetVoice's arguments at call time.
	std::memset(entry.wave, 0, sizeof(entry.wave));
	DSP_FlushDataCache(entry.pcm, entry.allocBytes);
	s_sfxCacheBytes += decoded.bytes;
	return &entry;
}

int nextSfxChannel()
{
	const int usable = CTR_SFX_CHANNEL_COUNT - CTR_SFX_FIRST_CHANNEL;
	for (int offset = 0; offset < usable; ++offset)
	{
		const int channel = CTR_SFX_FIRST_CHANNEL +
			((s_nextSfxChannel - CTR_SFX_FIRST_CHANNEL + offset) % usable);
		if (ndspChnIsPlaying(channel))
			continue;
		s_nextSfxChannel = channel + 1;
		if (s_nextSfxChannel >= CTR_SFX_CHANNEL_COUNT)
			s_nextSfxChannel = CTR_SFX_FIRST_CHANNEL;
		return channel;
	}
	// All busy: steal the round-robin cursor's channel outright.
	const int channel = s_nextSfxChannel;
	s_nextSfxChannel = channel + 1;
	if (s_nextSfxChannel >= CTR_SFX_CHANNEL_COUNT)
		s_nextSfxChannel = CTR_SFX_FIRST_CHANNEL;
	return channel;
}

void playSfxSample(CachedSample *sample, float gain, float pitch)
{
	if (sample == nullptr || sample->pcm == nullptr)
		return;
	const int channel = nextSfxChannel();
	ndspChnReset(channel);
	// POLYPHASE, like every other channel -- see the note in loadLibrary().
	ndspChnSetInterp(channel, NDSP_INTERP_POLYPHASE);
	ndspChnSetFormat(channel, sample->channels >= 2 ? NDSP_FORMAT_STEREO_PCM16
	                                                : NDSP_FORMAT_MONO_PCM16);
	const float rate = pitch > 0.0f ? sample->rate * pitch : static_cast<float>(sample->rate);
	ndspChnSetRate(channel, rate);
	setChannelMix(channel, gain);

	// Arm this channel's own queue node, the ndsp counterpart of the
	// buffer+size arguments ASND_SetVoice takes on the Wii: data the DSP
	// can DMA (the linearAlloc'd decode), frame count per channel, no loop.
	ndspWaveBuf &wave = sample->wave[channel];
	std::memset(&wave, 0, sizeof(wave));
	wave.data_vaddr = sample->pcm;
	wave.nsamples = sample->samples;
	wave.looping = false;
	ndspChnWaveBufAdd(channel, &wave);
}

int streamChannelCount()
{
	switch (s_stream.format)
	{
	case StreamFormat::Adp:  return s_stream.header.channels;
	case StreamFormat::Wav:  return s_stream.wav.channels;
	case StreamFormat::RawPcm: return CTR_RAW_PCM_CHANNELS;
	}
	return 1;
}

int streamSampleRate()
{
	switch (s_stream.format)
	{
	case StreamFormat::Adp:  return s_stream.header.sampleRate;
	case StreamFormat::Wav:  return s_stream.wav.sampleRate;
	case StreamFormat::RawPcm: return CTR_RAW_PCM_SAMPLE_RATE;
	}
	return CTR_RAW_PCM_SAMPLE_RATE;
}

// The RIFF chunk walk, the same one Ps2MusicStream::readWavHeader performs
// over its own probe handle: every chunk header is read from an absolute
// cursor so the position never needs a tell(), the fmt chunk decides
// whether the file is playable (PCM s16le, mono/stereo), and the data
// chunk's offset/size are what the stream reads back. `file` is expected to
// be open at RIFF start; it is left open either way, seeked to the data
// payload on success -- the caller owns it.
bool probeWavStream(DsStreamFile &file, WavStreamInfo &info)
{
	std::array<std::uint8_t, 12> riff{};
	if (!file.readExact(riff.data(), static_cast<int>(riff.size())) ||
	    std::memcmp(riff.data(), "RIFF", 4) != 0 ||
	    std::memcmp(riff.data() + 8, "WAVE", 4) != 0)
		return false;

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

	if (!haveFormat || !haveData || info.sampleRate <= 0 || info.dataBytes == 0 ||
	    !file.seek(info.dataOffset))
		return false;
	return true;
}

// Opens the stream's source and probes which of the three PS2 asset shapes
// it is -- the 3DS counterpart of Ps2MusicStream's validStreamFile/musicThread
// format trio, so a pack converted for the PS2 plays here unmodified. On
// success the reader(s) are open and positioned at the first playable byte:
// after the APCM header (with the right plane seeked for stereo ADP), at
// offset 0 for raw PCM, at the data payload for WAV.
bool openAudioStream(const std::string &path)
{
	s_stream.left.close();
	s_stream.right.close();
	s_stream.format = StreamFormat::Adp;
	s_stream.wav = WavStreamInfo{};
	s_stream.framesTotal = 0;

	if (audioPathHasExtension(path, ".pcm"))
	{
		// Headerless s16le: the rate/channel convention is the PS2's, and
		// the frame count is the whole file's worth of samples (an odd
		// trailing byte is dropped rather than read as a half sample).
		if (!s_stream.left.open(path.c_str()))
			return false;
		const long size = s_stream.left.size();
		if (size < 2)
			return false;
		s_stream.format = StreamFormat::RawPcm;
		s_stream.framesTotal = static_cast<std::uint32_t>(size / 2);
		return true;
	}

	if (audioPathHasExtension(path, ".wav"))
	{
		if (!s_stream.left.open(path.c_str()))
			return false;
		WavStreamInfo wav{};
		if (!probeWavStream(s_stream.left, wav))
			return false;
		s_stream.format = StreamFormat::Wav;
		s_stream.wav = wav;
		s_stream.framesTotal = wav.dataBytes /
			(static_cast<std::uint32_t>(wav.channels) * sizeof(std::int16_t));
		return s_stream.framesTotal > 0;
	}

	// ADP: the left plane starts right after the 16-byte header, the right
	// plane (stereo only) after the left plane's blocks -- the layout
	// streamAdpcm() seeks to on the PS2.
	Ps2AdpcmStream::Header &header = s_stream.header;
	std::array<std::uint8_t, Ps2AdpcmStream::kHeaderBytes> bytes{};

	if (!s_stream.left.open(path.c_str()) ||
	    !s_stream.left.readExact(bytes.data(), static_cast<int>(bytes.size())))
		return false;

	const long streamSize = s_stream.left.size();
	if (streamSize < 0 ||
	    !validAdpHeader(bytes.data(), static_cast<std::size_t>(streamSize), header))
		return false;

	const std::uint32_t totalBlocks =
		(header.samplesPerChannel + Ps2AdpcmStream::kSamplesPerBlock - 1u) /
		Ps2AdpcmStream::kSamplesPerBlock;
	if (!s_stream.left.seek(Ps2AdpcmStream::kHeaderBytes))
		return false;
	if (header.channels == 2)
	{
		const long rightOffset = Ps2AdpcmStream::kHeaderBytes +
			static_cast<long>(totalBlocks) * Ps2AdpcmStream::kBlockBytes;
		if (!s_stream.right.open(path.c_str()) || !s_stream.right.seek(rightOffset))
			return false;
	}
	return true;
}

// Raw-PCM and WAV fills: the bytes on disk are already interleaved s16le
// PCM, so a fill is a plain read -- no decoder state, no per-channel
// readers. Returns the frame count read, 0 at end of stream, negative on a
// read failure.
int decodePcmStreamFrames(std::int16_t *out, int framesWanted)
{
	const int channels = streamChannelCount();
	const std::uint32_t framesLeft = s_stream.framesTotal - s_stream.framesDecoded;
	if (framesLeft == 0)
		return 0;
	const int frames = static_cast<int>(std::min<std::uint32_t>(
		static_cast<std::uint32_t>(framesWanted), framesLeft));
	const int bytes = frames * channels * static_cast<int>(sizeof(std::int16_t));
	if (!s_stream.left.readExact(out, bytes))
		return -1;
	s_stream.framesDecoded += static_cast<std::uint32_t>(frames);
	return frames;
}

// Pulls the next framesWanted interleaved frames off the stream source. The
// ADP path walks whole 16-byte blocks: a buffer fills to the largest whole
// multiple of 28 frames that fits (8192 -> 8176), which keeps the
// per-channel decoder state and the two block readers in lockstep across
// refill calls -- the invariant streamAdpcm()'s chunk loop keeps on the PS2.
// The PCM paths read whole frames directly. Returns the frame count
// written, 0 at end of stream, negative on a read/decode failure.
int decodeStreamFrames(std::int16_t *out, int framesWanted)
{
	if (s_stream.format != StreamFormat::Adp)
		return decodePcmStreamFrames(out, framesWanted);

	const Ps2AdpcmStream::Header &header = s_stream.header;
	const bool stereo = header.channels == 2;
	std::array<std::uint8_t, Ps2AdpcmStream::kBlockBytes> block{};
	std::array<std::int16_t, Ps2AdpcmStream::kSamplesPerBlock> leftPcm{};
	std::array<std::int16_t, Ps2AdpcmStream::kSamplesPerBlock> rightPcm{};

	int frames = 0;
	while (frames + Ps2AdpcmStream::kSamplesPerBlock <= framesWanted &&
	       s_stream.framesDecoded < header.samplesPerChannel)
	{
		if (!s_stream.left.readExact(block.data(), Ps2AdpcmStream::kBlockBytes) ||
		    !Ps2AdpcmStream::decodeBlock(block.data(), s_stream.leftState, leftPcm.data()))
			return -1;
		if (stereo)
		{
			if (!s_stream.right.readExact(block.data(), Ps2AdpcmStream::kBlockBytes) ||
			    !Ps2AdpcmStream::decodeBlock(block.data(), s_stream.rightState, rightPcm.data()))
				return -1;
		}

		const std::uint32_t framesLeft = header.samplesPerChannel - s_stream.framesDecoded;
		const std::uint32_t blockFrames =
			std::min<std::uint32_t>(Ps2AdpcmStream::kSamplesPerBlock, framesLeft);
		for (std::uint32_t frame = 0; frame < blockFrames; ++frame)
		{
			out[static_cast<std::size_t>(frames) * header.channels] = leftPcm[frame];
			if (stereo)
				out[static_cast<std::size_t>(frames) * header.channels + 1] = rightPcm[frame];
			++frames;
		}
		s_stream.framesDecoded += blockFrames;
	}
	return frames;
}

bool startAudioStream(const std::string &path, float gain)
{
	const long fileBytes = PlatformResources::fileSize(path);
	if (fileBytes <= 0 ||
	    (!audioPathHasExtension(path, ".adp") && !audioPathHasExtension(path, ".pcm") &&
	     !audioPathHasExtension(path, ".wav")))
		return false;

	// Whatever was on channel 0 goes first: the same internal stop the
	// PS2's Ps2MusicStream::start() performs, so a start can never race a
	// previous stream's refill thread.
	stopStream();

	// Open and probe first: the probe names the format and carries channels
	// and rate, and a failed probe costs nothing (a half-open pair of
	// readers is closed on the way out so a rejected start leaves no
	// handles behind).
	if (!openAudioStream(path))
	{
		s_stream.left.close();
		s_stream.right.close();
		MC_LOG_WARN("audio", "3ds: audio stream open failed %s\n", path.c_str());
		return false;
	}
	const int channels = streamChannelCount();
	const int sampleRate = streamSampleRate();

	s_stream.path = path;
	s_stream.gain = gain;
	s_stream.framesDecoded = 0;
	s_stream.leftState = Ps2AdpcmStream::ChannelState{};
	s_stream.rightState = Ps2AdpcmStream::ChannelState{};
	s_stream.eof = false;

	// A fresh ndsp channel for the fresh stream: reset clears any queue the
	// previous stream left behind, and the format/rate/mix are set once
	// here -- the thread only queues buffers.
	//
	// Interpolation is the whole "music sounds robotic" question: the DSP
	// runs at ~32728 Hz while the ADP set arrives at its own rate (22050
	// from the converter), so this channel is being resampled by the DSP no
	// matter what. NDSP_INTERP_NONE nearest-neighbours that step and folds
	// the spectrum back over itself -- the buzz the report describes.
	// POLYPHASE filters properly.
	ndspChnReset(CTR_MUSIC_CHANNEL);
	ndspChnSetInterp(CTR_MUSIC_CHANNEL, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(CTR_MUSIC_CHANNEL, static_cast<float>(sampleRate));
	ndspChnSetFormat(CTR_MUSIC_CHANNEL, channels == 2 ? NDSP_FORMAT_STEREO_PCM16
	                                                 : NDSP_FORMAT_MONO_PCM16);
	setChannelMix(CTR_MUSIC_CHANNEL, gain);

	const std::size_t bufferBytes = static_cast<std::size_t>(CTR_STREAM_BUFFER_FRAMES) *
	                                static_cast<std::size_t>(channels) *
	                                sizeof(std::int16_t);
	for (int i = 0; i < CTR_STREAM_BUFFER_COUNT; ++i)
	{
		s_stream.pcm[i] = static_cast<std::int16_t *>(linearAlloc(bufferBytes));
		if (s_stream.pcm[i] == nullptr)
		{
			MC_LOG_WARN("audio", "3ds: stream ring allocation failed (%d x %zu bytes); stopped\n",
			            CTR_STREAM_BUFFER_COUNT, bufferBytes);
			stopStream();
			return false;
		}
		std::memset(&s_stream.wave[i], 0, sizeof(ndspWaveBuf));
		s_stream.wave[i].data_vaddr = s_stream.pcm[i];
	}

	s_stream.running = false;
	s_stream.eof = false;

	// Prebuffer: fill and queue the whole ring before returning, so playback
	// starts with the full ring behind it and the first callback refill only
	// has to replace one buffer.
	for (int i = 0; i < CTR_STREAM_BUFFER_COUNT; ++i)
	{
		const int frames = decodeStreamFrames(s_stream.pcm[i], CTR_STREAM_BUFFER_FRAMES);
		if (frames <= 0)
		{
			if (frames < 0)
				MC_LOG_WARN("audio", "3ds: audio stream failed %s\n", path.c_str());
			s_stream.eof = true;
			break;
		}
		s_stream.wave[i].nsamples = static_cast<u32>(frames);
		DSP_FlushDataCache(s_stream.pcm[i],
		                   static_cast<std::size_t>(frames) * channels * sizeof(std::int16_t));
		ndspChnWaveBufAdd(CTR_MUSIC_CHANNEL, &s_stream.wave[i]);
	}

	s_stream.running = true;
	s_streamActive = true;
	s_stream.thread = threadCreate(streamRefillThreadMain, nullptr,
	                               CTR_STREAM_THREAD_STACK_BYTES,
	                               CTR_STREAM_THREAD_PRIORITY, -2, false);
	if (s_stream.thread == nullptr)
	{
		// Without the thread the prebuffered ring above is all the music
		// there is; logged because a stream that stalls otherwise reads as a
		// silent one.
		MC_LOG_WARN("audio", "3ds: stream thread creation failed; music stops when the ring drains\n");
	}
	return true;
}

void stopStream()
{
	if (!s_streamActive && !s_stream.left.isOpen())
		return;

	// Told to stop: clear the queue, then every resource the stream owned
	// goes back. The thread checks s_stream.running, so clearing it first
	// means no refill can race the teardown.
	s_stream.running = false;
	s_stream.eof = true;
	if (s_stream.thread != nullptr)
	{
		// The thread observes running/eof and exits its loop promptly; join
		// before the teardown below frees anything it might still touch.
		threadJoin(s_stream.thread, U64_MAX);
		threadFree(s_stream.thread);
		s_stream.thread = nullptr;
	}
	ndspChnWaveBufClear(CTR_MUSIC_CHANNEL);
	ndspChnReset(CTR_MUSIC_CHANNEL);
	for (int i = 0; i < CTR_STREAM_BUFFER_COUNT; ++i)
	{
		if (s_stream.pcm[i] != nullptr)
		{
			linearFree(s_stream.pcm[i]);
			s_stream.pcm[i] = nullptr;
		}
	}
	s_stream.left.close();
	s_stream.right.close();
	s_stream.path.clear();
	s_streamActive = false;
}

// One pass over the ring, refilling whatever the channel has finished
// (NDSP_WBUF_DONE / FREE), pulling whole ADP blocks through the shared
// decoder or whole PCM frames through the plain reader -- the same pull the
// PS2's stream thread performs over its own block reader. Returns true when
// it decoded anything.
bool streamRefillPoll()
{
	if (!s_stream.running || s_stream.eof || !s_stream.left.isOpen())
		return false;

	bool decoded = false;
	for (int i = 0; i < CTR_STREAM_BUFFER_COUNT; ++i)
	{
		if (s_stream.wave[i].status == NDSP_WBUF_QUEUED ||
		    s_stream.wave[i].status == NDSP_WBUF_PLAYING)
			continue;

		const int frames = decodeStreamFrames(s_stream.pcm[i], CTR_STREAM_BUFFER_FRAMES);
		if (frames <= 0)
		{
			if (frames < 0)
				MC_LOG_WARN("audio", "3ds: audio stream failed %s\n", s_stream.path.c_str());
			// End of stream (or a broken one): stop refilling and let the
			// queued tail drain. playRandomMusicIfReady() observes the drain
			// and re-arms the next track after its timer, exactly like the
			// PS2 path.
			s_stream.eof = true;
			return decoded;
		}
		s_stream.wave[i].nsamples = static_cast<u32>(frames);
		DSP_FlushDataCache(s_stream.pcm[i],
		                   static_cast<std::size_t>(frames) * streamChannelCount() * sizeof(std::int16_t));
		ndspChnWaveBufAdd(CTR_MUSIC_CHANNEL, &s_stream.wave[i]);
		decoded = true;
	}
	return decoded;
}

// The stream thread. A priority above the game thread's own 0x30, so the
// tick-heavy game thread cannot starve it -- that starvation, on a thread run
// below the game thread, was the original "robotic music". Exits when
// stopStream() clears s_stream.running (it joins before freeing anything) or
// when the stream ends; without the thread the prebuffered ring is all the
// music there is.
void streamRefillThreadMain(void *)
{
	for (;;)
	{
		if (!s_stream.running || s_stream.eof)
			break;
		const bool decoded = streamRefillPoll();
		svcSleepThread(decoded ? 2000000ll : 16000000ll);
	}
}

} // namespace

void *SoundManager::sndSystem = nullptr;
bool SoundManager::loaded = false;

SoundManager::SoundManager()
    : soundPoolSounds(), soundPoolStreaming(), soundPoolMusic(), soundVolume(0),
      options(nullptr), rand(), ticksBeforeMusic(rand.nextInt(12000))
{
}

void SoundManager::loadSoundSettings(GameSettings *gamesettings)
{
	soundPoolStreaming.motionX = false;
	options = gamesettings;
	s_soundVolume = gamesettings ? gamesettings->soundVolume : 1.0f;
	s_musicVolume = gamesettings ? gamesettings->musicVolume : 1.0f;

	if (!loaded && (gamesettings == nullptr || gamesettings->soundVolume != 0.0f ||
	                gamesettings->musicVolume != 0.0f))
		tryToSetLibraryAndCodecs();
}

void SoundManager::tryToSetLibraryAndCodecs()
{
	if (loaded)
		return;

	if (R_FAILED(ndspInit()))
	{
		MC_LOG_WARN("audio", "3ds: ndspInit failed -- no sound (dspfirm.cdc missing on SD?)\n");
		return;
	}

	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspSetMasterVol(1.0f);
	// No sound-frame callback: the stream refills on its own thread now (see
	// the StreamState note) -- the callback's thread could not host a
	// decode.
	// The interpolation mode matters on *every* channel here, because ndsp
	// mixes at NDSP_SAMPLE_RATE (SYSCLOCK_SOC/512 ~ 32728 Hz) while the
	// assets arrive at their own rates -- the ADP set at 22050 -- so each
	// channel is being resampled by the DSP no matter what.
	// NDSP_INTERP_NONE is nearest-neighbour through that conversion: each
	// output sample copies whichever input sample is closest, folding
	// everything above the Nyquist step back over the spectrum. That
	// aliasing is the metallic/robotic cast the music was reported with,
	// and it is on the SFX path too. POLYPHASE is ndsp's band-limited
	// resampler -- the step a rate conversion actually needs -- and is what
	// this loop leaves every channel on; playSfxSample/startAudioStream must
	// not undo it (see their own ndspChnSetInterp calls).
	for (int channel = 0; channel < CTR_SFX_CHANNEL_COUNT; ++channel)
	{
		ndspChnReset(channel);
		ndspChnSetInterp(channel, NDSP_INTERP_POLYPHASE);
	}
	s_nextSfxChannel = CTR_SFX_FIRST_CHANNEL;
	loaded = true;
	MC_LOG_DEBUG("audio", "3ds: ndsp ready\n");
}

void SoundManager::onSoundOptionsChanged()
{
	if (options)
	{
		s_soundVolume = options->soundVolume;
		s_musicVolume = options->musicVolume;
	}

	if (!loaded && options && (options->soundVolume != 0.0f || options->musicVolume != 0.0f))
		tryToSetLibraryAndCodecs();

	if (loaded && s_streamActive)
	{
		const float gain = s_streamIsMusic ? s_musicVolume : s_soundVolume * s_stream.gain;
		if (gain <= 0.0f)
			stopStream();
		else
			setChannelMix(CTR_MUSIC_CHANNEL, gain);
	}
}

void SoundManager::closeMinecraft()
{
	stopStream();
	for (auto it = s_sfxCache.begin(); it != s_sfxCache.end(); it = s_sfxCache.begin())
		freeSfxEntry(it);
	s_rejectedSfx.clear();
	if (loaded)
	{
		for (int channel = 0; channel < CTR_SFX_CHANNEL_COUNT; ++channel)
			ndspChnWaveBufClear(channel);
		ndspExit();
	}
	loaded = false;
}

void SoundManager::addSound(const jstring &s, const std::string &file)
{
	if (audioPathHasExtension(file, ".adp"))
		soundPoolSounds.addSound(s, file);
}

void SoundManager::addStreaming(const jstring &s, const std::string &file)
{
	// The same three shapes the PS2's streamer accepts, so a pack staged for
	// the PS2 registers here unmodified -- the pack's records arrive as raw
	// .pcm (scripts/ogg to pcm), and .wav is the third script's shape.
	if (audioPathHasExtension(file, ".adp") || audioPathHasExtension(file, ".pcm") ||
	    audioPathHasExtension(file, ".wav"))
		soundPoolStreaming.addSound(s, file);
}

void SoundManager::addMusic(const jstring &s, const std::string &file)
{
	// Same trio: the pack's music/ and newmusic/ folders carry raw .pcm
	// tracks (the PS2's addMusic accepts exactly those), while an all-.adp
	// pack (scripts/ogg to adp over the whole tree) still registers too.
	if (audioPathHasExtension(file, ".adp") || audioPathHasExtension(file, ".pcm") ||
	    audioPathHasExtension(file, ".wav"))
		soundPoolMusic.addSound(s, file);
}

bool SoundManager::playMusicFileNow(const std::string &file)
{
	if (!loaded || !options || options->musicVolume == 0.0f)
		return false;

	// startAudioStream() stops whatever is on channel 0 itself, after the
	// format probe -- the PS2's order too, so a rejected request leaves
	// the current track alone.
	if (!startAudioStream(file, s_musicVolume))
		return false;

	s_streamIsMusic = true;
	ticksBeforeMusic = rand.nextInt(12000) + 12000;
	return true;
}

void SoundManager::playRandomMusicIfReady()
{
	if (!loaded || !options || options->musicVolume == 0.0f)
		return;

	// Channel 0 is a single-stream resource: whatever is live keeps it --
	// music never plays over a record, a record never over music -- until
	// its tail has drained (ndspChnIsPlaying goes false), and only a
	// drained stream is cleared here so the timer below can move on. The
	// PS2 keeps the same rule through its Ps2StreamKind check.
	if (s_streamActive && !ndspChnIsPlaying(CTR_MUSIC_CHANNEL))
		stopStream();
	if (s_streamActive)
		return;

	if (ticksBeforeMusic > 0)
	{
		ticksBeforeMusic--;
		return;
	}

	SoundPoolEntry *entry = soundPoolMusic.getRandomSound();
	ticksBeforeMusic = rand.nextInt(12000) + 12000;
	if (entry == nullptr)
		return;

	if (startAudioStream(entry->soundUrl, s_musicVolume))
		s_streamIsMusic = true;
}

void SoundManager::setListenerPosition(EntityLiving *entityliving, float partialTick)
{
	if (!loaded || !options || options->soundVolume == 0.0f)
		return;

	updateAudioListener(s_listener, entityliving, partialTick);
}

void SoundManager::playStreaming(const jstring &s, float x, float y, float z, float volume, float)
{
	if (!loaded || !options || (options->soundVolume == 0.0f && !s.empty()))
		return;

	if (s.empty())
	{
		if (s_streamActive && !s_streamIsMusic)
			stopStream();
		return;
	}

	if (volume <= 0.0f)
		return;

	const float attenuation = audioStreamingAttenuation(s_listener, x, y, z);
	if (attenuation <= 0.0f)
		return;

	SoundPoolEntry *entry = soundPoolStreaming.getRandomSoundFromSoundPool(s);
	if (entry == nullptr)
	{
		MC_LOG_WARN("audio", "missing 3ds streaming sound %s\n", s.c_str());
		return;
	}

	// Long records take the channel from whatever is playing -- the same
	// handover the PS2 performs -- and stream through the same ring as
	// music does.
	stopStream();
	ticksBeforeMusic = rand.nextInt(12000) + 12000;
	if (startAudioStream(entry->soundUrl, 0.5f * attenuation * s_soundVolume))
		s_streamIsMusic = false;
}

void SoundManager::playSound(const jstring &s, float x, float y, float z, float volume, float pitch)
{
	if (!loaded || !options || options->soundVolume == 0.0f)
		return;

	SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
	if (entry == nullptr || volume <= 0.0f)
		return;

	const float attenuation = audioSpatialAttenuation(s_listener, x, y, z, volume);
	if (attenuation <= 0.0f)
		return;

	CachedSample *sample = getDecodedSfx(entry->soundUrl);
	if (sample == nullptr)
		return;

	playSfxSample(sample, attenuation * s_soundVolume, pitch);
}

void SoundManager::playSoundFX(const jstring &s, float volume, float pitch)
{
	if (!loaded || !options || options->soundVolume == 0.0f)
		return;

	SoundPoolEntry *entry = soundPoolSounds.getRandomSoundFromSoundPool(s);
	if (entry == nullptr || volume <= 0.0f)
		return;

	CachedSample *sample = getDecodedSfx(entry->soundUrl);
	if (sample == nullptr)
		return;

	playSfxSample(sample, volume * s_soundVolume, pitch);
}

#endif // NO_SOUND
