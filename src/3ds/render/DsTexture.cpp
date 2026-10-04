// DsTexture.cpp -- PICA200 texture storage for the citro3d backend.
//
// Three facts about the PICA's texture units drive everything in this file:
//
//   * Sizes must be power-of-two and between 8 and 1024 -- citro3d enforces
//     both in C3D_TexInitWithParams. The game's PNGs are not always, so each
//     record stores the rounded-up GPU size next to the real image size, and
//     the ratio is folded into the texcoord by the shader's texScale uniform.
//     With that, the padding is invisible and GL_REPEAT keeps its meaning
//     without any UV rewriting on the CPU. Images larger than 1024 texels
//     (HD texture packs) are stored halved until they fit -- see allocate().
//
//   * Texel layout is tiled: pixels are Morton-interleaved (Z-order) inside
//     each 8x8 tile and the tiles are laid out raster-wise across the level.
//     That is the layout citro3d's own mipmap downsampler assumes -- its
//     block-walking code reads four consecutive words as a 2x2 quad, which
//     only holds for this interleaving -- and it is why every upload goes
//     through the swizzle below instead of a plain memcpy.
//
// Texel words are byte-swapped on the way in. The PICA's GPU_RGBA8 texture
// format stores each 32-bit texel big-endian: R is the most significant
// byte, so in memory the bytes run A,B,G,R -- the reverse of the game's
// R,G,B,A sequence (the 3DS GPU emulators decode RGBA8 texels as
// {bytes[3], bytes[2], bytes[1], bytes[0]} = R,G,B,A, which is where this
// requirement is grounded). Uploading without the swap made the GPU read
// the alpha byte as red and the red byte as alpha, so every textured
// surface came out with rotated hues and garbage transparency -- the
// "wrong colours" of the port's first run. Two things are deliberately
// NOT swapped: vertex colours (attribute fetch loads bytes in order; see
// DsRender.cpp) and the render-target clear value (0xRRGGBBAA is already
// the native word order).

#include "3ds/render/DsTexture.h"

#include <3ds.h>
#include <citro3d.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "platform/Log.h"

