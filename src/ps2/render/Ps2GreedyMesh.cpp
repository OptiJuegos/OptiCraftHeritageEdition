#include "ps2/render/Ps2GreedyMesh.h"

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2Tuning.h"

#include "net/minecraft/src/Block.h"
#include "net/minecraft/src/BlockGrass.h"
#include "net/minecraft/src/BlockLeaves.h"
#include "net/minecraft/src/Chunk.h"
#include "net/minecraft/src/ChunkCache.h"
#include "net/minecraft/src/ExtendedBlockStorage.h"
#include "net/minecraft/src/Tessellator.h"
#include "ps2/render/Ps2BlockRenderInfo.h"
#include "ps2/render/Ps2CaptureLayout.h"
#include "ps2/render/Ps2RenderLighting.h"
#include "ps2/render/Ps2MeshStagingPool.h"

#include <cstdint>
#include <cstring>

namespace
{
struct FaceKey
{
	int_t texture;
	int_t brightness;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	bool valid;
};

struct SectionReader
{
	ChunkCache &cache;
	const ExtendedBlockStorage *section;
	const std::vector<byte_t> *blockLsb;
	int_t originX;
	int_t originY;
	int_t originZ;

	int_t getBlockId(int_t x, int_t y, int_t z) const
	{
		if (section != nullptr &&
			x >= originX && x < originX + 16 &&
			y >= originY && y < originY + 16 &&
			z >= originZ && z < originZ + 16)
		{
			const int_t localX = x - originX;
			const int_t localY = y - originY;
			const int_t localZ = z - originZ;
			if (blockLsb != nullptr)
			{
				const std::size_t index = static_cast<std::size_t>((localY << 8) | (localZ << 4) | localX);
				return (*blockLsb)[index] & 0xff;
			}
			return section->getExtBlockID(localX, localY, localZ);
		}
		return cache.ChunkCache::getBlockId(x, y, z);
	}