namespace ds
{
namespace texture
{
namespace
{

struct Record
{
	C3D_Tex tex{};
	bool valid = false;
	int logicalW = 0;  // the image size the game addresses its UVs against
	int logicalH = 0;
	int storedW = 0;   // the size actually rasterised into GPU storage: the
	int storedH = 0;   // logical size, halved while a >1024-texel atlas is
	int downshift = 0;  // reduced to fit; downshift is log2 of that factor
	bool blur = true;
	bool clamp = false;
};

std::unordered_map<int, Record> s_records;

// Shared 8x8 white texel block, bound whenever a texture name has no storage
// behind it. 8x8 because citro3d rejects anything smaller; white because the
// shader modulates texel * vertex colour, so white lets the vertex colour
// through unchanged instead of sampling undefined texels.
C3D_Tex s_fallback{};
bool s_fallbackReady = false;

// Morton (Z) order of a pixel within its 8x8 tile: x bits on the even bit
// lanes, y bits on the odd ones. Three lanes are all an 8x8 tile needs.
std::uint32_t mortonInTile(std::uint32_t x, std::uint32_t y)
{
	std::uint32_t z = 0;
	for (std::uint32_t i = 0; i < 3; ++i)
	{
		z |= ((x >> i) & 1u) << (2 * i);
		z |= ((y >> i) & 1u) << (2 * i + 1);
	}
	return z;
}

int roundUpToPot(int value)
{
	int pot = 8;
	while (pot < value)
		pot <<= 1;
	return pot;
}

void applyParameters(Record& record)
{
	// Mipmap-less filtering: this backend reports no mipmap support, so the
	// minification filter never has a mip chain to walk.
	const GPU_TEXTURE_FILTER_PARAM filter = record.blur ? GPU_LINEAR : GPU_NEAREST;
	C3D_TexSetFilter(&record.tex, filter, filter);
	const GPU_TEXTURE_WRAP_PARAM wrap = record.clamp ? GPU_CLAMP_TO_EDGE : GPU_REPEAT;
	C3D_TexSetWrap(&record.tex, wrap, wrap);
}

Record* find(int id)
{
	const auto it = s_records.find(id);
	return it == s_records.end() ? nullptr : &it->second;
}

// The ARM11 data caches are not coherent with the PICA200's texture
// fetches: a texel written by the CPU can sit dirty in L1 (Old 3DS) or the
// L2 bypass (New 3DS) while the GPU samples the DRAM copy straight through
// the physical address baked into the frame's command list. The sampler
// then reads whatever previously occupied the freshly recycled linear-heap
// block -- audio PCM, old mesh floats, a previous texture -- which renders
// as garbage pixels of random colours: the intermittent entity/skin
// corruption of the 2026-10 multiplayer report, fed by the constant
// linear-heap churn of server chunk streaming and the mid-session uploads
// of first-seen entity/armor/item textures. Flush the data cache after
// every CPU write into texel storage: one GSP syscall, negligible against
// the upload it follows.
void flushAfterWrite(Record& record)
{
	if (record.valid)
		C3D_TexFlush(&record.tex);
}

void destroyFallback()
{
	if (s_fallbackReady)
	{
		C3D_TexDelete(&s_fallback);
		s_fallbackReady = false;
	}
}

void ensureFallback()
{
	if (s_fallbackReady)
		return;
	if (!C3D_TexInit(&s_fallback, 8, 8, GPU_RGBA8))
		return;
	std::uint32_t* dst = static_cast<std::uint32_t*>(s_fallback.data);
	for (std::uint32_t y = 0; y < 8; ++y)
		for (std::uint32_t x = 0; x < 8; ++x)
			dst[mortonInTile(x, y)] = 0xFFFFFFFFu;
	// Same coherency rule as upload(): the ARM11 data cache is not shared
	// with the PICA200, so a CPU-written texel is only visible to the
	// sampler after C3D_TexFlush.
	C3D_TexFlush(&s_fallback);
	C3D_TexSetFilter(&s_fallback, GPU_NEAREST, GPU_NEAREST);
	C3D_TexSetWrap(&s_fallback, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	s_fallbackReady = true;
}

} // namespace

void resetAll()
{
	for (auto& entry : s_records)
		if (entry.second.valid)
			C3D_TexDelete(&entry.second.tex);
	s_records.clear();
	destroyFallback();
}

bool allocate(int id, int width, int height)
{
	if (id <= 0 || width <= 0 || height <= 0)
		return false;
	destroy(id);

	Record record;
	record.logicalW = width;
	record.logicalH = height;

	// PICA200 textures must be power-of-two within 8..1024, but the image the
	// game hands over is not the port's to choose: OptiCraft's HD texture
	// packs routinely carry 2048-texel atlases. Rejecting those outright is
	// what used to happen, and every rejected texture silently became the
	// 8x8 white fallback -- blank buttons, a missing title logo, unreadable
	// text, the first run's "everything is white" report. Halving the image
	// until it fits is the honest alternative: the whole atlas stays visible
	// (halved, so an HD pack simply loses its extra detail), the game's UVs
	// keep addressing the same 0..1 image space, and bind() reports the
	// stored-to-padded ratio so texScale keeps landing every UV on content.
	int storedW = width;
	int storedH = height;
	int downshift = 0;
	while (roundUpToPot(storedW) > 1024 || roundUpToPot(storedH) > 1024)
	{
		storedW = std::max(1, storedW / 2);
		storedH = std::max(1, storedH / 2);
		++downshift;
	}
	const int gpuW = roundUpToPot(storedW);
	const int gpuH = roundUpToPot(storedH);
	if (downshift > 0)
	{
		MC_LOG_WARN("render",
			"3ds: texture %d (%dx%d) exceeds the PICA200 1024-texel limit; "
			"stored halved %dx to %dx%d\n",
			id, width, height, downshift, storedW, storedH);
	}
	if (!C3D_TexInit(&record.tex, static_cast<u16>(gpuW), static_cast<u16>(gpuH),
	                 GPU_RGBA8))
	{
		// Not the 1024-texel rejection above: this is the linear heap saying
		// no. Logged because it is the other silent path to the white
		// fallback -- an out-of-memory upload looks identical on screen.
		MC_LOG_WARN("render",
			"3ds: texture %d storage (%dx%d RGBA8) allocation failed\n",
			id, gpuW, gpuH);
		return false;
	}
	record.valid = true;
	record.storedW = storedW;
	record.storedH = storedH;
	record.downshift = downshift;
	applyParameters(record);
	s_records[id] = record;
	return true;
}

void destroy(int id)
{
	Record* record = find(id);
	if (record == nullptr)
		return;
	if (record->valid)
		C3D_TexDelete(&record->tex);
	s_records.erase(id);
}

bool exists(int id)
{
	const Record* record = find(id);
	return record != nullptr && record->valid;
}

void setParameters(int id, bool blur, bool clamp)
{
	Record* record = find(id);
	if (record == nullptr)
		return;
	record->blur = blur;
	record->clamp = clamp;
	if (record->valid)
		applyParameters(*record);
}

void upload(int id, int x, int y, int width, int height, const void* rgba)
{
	Record* record = find(id);
	if (record == nullptr || !record->valid || rgba == nullptr)
		return;
	if (width <= 0 || height <= 0 || x < 0 || y < 0)
		return;
	// Clip against the *logical* image, not the padded GPU size: a caller
	// passing the padded extent would walk off the tile grid.
	if (x + width > record->logicalW || y + height > record->logicalH)
		return;

	const int tilesAcross = record->tex.width / 8;
	std::uint32_t* dst = static_cast<std::uint32_t*>(record->tex.data);
	const std::uint32_t* src = static_cast<const std::uint32_t*>(rgba);

	if (record->downshift == 0)
	{
		// The common case: the stored image is the uploaded image, one word
		// per texel straight into its Morton slot. Rows are stored in the
		// file's own order: this target renders the Minecraft 3DS Edition
		// asset orientation natively (the official ecosystem's art is
		// stored upside down relative to Java -- that is the convention
		// the white-button/sub-rect mirroring reports attributed to the
		// sampler), so packs sourced from MC-3DS work unmodified here --
		// except their skins: MC-3DS Edition rotates those 180 degrees
		// against its own player model, while the sampler here mirrors
		// rows and never columns and the model keeps the Java UV layout,
		// so the X half has to be stripped (scripts/pak_flip_mc3ds.py
		// --all for the whole-pack recipe, --xflip for skins alone).
		// The script converts between the two conventions when a pack
		// must be shared with the other platforms.
		//
		// Full-image uploads carry the file's own row order, which matches
		// the sampler convention (V=0 reads the last stored row; see the
		// texcoord block of DsShader.v.pica). A SUB-rectangle instead
		// carries generated content in Java-orientation rows and addressed
		// by its Java Y -- animated TextureFX tiles (water, lava, portal,
		// fire, watch, compass) are produced that way -- so it must be
		// stored at the vertically mirrored row: without the mirror every
		// animated tile landed flipped and displaced by half the atlas,
		// mixing contents between neighbouring animation slots (lava
		// frames showing portal texels, the watch needle never updating).
		const bool partial = !(x == 0 && y == 0 &&
			width == record->logicalW && height == record->logicalH);
		for (int row = 0; row < height; ++row)
		{
			const int dstY = partial ? record->logicalH - 1 - (y + row)
			                         : (y + row);
			const std::uint32_t tileRow = static_cast<std::uint32_t>(dstY / 8);
			const std::uint32_t inTileY = static_cast<std::uint32_t>(dstY & 7);
			const std::uint32_t* srcRow =
				src + static_cast<std::size_t>(row) * static_cast<std::size_t>(width);
			for (int col = 0; col < width; ++col)
			{
				const int dstX = x + col;
				const std::uint32_t tileCol = static_cast<std::uint32_t>(dstX / 8);
				const std::uint32_t inTile = mortonInTile(static_cast<std::uint32_t>(dstX & 7),
				                                         inTileY);
				const std::size_t word =
					(static_cast<std::size_t>(tileRow * tilesAcross + tileCol) << 6) + inTile;
				// Game word (A<<24)|(B<<16)|(G<<8)|R -> native (R<<24)|(G<<16)|(B<<8)|A.
				// __builtin_bswap32 is the single REV instruction on the ARM11.
				dst[word] = __builtin_bswap32(srcRow[col]);
			}
		}
		// CPU-written texels are not visible to the PICA200 until the data
		// cache is flushed (see flushAfterWrite).
		flushAfterWrite(*record);
		return;
	}

	// The >1024-texel downscale path (see allocate): every stored texel is
	// the average of its 2^downshift x 2^downshift source block, intersected
	// with this upload's rect -- so a full-image upload averages whole
	// blocks while an animated sub-rect update averages only the samples it
	// actually carries. Blocks that fall partly outside the logical image
	// average fewer samples rather than reading past the caller's buffer.
	const int shift = record->downshift;
	const int block = 1 << shift;
	const bool partialRegion = !(x == 0 && y == 0 &&
		width == record->logicalW && height == record->logicalH);
	const int dstX0 = x >> shift;
	const int dstY0 = y >> shift;
	const int dstX1 = (x + width + block - 1) >> shift;
	const int dstY1 = (y + height + block - 1) >> shift;
	for (int dstY = dstY0; dstY < dstY1 && dstY < record->storedH; ++dstY)
	{
		// Partial uploads carry Java-oriented rows (see the common path
		// above): store them at the vertically mirrored destination row,
		// in downscaled coordinates.
		const int storedY = partialRegion ? record->storedH - 1 - dstY : dstY;
		const std::uint32_t tileRow = static_cast<std::uint32_t>(storedY / 8);
		const std::uint32_t inTileY = static_cast<std::uint32_t>(storedY & 7);
		for (int dstX = dstX0; dstX < dstX1 && dstX < record->storedW; ++dstX)
		{
			// The source block this stored texel represents, clipped to the
			// uploaded rect: rows [max(y, dstY<<shift), min(y+height, ...)).
			const int blockY0 = std::max(y, dstY << shift);
			const int blockY1 = std::min(y + height, (dstY << shift) + block);
			const int blockX0 = std::max(x, dstX << shift);
			const int blockX1 = std::min(x + width, (dstX << shift) + block);
			std::uint32_t rSum = 0, gSum = 0, bSum = 0, aSum = 0;
			std::uint32_t samples = 0;
			for (int sy = blockY0; sy < blockY1; ++sy)
			{
				const std::uint32_t* srcRow =
					src + static_cast<std::size_t>(sy - y) * static_cast<std::size_t>(width);
				for (int sx = blockX0; sx < blockX1; ++sx)
				{
					// Same channel split as the game's byte order: the word is
					// (A<<24)|(B<<16)|(G<<8)|R.
					const std::uint32_t word = srcRow[sx - x];
					rSum += word & 0xFFu;
					gSum += (word >> 8) & 0xFFu;
					bSum += (word >> 16) & 0xFFu;
					aSum += (word >> 24) & 0xFFu;
					++samples;
				}
			}
			if (samples == 0)
				continue;
			const std::uint32_t r = rSum / samples;
			const std::uint32_t g = gSum / samples;
			const std::uint32_t b = bSum / samples;
			const std::uint32_t a = aSum / samples;
			const std::uint32_t tileCol = static_cast<std::uint32_t>(dstX / 8);
			const std::uint32_t inTile = mortonInTile(static_cast<std::uint32_t>(dstX & 7),
			                                         inTileY);
			const std::size_t word =
				(static_cast<std::size_t>(tileRow * tilesAcross + tileCol) << 6) + inTile;
			dst[word] = __builtin_bswap32((a << 24) | (b << 16) | (g << 8) | r);
		}
	}
	flushAfterWrite(*record);
}

void bind(int id, float (&uvScale)[2])
{
	uvScale[0] = 1.0f;
	uvScale[1] = 1.0f;
	Record* record = id > 0 ? find(id) : nullptr;
	if (record != nullptr && record->valid)
	{
		C3D_TexBind(0, &record->tex);
		// The shader folds this into every UV so a non-power-of-two image (or
		// a >1024 one stored halved) keeps addressing real content inside its
		// power-of-two padding. The ratio is stored-to-padded, NOT
		// logical-to-padded: a downscaled atlas already covers the whole 0..1
		// UV space at its own resolution.
		uvScale[0] = static_cast<float>(record->storedW) /
		             static_cast<float>(record->tex.width);
		uvScale[1] = static_cast<float>(record->storedH) /
		             static_cast<float>(record->tex.height);
		return;
	}
	ensureFallback();
	if (s_fallbackReady)
		C3D_TexBind(0, &s_fallback);
}

} // namespace texture
} // namespace ds