	bool isOpaque(int_t x, int_t y, int_t z) const
	{
		if (y >= 0 && y < Chunk::WORLD_HEIGHT &&
			(x < originX || x >= originX + 16 || z < originZ || z >= originZ + 16) &&
			!cache.hasResidentChunkAtBlock(x, z))
			return true;

		const int_t id = getBlockId(x, y, z);
		if (id <= 0 || id >= Block::BLOCK_REGISTRY_SIZE)
			return false;

		Block *block = Block::blocksList[id];
		if (block == nullptr)
			return false;
		if (Block::staticOpaqueCubeLookupSafe[id])
			return Block::opaqueCubeLookup[id];
		return block->isOpaqueCube();
	}
};

static int_t sectionBlockIndex(int_t localX, int_t localY, int_t localZ)
{
	return (localY << 8) | (localZ << 4) | localX;
}

static int_t sectionRowIndex(int_t localY, int_t localZ)
{
	return (localY << 4) | localZ;
}

static bool isOpaqueBlockId(int_t id)
{
	if (id <= 0 || id >= Block::BLOCK_REGISTRY_SIZE)
		return false;

	Block *block = Block::blocksList[id];
	if (block == nullptr)
		return false;
	if (Block::staticOpaqueCubeLookupSafe[id])
		return Block::opaqueCubeLookup[id];
	return block->isOpaqueCube();
}

static bool isBoundaryOpaque(ChunkCache &cache, int_t originX, int_t originZ,
	                         int_t x, int_t y, int_t z)
{
	if (y >= 0 && y < Chunk::WORLD_HEIGHT &&
		(x < originX || x >= originX + 16 || z < originZ || z >= originZ + 16) &&
		!cache.hasResidentChunkAtBlock(x, z))
	{
		return true;
	}

	return isOpaqueBlockId(cache.ChunkCache::getBlockId(x, y, z));
}

static bool cacheHasVisibleFace(const Ps2MeshSectionCache &sectionCache, int_t face,
	                            int_t x, int_t y, int_t z)
{
	const int_t localX = x - sectionCache.originX;
	const int_t localY = y - sectionCache.originY;
	const int_t localZ = z - sectionCache.originZ;
	if (localX < 0 || localX >= 16 || localY < 0 || localY >= 16 || localZ < 0 || localZ >= 16)
		return false;

	const std::uint16_t bit = static_cast<std::uint16_t>(1u << localX);
	return (sectionCache.visibleRows[static_cast<std::size_t>(face)]
		[static_cast<std::size_t>(sectionRowIndex(localY, localZ))] & bit) != 0;
}

static bool sameKey(const FaceKey &a, const FaceKey &b)
{
	return a.valid && b.valid && a.texture == b.texture &&
	       a.brightness == b.brightness &&
	       a.red == b.red && a.green == b.green && a.blue == b.blue;
}

static unsigned char quantizeColor(float value)
{
	int_t component = (int_t)(value * 255.0f);
	if (component < 0)
		component = 0;
	else if (component > 255)
		component = 255;
	return (unsigned char)component;
}

static FaceKey makeFaceKey(SectionReader &reader, int_t face,
	                       int_t x, int_t y, int_t z)
{
	FaceKey key = { 0, 0, 0, 0, 0, false };
	const int_t id = reader.getBlockId(x, y, z);
	if (id <= 0 || id >= Block::BLOCK_REGISTRY_SIZE)
		return key;

	Block *block = Block::blocksList[id];
	if (!ps2_is_greedy_cube(block))
		return key;

	int_t nx = x;
	int_t ny = y;
	int_t nz = z;
	switch (face)
	{
		case 0: --ny; break;
		case 1: ++ny; break;
		case 2: --nz; break;
		case 3: ++nz; break;
		case 4: --nx; break;
		case 5: ++nx; break;
		default: return key;
	}

	if (reader.isOpaque(nx, ny, nz))
		return key;

	const Ps2BlockRenderInfo &renderInfo = ps2GetBlockRenderInfo(id);
	int_t texture = -1;
	if (renderInfo.staticTextureBySide)
	{
		texture = renderInfo.textureBySide[static_cast<std::size_t>(face)];
	}
	else
	{
		texture = block->getBlockTexture(&reader.cache, x, y, z, face);
	}
	if (texture < 0)
		return key;

	const int_t tint = renderInfo.defaultWhiteColorMultiplier
		? 0xffffff
		: block->colorMultiplier(&reader.cache, x, y, z);
	const float tintR = (float)(tint >> 16 & 0xff) / 255.0f;
	const float tintG = (float)(tint >> 8 & 0xff) / 255.0f;
	const float tintB = (float)(tint & 0xff) / 255.0f;
	static const float directionalShade[6] =
	{
		0.5f, 1.0f, 0.8f, 0.8f, 0.6f, 0.6f
	};
	const float shade = directionalShade[face];

	key.texture = texture;
	key.brightness = block->getMixedBrightnessForBlock(&reader.cache, nx, ny, nz);
	key.red = quantizeColor(shade * tintR);
	key.green = quantizeColor(shade * tintG);
	key.blue = quantizeColor(shade * tintB);
	key.valid = true;
	return key;
}

static FaceKey makeFaceKeyCached(SectionReader &reader, const Ps2MeshSectionCache &sectionCache,
	                             int_t face, int_t x, int_t y, int_t z)
{
	FaceKey key = { 0, 0, 0, 0, 0, false };
	if (!cacheHasVisibleFace(sectionCache, face, x, y, z))
		return key;

	const int_t localX = x - sectionCache.originX;
	const int_t localY = y - sectionCache.originY;
	const int_t localZ = z - sectionCache.originZ;
	const int_t id = sectionCache.blockIds[static_cast<std::size_t>(sectionBlockIndex(localX, localY, localZ))];
	if (id <= 0 || id >= Block::BLOCK_REGISTRY_SIZE)
		return key;

	Block *block = Block::blocksList[id];
	if (block == nullptr)
		return key;

	int_t nx = x;
	int_t ny = y;
	int_t nz = z;
	switch (face)
	{
		case 0: --ny; break;
		case 1: ++ny; break;
		case 2: --nz; break;
		case 3: ++nz; break;
		case 4: --nx; break;
		case 5: ++nx; break;
		default: return key;
	}

	const Ps2BlockRenderInfo &renderInfo = ps2GetBlockRenderInfo(id);
	int_t texture = -1;
	if (renderInfo.staticTextureBySide)
		texture = renderInfo.textureBySide[static_cast<std::size_t>(face)];
	else
		texture = block->getBlockTexture(&reader.cache, x, y, z, face);
	if (texture < 0)
		return key;

	const int_t tint = renderInfo.defaultWhiteColorMultiplier
		? 0xffffff
		: static_cast<int_t>(sectionCache.colorMultipliers[
			static_cast<std::size_t>(sectionBlockIndex(localX, localY, localZ))]);
	const float tintR = (float)(tint >> 16 & 0xff) / 255.0f;
	const float tintG = (float)(tint >> 8 & 0xff) / 255.0f;
	const float tintB = (float)(tint & 0xff) / 255.0f;
	static const float directionalShade[6] =
	{
		0.5f, 1.0f, 0.8f, 0.8f, 0.6f, 0.6f
	};
	const float shade = directionalShade[face];

	key.texture = texture;
	key.brightness = block->getMixedBrightnessForBlock(&reader.cache, nx, ny, nz);
	key.red = quantizeColor(shade * tintR);
	key.green = quantizeColor(shade * tintG);
	key.blue = quantizeColor(shade * tintB);
	key.valid = true;
	return key;
}

static void addVertex(Tessellator &t, tess_coord_t x, tess_coord_t y, tess_coord_t z,
	                  tess_coord_t u, tess_coord_t v)
{
	t.addVertexWithUV(x, y, z, u, v);
}

static void emitQuad(int_t face, const FaceKey &key,
	                 int_t x, int_t y, int_t z,
	                 int_t width, int_t height)
{
	Tessellator &t = Tessellator::instance;
	t.setBrightness(key.brightness);
	t.setColorOpaque((int_t)key.red, (int_t)key.green, (int_t)key.blue);

	// Four divisions plus the surrounding arithmetic, once per emitted quad, on
	// the emitter that produces most of the opaque terrain: every full cube is
	// merged here rather than drawn by RenderBlocks. As doubles all of it is
	// software floating point on the EE -- see PLATFORM_FLOAT_VERTEX_MATH. The
	// 0.01 guard is a typed constant because its float and double values differ.
	const tess_coord_t uvGuard = (tess_coord_t)0.01;
	const tess_coord_t atlasU = (tess_coord_t)((key.texture & 0xf) << 4);
	const tess_coord_t atlasV = (tess_coord_t)(key.texture & 0xf0);
	const tess_coord_t u0 = atlasU / 256.0f;
	const tess_coord_t v0 = atlasV / 256.0f;
	const tess_coord_t u1 = (atlasU + (tess_coord_t)(width * 16) - uvGuard) / 256.0f;
	const tess_coord_t v1 = (atlasV + (tess_coord_t)(height * 16) - uvGuard) / 256.0f;

	const tess_coord_t fx = (tess_coord_t)x;
	const tess_coord_t fy = (tess_coord_t)y;
	const tess_coord_t fz = (tess_coord_t)z;
	const tess_coord_t fw = (tess_coord_t)width;
	const tess_coord_t fh = (tess_coord_t)height;
	if (t.addAxisAlignedFaceWithUVFast(face, fx, fy, fz, fw, fh, u0, u1, v0, v1))
		return;

	switch (face)
	{
		case 0: // Y-, width=X, height=Z
			addVertex(t, fx,      fy, fz + fh, u0, v1);
			addVertex(t, fx,      fy, fz,      u0, v0);
			addVertex(t, fx + fw, fy, fz,      u1, v0);
			addVertex(t, fx + fw, fy, fz + fh, u1, v1);
			break;
		case 1: // Y+, width=X, height=Z
			addVertex(t, fx + fw, fy + 1.0f, fz + fh, u1, v1);
			addVertex(t, fx + fw, fy + 1.0f, fz,      u1, v0);
			addVertex(t, fx,      fy + 1.0f, fz,      u0, v0);
			addVertex(t, fx,      fy + 1.0f, fz + fh, u0, v1);
			break;
		case 2: // Z-, width=X, height=Y; texture U is mirrored
			addVertex(t, fx,      fy + fh, fz, u1, v0);
			addVertex(t, fx + fw, fy + fh, fz, u0, v0);
			addVertex(t, fx + fw, fy,      fz, u0, v1);
			addVertex(t, fx,      fy,      fz, u1, v1);
			break;
		case 3: // Z+, width=X, height=Y
			addVertex(t, fx,      fy + fh, fz + 1.0f, u0, v0);
			addVertex(t, fx,      fy,      fz + 1.0f, u0, v1);
			addVertex(t, fx + fw, fy,      fz + 1.0f, u1, v1);
			addVertex(t, fx + fw, fy + fh, fz + 1.0f, u1, v0);
			break;
		case 4: // X-, width=Z, height=Y
			addVertex(t, fx, fy + fh, fz + fw, u1, v0);
			addVertex(t, fx, fy + fh, fz,      u0, v0);
			addVertex(t, fx, fy,      fz,      u0, v1);
			addVertex(t, fx, fy,      fz + fw, u1, v1);
			break;
		case 5: // X+, width=Z, height=Y; texture U is mirrored
			addVertex(t, fx + 1.0f, fy,      fz + fw, u0, v1);
			addVertex(t, fx + 1.0f, fy,      fz,      u1, v1);
			addVertex(t, fx + 1.0f, fy + fh, fz,      u1, v0);
			addVertex(t, fx + 1.0f, fy + fh, fz + fw, u0, v0);
			break;
	}
}

static int_t floatBits(float value)
{
	static_assert(sizeof(float) == sizeof(int_t), "PS2 terrain capture requires 32-bit float slots");
	int_t bits = 0;
	std::memcpy(&bits, &value, sizeof(bits));
	return bits;
}

static void writeRawVertex(int_t *dst, float x, float y, float z,
	                       float u, float v, std::uint32_t color)
{
	dst[0] = floatBits(x);
	dst[1] = floatBits(y);
	dst[2] = floatBits(z);
	dst[3] = floatBits(u);
	dst[4] = floatBits(v);
	dst[5] = static_cast<int_t>(color);
}

static void reserveRawQuad(std::vector<int_t> &raw)
{
	const std::size_t required = raw.size() + 4u * Ps2CaptureLayout::Slots;
	if (required <= raw.capacity())
		return;

	const std::size_t growthInts = (16u * 1024u) / sizeof(int_t);
	const std::size_t target = ((required + growthInts - 1u) / growthInts) * growthInts;
	raw.reserve(target);
}

static void emitRawQuad(const Ps2GreedyRawTarget &target, int_t face, const FaceKey &key,
	                    int_t x, int_t y, int_t z, int_t width, int_t height)
{
	if (target.raw == nullptr)
		return;

	const float uvGuard = 0.01f;
	const float atlasU = (float)((key.texture & 0xf) << 4);
	const float atlasV = (float)(key.texture & 0xf0);
	const float invAtlas = 1.0f / 256.0f;
	const float u0 = atlasU * invAtlas;
	const float v0 = atlasV * invAtlas;
	const float u1 = (atlasU + (float)(width * 16) - uvGuard) * invAtlas;
	const float v1 = (atlasV + (float)(height * 16) - uvGuard) * invAtlas;
	const float fx = (float)(x - target.originX);
	const float fy = (float)(y - target.originY);
	const float fz = (float)(z - target.originZ);
	const float fw = (float)width;
	const float fh = (float)height;
	const std::uint32_t baseColor = 0xff000000u |
		((std::uint32_t)key.blue << 16) |
		((std::uint32_t)key.green << 8) |
		(std::uint32_t)key.red;
	const std::uint32_t color = ps2_lighting_apply_packed_brightness(baseColor, key.brightness);

	std::vector<int_t> &raw = *target.raw;
	reserveRawQuad(raw);
	const std::size_t base = raw.size();
	raw.resize(base + 4u * Ps2CaptureLayout::Slots);
	int_t *dst = raw.data() + base;

	switch (face)
	{
		case 0:
			writeRawVertex(dst + 0u * Ps2CaptureLayout::Slots, fx,      fy, fz + fh, u0, v1, color);
			writeRawVertex(dst + 1u * Ps2CaptureLayout::Slots, fx,      fy, fz,      u0, v0, color);
			writeRawVertex(dst + 2u * Ps2CaptureLayout::Slots, fx + fw, fy, fz,      u1, v0, color);
			writeRawVertex(dst + 3u * Ps2CaptureLayout::Slots, fx + fw, fy, fz + fh, u1, v1, color);
			break;
		case 1:
			writeRawVertex(dst + 0u * Ps2CaptureLayout::Slots, fx + fw, fy + 1.0f, fz + fh, u1, v1, color);
			writeRawVertex(dst + 1u * Ps2CaptureLayout::Slots, fx + fw, fy + 1.0f, fz,      u1, v0, color);
			writeRawVertex(dst + 2u * Ps2CaptureLayout::Slots, fx,      fy + 1.0f, fz,      u0, v0, color);
			writeRawVertex(dst + 3u * Ps2CaptureLayout::Slots, fx,      fy + 1.0f, fz + fh, u0, v1, color);
			break;
		case 2:
			writeRawVertex(dst + 0u * Ps2CaptureLayout::Slots, fx,      fy + fh, fz, u1, v0, color);
			writeRawVertex(dst + 1u * Ps2CaptureLayout::Slots, fx + fw, fy + fh, fz, u0, v0, color);
			writeRawVertex(dst + 2u * Ps2CaptureLayout::Slots, fx + fw, fy,      fz, u0, v1, color);
			writeRawVertex(dst + 3u * Ps2CaptureLayout::Slots, fx,      fy,      fz, u1, v1, color);
			break;
		case 3:
			writeRawVertex(dst + 0u * Ps2CaptureLayout::Slots, fx,      fy + fh, fz + 1.0f, u0, v0, color);
			writeRawVertex(dst + 1u * Ps2CaptureLayout::Slots, fx,      fy,      fz + 1.0f, u0, v1, color);
			writeRawVertex(dst + 2u * Ps2CaptureLayout::Slots, fx + fw, fy,      fz + 1.0f, u1, v1, color);
			writeRawVertex(dst + 3u * Ps2CaptureLayout::Slots, fx + fw, fy + fh, fz + 1.0f, u1, v0, color);
			break;
		case 4:
			writeRawVertex(dst + 0u * Ps2CaptureLayout::Slots, fx, fy + fh, fz + fw, u1, v0, color);
			writeRawVertex(dst + 1u * Ps2CaptureLayout::Slots, fx, fy + fh, fz,      u0, v0, color);
			writeRawVertex(dst + 2u * Ps2CaptureLayout::Slots, fx, fy,      fz,      u0, v1, color);
			writeRawVertex(dst + 3u * Ps2CaptureLayout::Slots, fx, fy,      fz + fw, u1, v1, color);
			break;
		case 5:
			writeRawVertex(dst + 0u * Ps2CaptureLayout::Slots, fx + 1.0f, fy,      fz + fw, u0, v1, color);
			writeRawVertex(dst + 1u * Ps2CaptureLayout::Slots, fx + 1.0f, fy,      fz,      u1, v1, color);
			writeRawVertex(dst + 2u * Ps2CaptureLayout::Slots, fx + 1.0f, fy + fh, fz,      u1, v0, color);
			writeRawVertex(dst + 3u * Ps2CaptureLayout::Slots, fx + 1.0f, fy + fh, fz + fw, u0, v0, color);
			break;
		default:
			raw.resize(base);
			break;
	}
}

static void faceCellCoordinates(int_t face, int_t slice, int_t u, int_t v,
	                            int_t x0, int_t y0, int_t z0,
	                            int_t &x, int_t &y, int_t &z)
{
	if (face <= 1)
	{
		x = x0 + u;
		y = y0 + slice;
		z = z0 + v;
	}
	else if (face <= 3)
	{
		x = x0 + u;
		y = y0 + v;
		z = z0 + slice;
	}
	else
	{
		x = x0 + slice;
		y = y0 + v;
		z = z0 + u;
	}
}
}

bool ps2_is_greedy_cube(Block *block)
{
	if (block == nullptr || block->blockID < 0 || block->blockID >= Block::BLOCK_REGISTRY_SIZE)
		return false;

	// Grass uses a side overlay/tint path and stays in RenderBlocks. Leaves are
	// different: in Fast graphics BlockLeaves becomes a full opaque cube and
	// already culls leaf-to-leaf faces. Let that mode use the same conservative
	// greedy path as terrain so dense canopies emit fewer quads and rebuild
	// faster. Fancy leaves remain non-opaque and continue through RenderBlocks.
	if (block == static_cast<Block *>(Block::grass) ||
		Block::isBlockContainer[block->blockID])
	{
		return false;
	}

	if (block == static_cast<Block *>(Block::leaves))
		return block->isOpaqueCube();

	return ps2GetBlockRenderInfo(block->blockID).simpleOpaqueCube;
}

bool ps2_prepare_greedy_section_cache(ChunkCache &cc, int_t originX, int_t originY, int_t originZ,
                                      Ps2MeshSectionCache &sectionCache)
{
	const ExtendedBlockStorage *section = cc.getResidentBlockStorageAt(originX, originY, originZ);
	const std::vector<byte_t> *blockLsb = section != nullptr && section->getBlockMSBArray() == nullptr
		? &section->func_48692_g()
		: nullptr;

	sectionCache.originX = originX;
	sectionCache.originY = originY;
	sectionCache.originZ = originZ;
	sectionCache.valid = false;
	sectionCache.greedyRows.fill(0);
	sectionCache.occluderRows.fill(0);
	for (std::size_t face = 0; face < sectionCache.visibleRows.size(); ++face)
		sectionCache.visibleRows[face].fill(0);

	for (int_t localY = 0; localY < 16; ++localY)
	{
		for (int_t localZ = 0; localZ < 16; ++localZ)
		{
			std::uint16_t greedyRow = 0;
			std::uint16_t occluderRow = 0;
			for (int_t localX = 0; localX < 16; ++localX)
			{
				const int_t blockIndex = sectionBlockIndex(localX, localY, localZ);
				int_t id = 0;
				if (blockLsb != nullptr && static_cast<std::size_t>(blockIndex) < blockLsb->size())
					id = (*blockLsb)[static_cast<std::size_t>(blockIndex)] & 0xff;
				else if (section != nullptr)
					id = section->getExtBlockID(localX, localY, localZ);
				else
					id = cc.ChunkCache::getBlockId(originX + localX, originY + localY, originZ + localZ);

				sectionCache.blockIds[static_cast<std::size_t>(blockIndex)] = static_cast<std::uint16_t>(id);
				sectionCache.colorMultipliers[static_cast<std::size_t>(blockIndex)] = 0xffffffu;
				if (id <= 0 || id >= Block::BLOCK_REGISTRY_SIZE)
					continue;

				Block *block = Block::blocksList[id];
				if (block == nullptr)
					continue;

				const std::uint16_t bit = static_cast<std::uint16_t>(1u << localX);
				if (ps2_is_greedy_cube(block))
				{
					greedyRow = static_cast<std::uint16_t>(greedyRow | bit);
					const Ps2BlockRenderInfo &renderInfo = ps2GetBlockRenderInfo(id);
					if (!renderInfo.defaultWhiteColorMultiplier)
					{
						sectionCache.colorMultipliers[static_cast<std::size_t>(blockIndex)] =
							static_cast<std::uint32_t>(block->colorMultiplier(
								&cc, originX + localX, originY + localY, originZ + localZ));
					}
				}
				if (isOpaqueBlockId(id))
					occluderRow = static_cast<std::uint16_t>(occluderRow | bit);
			}

			const int_t row = sectionRowIndex(localY, localZ);
			sectionCache.greedyRows[static_cast<std::size_t>(row)] = greedyRow;
			sectionCache.occluderRows[static_cast<std::size_t>(row)] = occluderRow;
		}
	}

	for (int_t localY = 0; localY < 16; ++localY)
	{
		for (int_t localZ = 0; localZ < 16; ++localZ)
		{
			const int_t row = sectionRowIndex(localY, localZ);
			const std::uint16_t greedy = sectionCache.greedyRows[static_cast<std::size_t>(row)];
			const std::uint16_t occluders = sectionCache.occluderRows[static_cast<std::size_t>(row)];

			std::uint16_t westVisible = static_cast<std::uint16_t>(greedy & ~static_cast<std::uint16_t>(occluders << 1));
			std::uint16_t eastVisible = static_cast<std::uint16_t>(greedy & ~static_cast<std::uint16_t>(occluders >> 1));
			const int_t worldY = originY + localY;
			const int_t worldZ = originZ + localZ;
			if (isBoundaryOpaque(cc, originX, originZ, originX - 1, worldY, worldZ))
				westVisible = static_cast<std::uint16_t>(westVisible & ~std::uint16_t{1});
			if (isBoundaryOpaque(cc, originX, originZ, originX + 16, worldY, worldZ))
				eastVisible = static_cast<std::uint16_t>(eastVisible & ~static_cast<std::uint16_t>(1u << 15));
			sectionCache.visibleRows[4][static_cast<std::size_t>(row)] = westVisible;
			sectionCache.visibleRows[5][static_cast<std::size_t>(row)] = eastVisible;

			std::uint16_t northOccluders = 0;
			if (localZ > 0)
				northOccluders = sectionCache.occluderRows[static_cast<std::size_t>(row - 1)];
			else
			{
				for (int_t localX = 0; localX < 16; ++localX)
					if (isBoundaryOpaque(cc, originX, originZ, originX + localX, worldY, originZ - 1))
						northOccluders = static_cast<std::uint16_t>(northOccluders | (1u << localX));
			}
			std::uint16_t southOccluders = 0;
			if (localZ < 15)
				southOccluders = sectionCache.occluderRows[static_cast<std::size_t>(row + 1)];
			else
			{
				for (int_t localX = 0; localX < 16; ++localX)
					if (isBoundaryOpaque(cc, originX, originZ, originX + localX, worldY, originZ + 16))
						southOccluders = static_cast<std::uint16_t>(southOccluders | (1u << localX));
			}
			sectionCache.visibleRows[2][static_cast<std::size_t>(row)] = static_cast<std::uint16_t>(greedy & ~northOccluders);
			sectionCache.visibleRows[3][static_cast<std::size_t>(row)] = static_cast<std::uint16_t>(greedy & ~southOccluders);

			std::uint16_t downOccluders = 0;
			if (localY > 0)
				downOccluders = sectionCache.occluderRows[static_cast<std::size_t>(row - 16)];
			else
			{
				for (int_t localX = 0; localX < 16; ++localX)
					if (isBoundaryOpaque(cc, originX, originZ, originX + localX, originY - 1, worldZ))
						downOccluders = static_cast<std::uint16_t>(downOccluders | (1u << localX));
			}
			std::uint16_t upOccluders = 0;
			if (localY < 15)
				upOccluders = sectionCache.occluderRows[static_cast<std::size_t>(row + 16)];
			else
			{
				for (int_t localX = 0; localX < 16; ++localX)
					if (isBoundaryOpaque(cc, originX, originZ, originX + localX, originY + 16, worldZ))
						upOccluders = static_cast<std::uint16_t>(upOccluders | (1u << localX));
			}
			sectionCache.visibleRows[0][static_cast<std::size_t>(row)] = static_cast<std::uint16_t>(greedy & ~downOccluders);
			sectionCache.visibleRows[1][static_cast<std::size_t>(row)] = static_cast<std::uint16_t>(greedy & ~upOccluders);
		}
	}

	sectionCache.valid = true;
	return true;
}

static int_t greedyMeshFaceImpl(ChunkCache &cc, int face,
	                            int x0, int y0, int z0,
	                            int x1, int y1, int z1,
	                            const Ps2GreedyRawTarget *rawTarget,
	                            const Ps2MeshSectionCache *sectionCache)
{
	if (face < 0 || face >= PS2_GREEDY_FACE_COUNT)
		return 0;

	const int_t sliceCount = face <= 1 ? y1 - y0 : (face <= 3 ? z1 - z0 : x1 - x0);
	const int_t uCount = face <= 3 ? x1 - x0 : z1 - z0;
	const int_t vCount = face <= 1 ? z1 - z0 : y1 - y0;
	if (sliceCount <= 0 || sliceCount > 16 || uCount <= 0 || uCount > 16 ||
	    vCount <= 0 || vCount > 16)
		return 0;

	FaceKey mask[16 * 16];
	bool used[16 * 16];
	int_t emittedVertices = 0;
	const int_t configuredMerge = PS2_GREEDY_MAX_MERGE;
	const int_t maxMerge = configuredMerge < 1 ? 1 :
	                       (configuredMerge > 16 ? 16 : configuredMerge);
	const int_t sectionX = x0 & ~15;
	const int_t sectionY = y0 & ~15;
	const int_t sectionZ = z0 & ~15;
	const ExtendedBlockStorage *section = cc.getResidentBlockStorageAt(sectionX, sectionY, sectionZ);
	const std::vector<byte_t> *blockLsb = section != nullptr && section->getBlockMSBArray() == nullptr
		? &section->func_48692_g()
		: nullptr;
	SectionReader reader = {
		cc,
		section,
		blockLsb,
		sectionX,
		sectionY,
		sectionZ
	};

	for (int_t slice = 0; slice < sliceCount; ++slice)
	{
		for (int_t index = 0; index < 16 * 16; ++index)
		{
			mask[index].valid = false;
			used[index] = false;
		}

		for (int_t v = 0; v < vCount; ++v)
		{
			for (int_t u = 0; u < uCount; ++u)
			{
				int_t x, y, z;
				faceCellCoordinates(face, slice, u, v, x0, y0, z0, x, y, z);
				if (sectionCache != nullptr)
					mask[v * 16 + u] = makeFaceKeyCached(reader, *sectionCache, face, x, y, z);
				else
					mask[v * 16 + u] = makeFaceKey(reader, face, x, y, z);
			}
		}

		for (int_t v = 0; v < vCount; ++v)
		{
			for (int_t u = 0; u < uCount; ++u)
			{
				const int_t first = v * 16 + u;
				if (used[first] || !mask[first].valid)
					continue;

				int_t width = 1;
				while (width < maxMerge && u + width < uCount)
				{
					const int_t candidate = v * 16 + u + width;
					if (used[candidate] || !sameKey(mask[first], mask[candidate]))
						break;
					++width;
				}

				int_t height = 1;
				while (height < maxMerge && v + height < vCount)
				{
					bool rowMatches = true;
					for (int_t du = 0; du < width; ++du)
					{
						const int_t candidate = (v + height) * 16 + u + du;
						if (used[candidate] || !sameKey(mask[first], mask[candidate]))
						{
							rowMatches = false;
							break;
						}
					}
					if (!rowMatches)
						break;
					++height;
				}

				for (int_t dv = 0; dv < height; ++dv)
					for (int_t du = 0; du < width; ++du)
						used[(v + dv) * 16 + u + du] = true;

				int_t x, y, z;
				faceCellCoordinates(face, slice, u, v, x0, y0, z0, x, y, z);
				if (rawTarget != nullptr)
					emitRawQuad(*rawTarget, face, mask[first], x, y, z, width, height);
				else
					emitQuad(face, mask[first], x, y, z, width, height);
				emittedVertices += 4;
			}
		}
	}

	return emittedVertices;
}

bool ps2_greedy_mesh_face(ChunkCache &cc, int face,
	                      int x0, int y0, int z0,
	                      int x1, int y1, int z1)
{
	return greedyMeshFaceImpl(cc, face, x0, y0, z0, x1, y1, z1, nullptr, nullptr) > 0;
}

int_t ps2_greedy_mesh_face_raw(ChunkCache &cc, int face,
	                           int x0, int y0, int z0,
	                           int x1, int y1, int z1,
	                           const Ps2GreedyRawTarget &target)
{
	if (target.raw == nullptr)
		return 0;
	return greedyMeshFaceImpl(cc, face, x0, y0, z0, x1, y1, z1, &target, nullptr);
}

int_t ps2_greedy_mesh_face_raw(ChunkCache &cc, int face,
	                           int x0, int y0, int z0,
	                           int x1, int y1, int z1,
	                           const Ps2GreedyRawTarget &target,
	                           const Ps2MeshSectionCache &sectionCache)
{
	if (target.raw == nullptr || !sectionCache.valid)
		return 0;
	return greedyMeshFaceImpl(cc, face, x0, y0, z0, x1, y1, z1, &target, &sectionCache);
}

#endif // PS2_PLATFORM
