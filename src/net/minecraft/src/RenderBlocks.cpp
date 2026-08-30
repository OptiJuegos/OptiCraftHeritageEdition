#include "RenderBlocks.h"
#include "ChestItemRenderHelper.h"
#include "java/Arithmetic.h"

#include "Block.h"
#include "BlockBed.h"
#include "BlockDoor.h"
#include "BlockFire.h"
#include "BlockFence.h"
#include "BlockFluid.h"
#include "BlockGrass.h"
#include "BlockPistonBase.h"
#include "BlockPistonExtension.h"
#include "BlockRail.h"
#include "BlockRedstoneRepeater.h"
#include "BlockRedstoneWire.h"
#include "BlockTallGrass.h"
#include "ChunkCache.h"
#include "Config.h"
#include "CustomColorizer.h"
#include "NaturalProperties.h"
#include "NaturalTextures.h"
#include "ConnectedTextures.h"
#include "EntityRenderer.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "MathHelper.h"
#include "ModelBed.h"
#include "Tessellator.h"
#include "Vec3D.h"
#include "World.h"
#include "client/Minecraft.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformTuning.h"

#if PLATFORM_REUSE_AO_SCRATCH
#include <cstdint>
#include <cstring>
#endif

bool RenderBlocks::fancyGrass = true;

// Epsilon that pulls a face's far UV edge back inside its atlas cell.
//
// It has to be a typed constant rather than a literal: 0.01 written as a float
// and 0.01 written as a double are different numbers, so a bare literal would
// silently change the value the desktop double path computes when
// tess_coord_t is float. 16.0 and 256.0 are exactly representable in both and
// stay as plain literals. See PLATFORM_FLOAT_VERTEX_MATH.
static const tess_coord_t kAtlasUvGuard = (tess_coord_t)0.01;

#if PLATFORM_REUSE_AO_SCRATCH
namespace
{
	struct Ps2AoCacheScratch
	{
		float brightness[27];
		int_t packedBrightness[27];
		int_t blockId[27];
		std::uint16_t brightnessStamp[27];
		std::uint16_t packedBrightnessStamp[27];
		std::uint16_t blockIdStamp[27];
		std::uint16_t generation;

		Ps2AoCacheScratch() : generation(0)
		{
			std::memset(brightnessStamp, 0, sizeof(brightnessStamp));
			std::memset(packedBrightnessStamp, 0, sizeof(packedBrightnessStamp));
			std::memset(blockIdStamp, 0, sizeof(blockIdStamp));
		}

		std::uint16_t beginGeneration()
		{
			++generation;
			if (generation == 0)
			{
				std::memset(brightnessStamp, 0, sizeof(brightnessStamp));
				std::memset(packedBrightnessStamp, 0, sizeof(packedBrightnessStamp));
				std::memset(blockIdStamp, 0, sizeof(blockIdStamp));
				generation = 1;
			}
			return generation;
		}
	};

	Ps2AoCacheScratch s_ps2AoCacheScratch;
}
#endif

RenderBlocks::RenderBlocks(IBlockAccess *iblockaccess) :
	usedAlphaTestedTexture(false),
	overrideBlockTexture(-1),
	flipTexture(false),
	renderAllFaces(false),
	field_31088_b(true),
	eastFaceRotation(0),
	westFaceRotation(0),
	southFaceRotation(0),
	northFaceRotation(0),
	topFaceRotation(0),
	bottomFaceRotation(0),
	naturalTextureTransformActive(false),
	naturalSavedFlipTexture(false),
	naturalSavedFaceRotation(0),
	naturalTextureTransformSide(0),
	lightingQuality(1),
	blockAccess(iblockaccess),
	blockAccessCache(dynamic_cast<ChunkCache *>(iblockaccess)),
	enableAO(false),
	brightnessTopLeft(0),
	brightnessBottomLeft(0),
	brightnessBottomRight(0),
	brightnessTopRight(0),
	colorRedTopLeft(0.0f),
	colorRedBottomLeft(0.0f),
	colorRedBottomRight(0.0f),
	colorRedTopRight(0.0f),
	colorGreenTopLeft(0.0f),
	colorGreenBottomLeft(0.0f),
	colorGreenBottomRight(0.0f),
	colorGreenTopRight(0.0f),
	colorBlueTopLeft(0.0f),
	colorBlueBottomLeft(0.0f),
	colorBlueBottomRight(0.0f),
	colorBlueTopRight(0.0f)
{
}

RenderBlocks::RenderBlocks() :
	usedAlphaTestedTexture(false),
	overrideBlockTexture(-1),
	flipTexture(false),
	renderAllFaces(false),
	field_31088_b(true),
	eastFaceRotation(0),
	westFaceRotation(0),
	southFaceRotation(0),
	northFaceRotation(0),
	topFaceRotation(0),
	bottomFaceRotation(0),
	naturalTextureTransformActive(false),
	naturalSavedFlipTexture(false),
	naturalSavedFaceRotation(0),
	naturalTextureTransformSide(0),
	lightingQuality(1),
	blockAccess(nullptr),
	blockAccessCache(nullptr),
	enableAO(false),
	brightnessTopLeft(0),
	brightnessBottomLeft(0),
	brightnessBottomRight(0),
	brightnessTopRight(0),
	colorRedTopLeft(0.0f),
	colorRedBottomLeft(0.0f),
	colorRedBottomRight(0.0f),
	colorRedTopRight(0.0f),
	colorGreenTopLeft(0.0f),
	colorGreenBottomLeft(0.0f),
	colorGreenBottomRight(0.0f),
	colorGreenTopRight(0.0f),
	colorBlueTopLeft(0.0f),
	colorBlueBottomLeft(0.0f),
	colorBlueBottomRight(0.0f),
	colorBlueTopRight(0.0f)
{
}

// Devirtualised block reads; see the declarations in RenderBlocks.h for why.
// Defined here rather than in the header so the qualified ChunkCache calls stay
// inside this translation unit, which is the only place that uses them -- and
// which is also where the compiler can inline the branch away.
int_t RenderBlocks::accessGetBlockId(int_t i, int_t j, int_t k)
{
	if (blockAccessCache != nullptr)
		return blockAccessCache->ChunkCache::getBlockId(i, j, k);
	return blockAccess->getBlockId(i, j, k);
}

int_t RenderBlocks::accessGetBlockMetadata(int_t i, int_t j, int_t k)
{
	if (blockAccessCache != nullptr)
		return blockAccessCache->ChunkCache::getBlockMetadata(i, j, k);
	return blockAccess->getBlockMetadata(i, j, k);
}

Material *RenderBlocks::accessGetBlockMaterial(int_t i, int_t j, int_t k)
{
	if (blockAccessCache != nullptr)
		return blockAccessCache->ChunkCache::getBlockMaterial(i, j, k);
	return blockAccess->getBlockMaterial(i, j, k);
}

bool RenderBlocks::accessIsBlockNormalCube(int_t i, int_t j, int_t k)
{
	if (blockAccessCache != nullptr)
		return blockAccessCache->ChunkCache::isBlockNormalCube(i, j, k);
	return blockAccess->isBlockNormalCube(i, j, k);
}

bool RenderBlocks::accessIsBlockOpaqueCube(int_t i, int_t j, int_t k)
{
	if (blockAccessCache != nullptr)
		return blockAccessCache->ChunkCache::isBlockOpaqueCube(i, j, k);
	return blockAccess->isBlockOpaqueCube(i, j, k);
}

bool RenderBlocks::accessIsAirBlock(int_t i, int_t j, int_t k)
{
	if (blockAccessCache != nullptr)
		return blockAccessCache->ChunkCache::isAirBlock(i, j, k);
	return blockAccess->isAirBlock(i, j, k);
}

bool RenderBlocks::shouldRenderFace(Block *block, int_t i, int_t j, int_t k, int_t side)
{
	if (!Block::usesDefaultFaceCullingLookup[block->blockID])
		return block->shouldSideBeRendered(blockAccess, i, j, k, side);

	// Inlined Block::shouldSideBeRendered default body (bevel checks, then
	// neighbour-opacity), routed through the already-devirtualised accessor
	// instead of a second indirect call through blockAccess.
	if (side == 0 && block->minY > 0.0) return true;
	if (side == 1 && block->maxY < 1.0) return true;
	if (side == 2 && block->minZ > 0.0) return true;
	if (side == 3 && block->maxZ < 1.0) return true;
	if (side == 4 && block->minX > 0.0) return true;
	if (side == 5 && block->maxX < 1.0) return true;
#if PLATFORM_CULL_MISSING_CHUNK_BOUNDARY_FACES
	// ChunkCache intentionally reads a missing source column as air so general
	// callers never force generation. For an opaque default-culled cube that
	// would expose an entire temporary chunk wall. Suppress only that case;
	// transparent/special blocks never reach this fast path unchanged.
	if (blockAccessCache != nullptr && Block::opaqueCubeLookup[block->blockID] &&
		!blockAccessCache->hasResidentChunkAtBlock(i, k))
		return false;
#endif
	return !accessIsBlockOpaqueCube(i, j, k);
}

void RenderBlocks::renderBlockUsingTexture(Block *block, int_t i, int_t j, int_t k, int_t l)
{
	overrideBlockTexture = l;
	renderBlockByRenderType(block, i, j, k);
	clearOverrideBlockTexture();
}

void RenderBlocks::renderBlockAllFaces(Block *block, int_t i, int_t j, int_t k)
{
	renderAllFaces = true;
	renderBlockByRenderType(block, i, j, k);
	renderAllFaces = false;
}

bool RenderBlocks::renderBlockByRenderType(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = block->getRenderType();
	// An opaque cube covers its whole 1x1 face with texels the alpha test always
	// passes; anything else may not. See RenderBlocks::usedAlphaTestedTexture.
	// The grass side overlay is the one case this test misses, and it raises the
	// flag at each of its own call sites below.
	if (!block->isOpaqueCube())
		usedAlphaTestedTexture = true;
	block->setBlockBoundsBasedOnState(blockAccess, i, j, k);
	if (l == 0)
	{
		return renderStandardBlock(block, i, j, k);
	}
	if (l == 4)
	{
		return renderBlockFluids(block, i, j, k);
	}
	if (l == 13)
	{
		return renderBlockCactus(block, i, j, k);
	}
	if (l == 1)
	{
		return renderCrossedSquares(block, i, j, k);
	}
	if (l == 6)
	{
		return renderBlockCrops(block, i, j, k);
	}
	if (l == 2)
	{
		return renderBlockTorch(block, i, j, k);
	}
	if (l == 3)
	{
		return renderBlockFire(block, i, j, k);
	}
	if (l == 5)
	{
		return renderBlockRedstoneWire(block, i, j, k);
	}
	if (l == 8)
	{
		return renderBlockLadder(block, i, j, k);
	}
	if (l == 7)
	{
		return renderBlockDoor(block, i, j, k);
	}
	if (l == 9)
	{
		return renderBlockMinecartTrack((BlockRail *)block, i, j, k);
	}
	if (l == 10)
	{
		return renderBlockStairs(block, i, j, k);
	}
	if (l == 11)
	{
		return renderBlockFence(block, i, j, k);
	}
	if (l == 12)
	{
		return renderBlockLever(block, i, j, k);
	}
	if (l == 14)
	{
		return renderBlockBed(block, i, j, k);
	}
	if (l == 15)
	{
		return renderBlockRepeater(block, i, j, k);
	}
	if (l == 16)
	{
		return renderPistonBase(block, i, j, k, false);
	}
	if (l == 17)
	{
		return renderPistonExtension(block, i, j, k, true);
	}
	if (l == 18)
	{
		return renderBlockPane(block, i, j, k);
	}
	if (l == 19)
	{
		return renderBlockStem(block, i, j, k);
	}
	if (l == 20)
	{
		return renderBlockVine(block, i, j, k);
	}
	if (l == 21)
	{
		return renderBlockFenceGate(block, i, j, k);
	}
	if (l == 23)
	{
		return renderBlockLilyPad(block, i, j, k);
	}
	if (l == 24)
	{
		return renderBlockCauldron(block, i, j, k);
	}
	if (l == 25)
	{
		return renderBlockBrewingStand(block, i, j, k);
	}
	if (l == 26)
	{
		return renderBlockEndPortalFrame(block, i, j, k);
	}
	if (l == 27)
	{
		return renderBlockDragonEgg(block, i, j, k);
	}
	return false;
}

bool RenderBlocks::renderBlockBed(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = accessGetBlockMetadata(i, j, k);
	int_t i1 = BlockBed::getDirectionFromMetadata(l);
	bool flag = BlockBed::isBlockFootOfBed(l);
	float f = 0.5f;
	float f1 = 1.0f;
	float f2 = 0.8f;
	float f3 = 0.6f;
	float f4 = f1;
	float f5 = f1;
	float f6 = f1;
	float f7 = f;
	float f8 = f2;
	float f9 = f3;
	float f10 = f;
	float f11 = f2;
	float f12 = f3;
	float f13 = f;
	float f14 = f2;
	float f15 = f3;
	float f16 = block->getBlockBrightness(blockAccess, i, j, k);
	(void)f16;
	const int_t packedBrightness = block->getMixedBrightnessForBlock(blockAccess, i, j, k);
	tessellator->setBrightness(packedBrightness);
	tessellator->setColorOpaque_F(f7, f10, f13);
	int_t j1 = block->getBlockTexture(blockAccess, i, j, k, 0);
	int_t k1 = (j1 & 0xf) << 4;
	int_t l1 = j1 & 0xf0;
	tess_coord_t d = (float)k1 / 256.0f;
	tess_coord_t d2 = ((tess_coord_t)(k1 + 16) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d4 = (float)l1 / 256.0f;
	tess_coord_t d6 = ((tess_coord_t)(l1 + 16) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d8 = (tess_coord_t)i + (tess_coord_t)block->minX;
	tess_coord_t d10 = (tess_coord_t)i + (tess_coord_t)block->maxX;
	tess_coord_t d12 = (tess_coord_t)j + (tess_coord_t)block->minY + 0.1875f;
	tess_coord_t d14 = (tess_coord_t)k + (tess_coord_t)block->minZ;
	tess_coord_t d16 = (tess_coord_t)k + (tess_coord_t)block->maxZ;
	tessellator->addVertexWithUV(d8, d12, d16, d, d6);
	tessellator->addVertexWithUV(d8, d12, d14, d, d4);
	tessellator->addVertexWithUV(d10, d12, d14, d2, d4);
	tessellator->addVertexWithUV(d10, d12, d16, d2, d6);
	float f17 = block->getBlockBrightness(blockAccess, i, j + 1, k);
	(void)f17;
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j + 1, k));
	tessellator->setColorOpaque_F(f4, f5, f6);
	j1 = block->getBlockTexture(blockAccess, i, j, k, 1);
	k1 = (j1 & 0xf) << 4;
	d = j1 & 0xf0;
	tess_coord_t d1 = (float)k1 / 256.0f;
	tess_coord_t d3 = ((tess_coord_t)(k1 + 16) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d5 = (float)d / 256.0f;
	tess_coord_t d7 = ((tess_coord_t)(d + 16) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d9 = d1;
	tess_coord_t d11 = d3;
	tess_coord_t d13 = d5;
	tess_coord_t d15 = d5;
	tess_coord_t d17 = d1;
	tess_coord_t d18 = d3;
	tess_coord_t d19 = d7;
	tess_coord_t d20 = d7;
	if (i1 == 0)
	{
		d11 = d1;
		d13 = d7;
		d17 = d3;
		d20 = d5;
	}
	else if (i1 == 2)
	{
		d9 = d3;
		d15 = d7;
		d18 = d1;
		d19 = d5;
	}
	else if (i1 == 3)
	{
		d9 = d3;
		d15 = d7;
		d18 = d1;
		d19 = d5;
		d11 = d1;
		d13 = d7;
		d17 = d3;
		d20 = d5;
	}
	tess_coord_t d21 = (tess_coord_t)i + (tess_coord_t)block->minX;
	tess_coord_t d22 = (tess_coord_t)i + (tess_coord_t)block->maxX;
	tess_coord_t d23 = (tess_coord_t)j + (tess_coord_t)block->maxY;
	tess_coord_t d24 = (tess_coord_t)k + (tess_coord_t)block->minZ;
	tess_coord_t d25 = (tess_coord_t)k + (tess_coord_t)block->maxZ;
	tessellator->addVertexWithUV(d22, d23, d25, d17, d19);
	tessellator->addVertexWithUV(d22, d23, d24, d9, d13);
	tessellator->addVertexWithUV(d21, d23, d24, d11, d15);
	tessellator->addVertexWithUV(d21, d23, d25, d18, d20);
	int_t f17a = ModelBed::field_22280_a[i1];
	if (flag)
	{
		f17a = ModelBed::field_22280_a[ModelBed::field_22279_b[i1]];
	}
	j1 = 4;
	switch (i1)
	{
	case 0:
		j1 = 5;
		break;
	case 3:
		j1 = 2;
		break;
	case 1:
		j1 = 3;
		break;
	}
	if (f17a != 2 && (renderAllFaces || shouldRenderFace(block, i, j, k - 1, 2)))
	{
		float f18 = block->getBlockBrightness(blockAccess, i, j, k - 1);
		(void)f18;
		tessellator->setBrightness(block->minZ > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k - 1));
		tessellator->setColorOpaque_F(f8, f11, f14);
		flipTexture = j1 == 2;
		renderEastFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 2));
	}
	if (f17a != 3 && (renderAllFaces || shouldRenderFace(block, i, j, k + 1, 3)))
	{
		float f19 = block->getBlockBrightness(blockAccess, i, j, k + 1);
		(void)f19;
		tessellator->setBrightness(block->maxZ < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k + 1));
		tessellator->setColorOpaque_F(f8, f11, f14);
		flipTexture = j1 == 3;
		renderWestFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 3));
	}
	if (f17a != 4 && (renderAllFaces || shouldRenderFace(block, i - 1, j, k, 4)))
	{
		float f20 = block->getBlockBrightness(blockAccess, i - 1, j, k);
		(void)f20;
		tessellator->setBrightness(block->minX > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i - 1, j, k));
		tessellator->setColorOpaque_F(f9, f12, f15);
		flipTexture = j1 == 4;
		renderNorthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 4));
	}
	if (f17a != 5 && (renderAllFaces || shouldRenderFace(block, i + 1, j, k, 5)))
	{
		float f21 = block->getBlockBrightness(blockAccess, i + 1, j, k);
		(void)f21;
		tessellator->setBrightness(block->maxX < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i + 1, j, k));
		tessellator->setColorOpaque_F(f9, f12, f15);
		flipTexture = j1 == 5;
		renderSouthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 5));
	}
	flipTexture = false;
	return true;
}

bool RenderBlocks::hasSnowNeighbours(int_t x, int_t y, int_t z)
{
	if (Block::snow == nullptr)
		return false;

	const int_t snowId = Block::snow->blockID;
	const bool hasAdjacentSnow = accessGetBlockId(x - 1, y, z) == snowId ||
	                             accessGetBlockId(x + 1, y, z) == snowId ||
	                             accessGetBlockId(x, y, z - 1) == snowId ||
	                             accessGetBlockId(x, y, z + 1) == snowId;
	return hasAdjacentSnow && accessIsBlockNormalCube(x, y - 1, z);
}

void RenderBlocks::renderBetterSnow(int_t x, int_t y, int_t z, double maxY)
{
	if (!Config::isBetterSnow() || !hasSnowNeighbours(x, y, z) || Block::snow == nullptr)
		return;

	if (maxY < 0.0)
	{
		renderStandardBlock(Block::snow, x, y, z);
		return;
	}

	const double oldMaxY = Block::snow->maxY;
	Block::snow->maxY = maxY;
	renderStandardBlock(Block::snow, x, y, z);
	Block::snow->maxY = oldMaxY;
}

bool RenderBlocks::renderBlockTorch(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = accessGetBlockMetadata(i, j, k);
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	tess_coord_t d = (tess_coord_t)0.40000000596046448;
	tess_coord_t d1 = 0.5f - d;
	tess_coord_t d2 = (tess_coord_t)0.20000000298023224;
	if (l == 1)
	{
		renderTorchAtAngle(block, (tess_coord_t)i - d1, (tess_coord_t)j + d2, k, -d, 0.0);
	}
	else if (l == 2)
	{
		renderTorchAtAngle(block, (tess_coord_t)i + d1, (tess_coord_t)j + d2, k, d, 0.0);
	}
	else if (l == 3)
	{
		renderTorchAtAngle(block, i, (tess_coord_t)j + d2, (tess_coord_t)k - d1, 0.0, -d);
	}
	else if (l == 4)
	{
		renderTorchAtAngle(block, i, (tess_coord_t)j + d2, (tess_coord_t)k + d1, 0.0, d);
	}
	else
	{
		renderTorchAtAngle(block, i, j, k, 0.0, 0.0);
		renderBetterSnow(i, j, k);
	}
	return true;
}

bool RenderBlocks::renderBlockRepeater(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = accessGetBlockMetadata(i, j, k);
	int_t i1 = l & 3;
	int_t j1 = (l & 0xc) >> 2;
	renderStandardBlock(block, i, j, k);
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	tess_coord_t d = -0.1875f;
	tess_coord_t d1 = 0.0f;
	tess_coord_t d2 = 0.0f;
	tess_coord_t d3 = 0.0f;
	tess_coord_t d4 = 0.0f;
	switch (i1)
	{
	case 0:
		d4 = -0.3125f;
		d2 = BlockRedstoneRepeater::isPowered[j1];
		break;
	case 2:
		d4 = 0.3125f;
		d2 = -BlockRedstoneRepeater::isPowered[j1];
		break;
	case 3:
		d3 = -0.3125f;
		d1 = BlockRedstoneRepeater::isPowered[j1];
		break;
	case 1:
		d3 = 0.3125f;
		d1 = -BlockRedstoneRepeater::isPowered[j1];
		break;
	}
	renderTorchAtAngle(block, (tess_coord_t)i + d1, (tess_coord_t)j + d, (tess_coord_t)k + d2, 0.0, 0.0);
	renderTorchAtAngle(block, (tess_coord_t)i + d3, (tess_coord_t)j + d, (tess_coord_t)k + d4, 0.0, 0.0);
	int_t k1 = block->getBlockTextureFromSide(1);
	int_t l1 = (k1 & 0xf) << 4;
	int_t i2 = k1 & 0xf0;
	tess_coord_t d5 = (float)l1 / 256.0f;
	tess_coord_t d6 = ((float)l1 + 15.99f) / 256.0f;
	tess_coord_t d7 = (float)i2 / 256.0f;
	tess_coord_t d8 = ((float)i2 + 15.99f) / 256.0f;
	float f1 = 0.125f;
	float f2 = i + 1;
	float f3 = i + 1;
	float f4 = i + 0;
	float f5 = i + 0;
	float f6 = k + 0;
	float f7 = k + 1;
	float f8 = k + 1;
	float f9 = k + 0;
	float f10 = (float)j + f1;
	if (i1 == 2)
	{
		f2 = f3 = i + 0;
		f4 = f5 = i + 1;
		f6 = f9 = k + 1;
		f7 = f8 = k + 0;
	}
	else if (i1 == 3)
	{
		f2 = f5 = i + 0;
		f3 = f4 = i + 1;
		f6 = f7 = k + 0;
		f8 = f9 = k + 1;
	}
	else if (i1 == 1)
	{
		f2 = f5 = i + 1;
		f3 = f4 = i + 0;
		f6 = f7 = k + 1;
		f8 = f9 = k + 0;
	}
	tessellator->addVertexWithUV(f5, f10, f9, d5, d7);
	tessellator->addVertexWithUV(f4, f10, f8, d5, d8);
	tessellator->addVertexWithUV(f3, f10, f7, d6, d8);
	tessellator->addVertexWithUV(f2, f10, f6, d6, d7);
	return true;
}

void RenderBlocks::renderPistonBaseAllFaces(Block *block, int_t i, int_t j, int_t k)
{
	renderAllFaces = true;
	renderPistonBase(block, i, j, k, true);
	renderAllFaces = false;
}

bool RenderBlocks::renderPistonBase(Block *block, int_t i, int_t j, int_t k, bool flag)
{
	int_t l = accessGetBlockMetadata(i, j, k);
	bool flag1 = flag || (l & 8) != 0;
	int_t i1 = BlockPistonBase::getPistonOrientation(l);
	if (flag1)
	{
		switch (i1)
		{
		case 0:
			eastFaceRotation = 3;
			westFaceRotation = 3;
			southFaceRotation = 3;
			northFaceRotation = 3;
			block->setBlockBounds(0.0f, 0.25f, 0.0f, 1.0f, 1.0f, 1.0f);
			break;
		case 1:
			block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f);
			break;
		case 2:
			southFaceRotation = 1;
			northFaceRotation = 2;
			block->setBlockBounds(0.0f, 0.0f, 0.25f, 1.0f, 1.0f, 1.0f);
			break;
		case 3:
			southFaceRotation = 2;
			northFaceRotation = 1;
			topFaceRotation = 3;
			bottomFaceRotation = 3;
			block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.75f);
			break;
		case 4:
			eastFaceRotation = 1;
			westFaceRotation = 2;
			topFaceRotation = 2;
			bottomFaceRotation = 1;
			block->setBlockBounds(0.25f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
			break;
		case 5:
			eastFaceRotation = 2;
			westFaceRotation = 1;
			topFaceRotation = 1;
			bottomFaceRotation = 2;
			block->setBlockBounds(0.0f, 0.0f, 0.0f, 0.75f, 1.0f, 1.0f);
			break;
		}
		renderStandardBlock(block, i, j, k);
		eastFaceRotation = 0;
		westFaceRotation = 0;
		southFaceRotation = 0;
		northFaceRotation = 0;
		topFaceRotation = 0;
		bottomFaceRotation = 0;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		switch (i1)
		{
		case 0:
			eastFaceRotation = 3;
			westFaceRotation = 3;
			southFaceRotation = 3;
			northFaceRotation = 3;
			break;
		case 2:
			southFaceRotation = 1;
			northFaceRotation = 2;
			break;
		case 3:
			southFaceRotation = 2;
			northFaceRotation = 1;
			topFaceRotation = 3;
			bottomFaceRotation = 3;
			break;
		case 4:
			eastFaceRotation = 1;
			westFaceRotation = 2;
			topFaceRotation = 2;
			bottomFaceRotation = 1;
			break;
		case 5:
			eastFaceRotation = 2;
			westFaceRotation = 1;
			topFaceRotation = 1;
			bottomFaceRotation = 2;
			break;
		}
		renderStandardBlock(block, i, j, k);
		eastFaceRotation = 0;
		westFaceRotation = 0;
		southFaceRotation = 0;
		northFaceRotation = 0;
		topFaceRotation = 0;
		bottomFaceRotation = 0;
	}
	return true;
}

void RenderBlocks::renderPistonArmX(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6)
{
	int_t i = 108;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	Tessellator *tessellator = &Tessellator::instance;
	tess_coord_t d7 = (float)(j + 0) / 256.0f;
	tess_coord_t d8 = (float)(k + 0) / 256.0f;
	tess_coord_t d9 = (((tess_coord_t)j + (tess_coord_t)d6) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d10 = ((tess_coord_t)((float)k + 4.0f) - kAtlasUvGuard) / 256.0f;
	tessellator->setColorOpaque_F(f, f, f);
	tessellator->addVertexWithUV(d, d3, d4, d9, d8);
	tessellator->addVertexWithUV(d, d2, d4, d7, d8);
	tessellator->addVertexWithUV(d1, d2, d5, d7, d10);
	tessellator->addVertexWithUV(d1, d3, d5, d9, d10);
}

void RenderBlocks::renderPistonArmY(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6)
{
	int_t i = 108;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	Tessellator *tessellator = &Tessellator::instance;
	tess_coord_t d7 = (float)(j + 0) / 256.0f;
	tess_coord_t d8 = (float)(k + 0) / 256.0f;
	tess_coord_t d9 = (((tess_coord_t)j + (tess_coord_t)d6) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d10 = ((tess_coord_t)((float)k + 4.0f) - kAtlasUvGuard) / 256.0f;
	tessellator->setColorOpaque_F(f, f, f);
	tessellator->addVertexWithUV(d, d2, d5, d9, d8);
	tessellator->addVertexWithUV(d, d2, d4, d7, d8);
	tessellator->addVertexWithUV(d1, d3, d4, d7, d10);
	tessellator->addVertexWithUV(d1, d3, d5, d9, d10);
}

void RenderBlocks::renderPistonArmZ(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6)
{
	int_t i = 108;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	Tessellator *tessellator = &Tessellator::instance;
	tess_coord_t d7 = (float)(j + 0) / 256.0f;
	tess_coord_t d8 = (float)(k + 0) / 256.0f;
	tess_coord_t d9 = (((tess_coord_t)j + (tess_coord_t)d6) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d10 = ((tess_coord_t)((float)k + 4.0f) - kAtlasUvGuard) / 256.0f;
	tessellator->setColorOpaque_F(f, f, f);
	tessellator->addVertexWithUV(d1, d2, d4, d9, d8);
	tessellator->addVertexWithUV(d, d2, d4, d7, d8);
	tessellator->addVertexWithUV(d, d3, d5, d7, d10);
	tessellator->addVertexWithUV(d1, d3, d5, d9, d10);
}

void RenderBlocks::renderPistonExtensionAllFaces(Block *block, int_t i, int_t j, int_t k, bool flag)
{
	renderAllFaces = true;
	renderPistonExtension(block, i, j, k, flag);
	renderAllFaces = false;
}

bool RenderBlocks::renderPistonExtension(Block *block, int_t i, int_t j, int_t k, bool flag)
{
	int_t l = accessGetBlockMetadata(i, j, k);
	int_t i1 = BlockPistonExtension::getPistonExtensionType(l); // orientation from metadata low 3 bits
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	float f1 = flag ? 1.0f : 0.5f;
	float d = flag ? 16.0f : 8.0f;
	switch (i1)
	{
	case 0:
		eastFaceRotation = 3;
		westFaceRotation = 3;
		southFaceRotation = 3;
		northFaceRotation = 3;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.625f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.625f, (float)k + 0.625f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.375f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.375f, (float)k + 0.375f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.375f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.375f, (float)k + 0.625f, f * 0.6f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.625f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.625f, (float)k + 0.375f, f * 0.6f, d);
		break;
	case 1:
		block->setBlockBounds(0.0f, 0.75f, 0.0f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.625f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.625f, (float)k + 0.625f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.375f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.375f, (float)k + 0.375f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.375f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.375f, (float)k + 0.625f, f * 0.6f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.625f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.625f, (float)k + 0.375f, f * 0.6f, d);
		break;
	case 2:
		southFaceRotation = 1;
		northFaceRotation = 2;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.375f, (float)k + 0.25f, (float)k + 0.25f + f1, f * 0.6f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.625f, (float)k + 0.25f, (float)k + 0.25f + f1, f * 0.6f, d);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.375f, (float)k + 0.25f, (float)k + 0.25f + f1, f * 0.5f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.625f, (float)k + 0.25f, (float)k + 0.25f + f1, f, d);
		break;
	case 3:
		southFaceRotation = 2;
		northFaceRotation = 1;
		topFaceRotation = 3;
		bottomFaceRotation = 3;
		block->setBlockBounds(0.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.375f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f * 0.6f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.625f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f * 0.6f, d);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.375f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f * 0.5f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.625f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f, d);
		break;
	case 4:
		eastFaceRotation = 1;
		westFaceRotation = 2;
		topFaceRotation = 2;
		bottomFaceRotation = 1;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 0.25f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.375f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.375f, f * 0.5f, d);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.625f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.625f, f, d);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.375f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.375f, f * 0.6f, d);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.625f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.625f, f * 0.6f, d);
		break;
	case 5:
		eastFaceRotation = 2;
		westFaceRotation = 1;
		topFaceRotation = 1;
		bottomFaceRotation = 2;
		block->setBlockBounds(0.75f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.375f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.375f, f * 0.5f, d);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.625f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.625f, f, d);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.375f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.375f, f * 0.6f, d);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.625f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.625f, f * 0.6f, d);
		break;
	}
	eastFaceRotation = 0;
	westFaceRotation = 0;
	southFaceRotation = 0;
	northFaceRotation = 0;
	topFaceRotation = 0;
	bottomFaceRotation = 0;
	block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool RenderBlocks::renderBlockLever(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = accessGetBlockMetadata(i, j, k);
	int_t i1 = l & 7;
	bool flag = (l & 8) > 0;
	Tessellator *tessellator = &Tessellator::instance;
	bool flag1 = overrideBlockTexture >= 0;
	if (!flag1)
	{
		overrideBlockTexture = Block::cobblestone->blockIndexInTexture;
	}
	float f = 0.25f;
	float f1 = 0.1875f;
	float f2 = 0.1875f;
	if (i1 == 5)
	{
		block->setBlockBounds(0.5f - f1, 0.0f, 0.5f - f, 0.5f + f1, f2, 0.5f + f);
	}
	else if (i1 == 6)
	{
		block->setBlockBounds(0.5f - f, 0.0f, 0.5f - f1, 0.5f + f, f2, 0.5f + f1);
	}
	else if (i1 == 4)
	{
		block->setBlockBounds(0.5f - f1, 0.5f - f, 1.0f - f2, 0.5f + f1, 0.5f + f, 1.0f);
	}
	else if (i1 == 3)
	{
		block->setBlockBounds(0.5f - f1, 0.5f - f, 0.0f, 0.5f + f1, 0.5f + f, f2);
	}
	else if (i1 == 2)
	{
		block->setBlockBounds(1.0f - f2, 0.5f - f, 0.5f - f1, 1.0f, 0.5f + f, 0.5f + f1);
	}
	else if (i1 == 1)
	{
		block->setBlockBounds(0.0f, 0.5f - f, 0.5f - f1, f2, 0.5f + f, 0.5f + f1);
	}
	renderStandardBlock(block, i, j, k);
	if (!flag1)
	{
		clearOverrideBlockTexture();
	}
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	int_t j1 = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		j1 = overrideBlockTexture;
	}
	int_t k1 = (j1 & 0xf) << 4;
	int_t l1 = j1 & 0xf0;
	float f4 = (float)k1 / 256.0f;
	float f5 = ((float)k1 + 15.99f) / 256.0f;
	float f6 = (float)l1 / 256.0f;
	float f7 = ((float)l1 + 15.99f) / 256.0f;
	Vec3D *avec3d[8];
	float f8 = 0.0625f;
	float f9 = 0.0625f;
	float f10 = 0.625f;
	avec3d[0] = Vec3D::createVector(-f8, 0.0, -f9);
	avec3d[1] = Vec3D::createVector(f8, 0.0, -f9);
	avec3d[2] = Vec3D::createVector(f8, 0.0, f9);
	avec3d[3] = Vec3D::createVector(-f8, 0.0, f9);
	avec3d[4] = Vec3D::createVector(-f8, f10, -f9);
	avec3d[5] = Vec3D::createVector(f8, f10, -f9);
	avec3d[6] = Vec3D::createVector(f8, f10, f9);
	avec3d[7] = Vec3D::createVector(-f8, f10, f9);
	for (int_t i2 = 0; i2 < 8; i2++)
	{
		if (flag)
		{
			avec3d[i2]->zCoord -= 0.0625;
			avec3d[i2]->rotateAroundX(0.6981317f);
		}
		else
		{
			avec3d[i2]->zCoord += 0.0625;
			avec3d[i2]->rotateAroundX(-0.6981317f);
		}
		if (i1 == 6)
		{
			avec3d[i2]->rotateAroundY(1.5707964f);
		}
		if (i1 < 5)
		{
			avec3d[i2]->yCoord -= 0.375;
			avec3d[i2]->rotateAroundX(1.5707964f);
			if (i1 == 4)
			{
				avec3d[i2]->rotateAroundY(0.0f);
			}
			if (i1 == 3)
			{
				avec3d[i2]->rotateAroundY(3.1415927f);
			}
			if (i1 == 2)
			{
				avec3d[i2]->rotateAroundY(1.5707964f);
			}
			if (i1 == 1)
			{
				avec3d[i2]->rotateAroundY(-1.5707964f);
			}
			avec3d[i2]->xCoord += (double)i + 0.5;
			avec3d[i2]->yCoord += (float)j + 0.5f;
			avec3d[i2]->zCoord += (double)k + 0.5;
		}
		else
		{
			avec3d[i2]->xCoord += (double)i + 0.5;
			avec3d[i2]->yCoord += (float)j + 0.125f;
			avec3d[i2]->zCoord += (double)k + 0.5;
		}
	}

	Vec3D *vec3d = nullptr;
	Vec3D *vec3d1 = nullptr;
	Vec3D *vec3d2 = nullptr;
	Vec3D *vec3d3 = nullptr;
	for (int_t j2 = 0; j2 < 6; j2++)
	{
		if (j2 == 0)
		{
			f4 = (float)(k1 + 7) / 256.0f;
			f5 = ((float)(k1 + 9) - 0.01f) / 256.0f;
			f6 = (float)(l1 + 6) / 256.0f;
			f7 = ((float)(l1 + 8) - 0.01f) / 256.0f;
		}
		else if (j2 == 2)
		{
			f4 = (float)(k1 + 7) / 256.0f;
			f5 = ((float)(k1 + 9) - 0.01f) / 256.0f;
			f6 = (float)(l1 + 6) / 256.0f;
			f7 = ((float)(l1 + 16) - 0.01f) / 256.0f;
		}
		if (j2 == 0)
		{
			vec3d = avec3d[0];
			vec3d1 = avec3d[1];
			vec3d2 = avec3d[2];
			vec3d3 = avec3d[3];
		}
		else if (j2 == 1)
		{
			vec3d = avec3d[7];
			vec3d1 = avec3d[6];
			vec3d2 = avec3d[5];
			vec3d3 = avec3d[4];
		}
		else if (j2 == 2)
		{
			vec3d = avec3d[1];
			vec3d1 = avec3d[0];
			vec3d2 = avec3d[4];
			vec3d3 = avec3d[5];
		}
		else if (j2 == 3)
		{
			vec3d = avec3d[2];
			vec3d1 = avec3d[1];
			vec3d2 = avec3d[5];
			vec3d3 = avec3d[6];
		}
		else if (j2 == 4)
		{
			vec3d = avec3d[3];
			vec3d1 = avec3d[2];
			vec3d2 = avec3d[6];
			vec3d3 = avec3d[7];
		}
		else if (j2 == 5)
		{
			vec3d = avec3d[0];
			vec3d1 = avec3d[3];
			vec3d2 = avec3d[7];
			vec3d3 = avec3d[4];
		}
		tessellator->addVertexWithUV(vec3d->xCoord, vec3d->yCoord, vec3d->zCoord, f4, f7);
		tessellator->addVertexWithUV(vec3d1->xCoord, vec3d1->yCoord, vec3d1->zCoord, f5, f7);
		tessellator->addVertexWithUV(vec3d2->xCoord, vec3d2->yCoord, vec3d2->zCoord, f5, f6);
		tessellator->addVertexWithUV(vec3d3->xCoord, vec3d3->yCoord, vec3d3->zCoord, f4, f6);
	}

	return true;
}

bool RenderBlocks::renderBlockFire(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		l = overrideBlockTexture;
	}
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	int_t i1 = (l & 0xf) << 4;
	int_t j1 = l & 0xf0;
	tess_coord_t d = (float)i1 / 256.0f;
	tess_coord_t d2 = ((float)i1 + 15.99f) / 256.0f;
	tess_coord_t d4 = (float)j1 / 256.0f;
	tess_coord_t d6 = ((float)j1 + 15.99f) / 256.0f;
	float f1 = 1.4f;
	if (accessIsBlockNormalCube(i, j - 1, k) || ((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j - 1, k))
	{
		tess_coord_t d8 = (tess_coord_t)i + 0.5f + (tess_coord_t)0.20000000000000001;
		tess_coord_t d9 = ((tess_coord_t)i + 0.5f) - (tess_coord_t)0.20000000000000001;
		tess_coord_t d12 = (tess_coord_t)k + 0.5f + (tess_coord_t)0.20000000000000001;
		tess_coord_t d14 = ((tess_coord_t)k + 0.5f) - (tess_coord_t)0.20000000000000001;
		tess_coord_t d16 = ((tess_coord_t)i + 0.5f) - (tess_coord_t)0.29999999999999999;
		tess_coord_t d18 = (tess_coord_t)i + 0.5f + (tess_coord_t)0.29999999999999999;
		tess_coord_t d20 = ((tess_coord_t)k + 0.5f) - (tess_coord_t)0.29999999999999999;
		tess_coord_t d22 = (tess_coord_t)k + 0.5f + (tess_coord_t)0.29999999999999999;
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 1, d2, d4);
		tessellator->addVertexWithUV(d8, j + 0, k + 1, d2, d6);
		tessellator->addVertexWithUV(d8, j + 0, k + 0, d, d6);
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 0, d, d4);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 0, d2, d4);
		tessellator->addVertexWithUV(d9, j + 0, k + 0, d2, d6);
		tessellator->addVertexWithUV(d9, j + 0, k + 1, d, d6);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 1, d, d4);
		d = (float)i1 / 256.0f;
		d2 = ((float)i1 + 15.99f) / 256.0f;
		d4 = (float)(j1 + 16) / 256.0f;
		d6 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d22, d2, d4);
		tessellator->addVertexWithUV(i + 1, j + 0, d14, d2, d6);
		tessellator->addVertexWithUV(i + 0, j + 0, d14, d, d6);
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d22, d, d4);
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d20, d2, d4);
		tessellator->addVertexWithUV(i + 0, j + 0, d12, d2, d6);
		tessellator->addVertexWithUV(i + 1, j + 0, d12, d, d6);
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d20, d, d4);
		d8 = ((tess_coord_t)i + 0.5f) - 0.5f;
		d9 = (tess_coord_t)i + 0.5f + 0.5f;
		d12 = ((tess_coord_t)k + 0.5f) - 0.5f;
		d14 = (tess_coord_t)k + 0.5f + 0.5f;
 
		d16 = ((tess_coord_t)i + 0.5f) - (tess_coord_t)0.40000000000000002;
		d18 = (tess_coord_t)i + 0.5f + (tess_coord_t)0.40000000000000002;
		d20 = ((tess_coord_t)k + 0.5f) - (tess_coord_t)0.40000000000000002;
		d22 = (tess_coord_t)k + 0.5f + (tess_coord_t)0.40000000000000002;
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 0, d, d4);
		tessellator->addVertexWithUV(d8, j + 0, k + 0, d, d6);
		tessellator->addVertexWithUV(d8, j + 0, k + 1, d2, d6);
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 1, d2, d4);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 1, d, d4);
		tessellator->addVertexWithUV(d9, j + 0, k + 1, d, d6);
		tessellator->addVertexWithUV(d9, j + 0, k + 0, d2, d6);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 0, d2, d4);
		d = (float)i1 / 256.0f;
		d2 = ((float)i1 + 15.99f) / 256.0f;
		d4 = (float)j1 / 256.0f;
		d6 = ((float)j1 + 15.99f) / 256.0f;
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d22, d, d4);
		tessellator->addVertexWithUV(i + 0, j + 0, d14, d, d6);
		tessellator->addVertexWithUV(i + 1, j + 0, d14, d2, d6);
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d22, d2, d4);
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d20, d, d4);
		tessellator->addVertexWithUV(i + 1, j + 0, d12, d, d6);
		tessellator->addVertexWithUV(i + 0, j + 0, d12, d2, d6);
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d20, d2, d4);
	}
	else
	{
		float f3 = 0.2f;
		float f4 = 0.0625f;
		if ((i + j + k & 1) == 1)
		{
			d = (float)i1 / 256.0f;
			d2 = ((float)i1 + 15.99f) / 256.0f;
			d4 = (float)(j1 + 16) / 256.0f;
			d6 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
		}
		if ((i / 2 + j / 2 + k / 2 & 1) == 1)
		{
			tess_coord_t d10 = d2;
			d2 = d;
			d = d10;
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i - 1, j, k))
		{
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 1, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 0, d, d4);
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 0, d, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 1, d2, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i + 1, j, k))
		{
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 0, d, d4);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 1, d2, d4);
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 1, d2, d4);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 0, d, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j, k - 1))
		{
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)k + f3, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d2, d6);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)k + f3, d, d4);
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)k + f3, d, d4);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d2, d6);
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)k + f3, d2, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j, k + 1))
		{
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)(k + 1) - f3, d, d4);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, (k + 1) - 0, d, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, (k + 1) - 0, d2, d6);
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)(k + 1) - f3, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)(k + 1) - f3, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, (k + 1) - 0, d2, d6);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, (k + 1) - 0, d, d6);
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)(k + 1) - f3, d, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j + 1, k))
		{
			tess_coord_t d11 = (tess_coord_t)i + 0.5f + 0.5f;
			tess_coord_t d13 = ((tess_coord_t)i + 0.5f) - 0.5f;
			tess_coord_t d15 = (tess_coord_t)k + 0.5f + 0.5f;
			tess_coord_t d17 = ((tess_coord_t)k + 0.5f) - 0.5f;
			tess_coord_t d19 = ((tess_coord_t)i + 0.5f) - 0.5f;
			tess_coord_t d21 = (tess_coord_t)i + 0.5f + 0.5f;
			tess_coord_t d23 = ((tess_coord_t)k + 0.5f) - 0.5f;
			tess_coord_t d24 = (tess_coord_t)k + 0.5f + 0.5f;
			tess_coord_t d1 = (float)i1 / 256.0f;
			tess_coord_t d3 = ((float)i1 + 15.99f) / 256.0f;
			tess_coord_t d5 = (float)j1 / 256.0f;
			tess_coord_t d7 = ((float)j1 + 15.99f) / 256.0f;
			j++;
			float f2 = -0.2f;
			if ((i + j + k & 1) == 0)
			{
				tessellator->addVertexWithUV(d19, (float)j + f2, k + 0, d3, d5);
				tessellator->addVertexWithUV(d11, j + 0, k + 0, d3, d7);
				tessellator->addVertexWithUV(d11, j + 0, k + 1, d1, d7);
				tessellator->addVertexWithUV(d19, (float)j + f2, k + 1, d1, d5);
				d1 = (float)i1 / 256.0f;
				d3 = ((float)i1 + 15.99f) / 256.0f;
				d5 = (float)(j1 + 16) / 256.0f;
				d7 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
				tessellator->addVertexWithUV(d21, (float)j + f2, k + 1, d3, d5);
				tessellator->addVertexWithUV(d13, j + 0, k + 1, d3, d7);
				tessellator->addVertexWithUV(d13, j + 0, k + 0, d1, d7);
				tessellator->addVertexWithUV(d21, (float)j + f2, k + 0, d1, d5);
			}
			else
			{
				tessellator->addVertexWithUV(i + 0, (float)j + f2, d24, d3, d5);
				tessellator->addVertexWithUV(i + 0, j + 0, d17, d3, d7);
				tessellator->addVertexWithUV(i + 1, j + 0, d17, d1, d7);
				tessellator->addVertexWithUV(i + 1, (float)j + f2, d24, d1, d5);
				d1 = (float)i1 / 256.0f;
				d3 = ((float)i1 + 15.99f) / 256.0f;
				d5 = (float)(j1 + 16) / 256.0f;
				d7 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
				tessellator->addVertexWithUV(i + 1, (float)j + f2, d23, d3, d5);
				tessellator->addVertexWithUV(i + 1, j + 0, d15, d3, d7);
				tessellator->addVertexWithUV(i + 0, j + 0, d15, d1, d7);
				tessellator->addVertexWithUV(i + 0, (float)j + f2, d23, d1, d5);
			}
		}
	}
	renderBetterSnow(i, j, k);
	return true;
}

bool RenderBlocks::renderBlockRedstoneWire(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = accessGetBlockMetadata(i, j, k);
	int_t i1 = block->getBlockTextureFromSideAndMetadata(1, l);
	if (overrideBlockTexture >= 0)
	{
		i1 = overrideBlockTexture;
	}
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	float f = 1.0f;
	float f1 = (float)l / 15.0f;
	float f2 = f1 * 0.6f + 0.4f;
	if (l == 0)
	{
		f2 = 0.3f;
	}
	float f3 = f1 * f1 * 0.7f - 0.5f;
	float f4 = f1 * f1 * 0.6f - 0.7f;
	if (f3 < 0.0f)
	{
		f3 = 0.0f;
	}
	if (f4 < 0.0f)
	{
		f4 = 0.0f;
	}
	const int_t customRedstoneColor = CustomColorizer::getRedstoneColor(l);
	if (customRedstoneColor >= 0)
	{
		f2 = ((customRedstoneColor >> 16) & 255) / 255.0f;
		f3 = ((customRedstoneColor >> 8) & 255) / 255.0f;
		f4 = (customRedstoneColor & 255) / 255.0f;
	}
	tessellator->setColorOpaque_F(f2, f3, f4);
	int_t j1 = (i1 & 0xf) << 4;
	int_t k1 = i1 & 0xf0;
	tess_coord_t d = (float)j1 / 256.0f;
	tess_coord_t d2 = ((float)j1 + 15.99f) / 256.0f;
	tess_coord_t d4 = (float)k1 / 256.0f;
	tess_coord_t d6 = ((float)k1 + 15.99f) / 256.0f;
	bool flag = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i - 1, j, k, 1) ||
	            !accessIsBlockNormalCube(i - 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i - 1, j - 1, k, -1);
	bool flag1 = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i + 1, j, k, 3) ||
	             !accessIsBlockNormalCube(i + 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i + 1, j - 1, k, -1);
	bool flag2 = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j, k - 1, 2) ||
	             !accessIsBlockNormalCube(i, j, k - 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j - 1, k - 1, -1);
	bool flag3 = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j, k + 1, 0) ||
	             !accessIsBlockNormalCube(i, j, k + 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j - 1, k + 1, -1);
	if (!accessIsBlockNormalCube(i, j + 1, k))
	{
		if (accessIsBlockNormalCube(i - 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i - 1, j + 1, k, -1))
		{
			flag = true;
		}
		if (accessIsBlockNormalCube(i + 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i + 1, j + 1, k, -1))
		{
			flag1 = true;
		}
		if (accessIsBlockNormalCube(i, j, k - 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j + 1, k - 1, -1))
		{
			flag2 = true;
		}
		if (accessIsBlockNormalCube(i, j, k + 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j + 1, k + 1, -1))
		{
			flag3 = true;
		}
	}
	float f5 = i + 0;
	float f6 = i + 1;
	float f7 = k + 0;
	float f8 = k + 1;
	byte_t byte0 = 0;
	if ((flag || flag1) && !flag2 && !flag3)
	{
		byte0 = 1;
	}
	if ((flag2 || flag3) && !flag1 && !flag)
	{
		byte0 = 2;
	}
	if (byte0 != 0)
	{
		d = (float)(j1 + 16) / 256.0f;
		d2 = ((float)(j1 + 16) + 15.99f) / 256.0f;
		d4 = (float)k1 / 256.0f;
		d6 = ((float)k1 + 15.99f) / 256.0f;
	}
	if (byte0 == 0)
	{
		if (flag1 || flag2 || flag3 || flag)
		{
			if (!flag)
			{
				f5 += 0.3125f;
			}
			if (!flag)
			{
				d += 0.01953125f;
			}
			if (!flag1)
			{
				f6 -= 0.3125f;
			}
			if (!flag1)
			{
				d2 -= 0.01953125f;
			}
			if (!flag2)
			{
				f7 += 0.3125f;
			}
			if (!flag2)
			{
				d4 += 0.01953125f;
			}
			if (!flag3)
			{
				f8 -= 0.3125f;
			}
			if (!flag3)
			{
				d6 -= 0.01953125f;
			}
		}
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6);
		tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6 + 0.0625f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4 + 0.0625f);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4 + 0.0625f);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6 + 0.0625f);
	}
	else if (byte0 == 1)
	{
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6);
		tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6 + 0.0625f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4 + 0.0625f);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4 + 0.0625f);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6 + 0.0625f);
	}
	else if (byte0 == 2)
	{
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d, d6);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d2, d4);
		tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6 + 0.0625f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d, d6 + 0.0625f);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4 + 0.0625f);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d2, d4 + 0.0625f);
	}
	if (!accessIsBlockNormalCube(i, j + 1, k))
	{
		tess_coord_t d1 = (float)(j1 + 16) / 256.0f;
		tess_coord_t d3 = ((float)(j1 + 16) + 15.99f) / 256.0f;
		tess_coord_t d5 = (float)k1 / 256.0f;
		tess_coord_t d7 = ((float)k1 + 15.99f) / 256.0f;
		if (accessIsBlockNormalCube(i - 1, j, k) && accessGetBlockId(i - 1, j + 1, k) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d5);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 1, d1, d5);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 0, d1, d7);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d7);
			tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d5 + 0.0625f);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 1, d1, d5 + 0.0625f);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 0, d1, d7 + 0.0625f);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d7 + 0.0625f);
		}
		if (accessIsBlockNormalCube(i + 1, j, k) && accessGetBlockId(i + 1, j + 1, k) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 1, d1, d7);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d7);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d5);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 0, d1, d5);
			tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 1, d1, d7 + 0.0625f);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d7 + 0.0625f);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d5 + 0.0625f);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 0, d1, d5 + 0.0625f);
		}
		if (accessIsBlockNormalCube(i, j, k - 1) && accessGetBlockId(i, j + 1, k - 1) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)k + 0.015625f, d1, d7);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d7);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d5);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)k + 0.015625f, d1, d5);
			tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)k + 0.015625f, d1, d7 + 0.0625f);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d7 + 0.0625f);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d5 + 0.0625f);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)k + 0.015625f, d1, d5 + 0.0625f);
		}
		if (accessIsBlockNormalCube(i, j, k + 1) && accessGetBlockId(i, j + 1, k + 1) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d5);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)(k + 1) - 0.015625f, d1, d5);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)(k + 1) - 0.015625f, d1, d7);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d7);
			tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d5 + 0.0625f);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)(k + 1) - 0.015625f, d1, d5 + 0.0625f);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)(k + 1) - 0.015625f, d1, d7 + 0.0625f);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d7 + 0.0625f);
		}
	}
	return true;
}

bool RenderBlocks::renderBlockMinecartTrack(BlockRail *blockrail, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = accessGetBlockMetadata(i, j, k);
	int_t i1 = blockrail->getBlockTextureFromSideAndMetadata(0, l);
	if (overrideBlockTexture >= 0)
	{
		i1 = overrideBlockTexture;
	}
	if (blockrail->getIsPowered())
	{
		l &= 7;
	}
	tessellator->setBrightness(blockrail->getMixedBrightnessForBlock(blockAccess, i, j, k));
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	int_t j1 = (i1 & 0xf) << 4;
	int_t k1 = i1 & 0xf0;
	tess_coord_t d = (float)j1 / 256.0f;
	tess_coord_t d1 = ((float)j1 + 15.99f) / 256.0f;
	tess_coord_t d2 = (float)k1 / 256.0f;
	tess_coord_t d3 = ((float)k1 + 15.99f) / 256.0f;
	float f1 = 0.0625f;
	float f2 = i + 1;
	float f3 = i + 1;
	float f4 = i + 0;
	float f5 = i + 0;
	float f6 = k + 0;
	float f7 = k + 1;
	float f8 = k + 1;
	float f9 = k + 0;
	float f10 = (float)j + f1;
	float f11 = (float)j + f1;
	float f12 = (float)j + f1;
	float f13 = (float)j + f1;
	if (l == 1 || l == 2 || l == 3 || l == 7)
	{
		f2 = f5 = i + 1;
		f3 = f4 = i + 0;
		f6 = f7 = k + 1;
		f8 = f9 = k + 0;
	}
	else if (l == 8)
	{
		f2 = f3 = i + 0;
		f4 = f5 = i + 1;
		f6 = f9 = k + 1;
		f7 = f8 = k + 0;
	}
	else if (l == 9)
	{
		f2 = f5 = i + 0;
		f3 = f4 = i + 1;
		f6 = f7 = k + 0;
		f8 = f9 = k + 1;
	}
	if (l == 2 || l == 4)
	{
		f10++;
		f13++;
	}
	else if (l == 3 || l == 5)
	{
		f11++;
		f12++;
	}
	tessellator->addVertexWithUV(f2, f10, f6, d1, d2);
	tessellator->addVertexWithUV(f3, f11, f7, d1, d3);
	tessellator->addVertexWithUV(f4, f12, f8, d, d3);
	tessellator->addVertexWithUV(f5, f13, f9, d, d2);
	tessellator->addVertexWithUV(f5, f13, f9, d, d2);
	tessellator->addVertexWithUV(f4, f12, f8, d, d3);
	tessellator->addVertexWithUV(f3, f11, f7, d1, d3);
	tessellator->addVertexWithUV(f2, f10, f6, d1, d2);
	renderBetterSnow(i, j, k, 0.01);
	return true;
}

bool RenderBlocks::renderBlockLadder(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		l = overrideBlockTexture;
	}
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	int_t i1 = (l & 0xf) << 4;
	int_t j1 = l & 0xf0;
	tess_coord_t d = (float)i1 / 256.0f;
	tess_coord_t d1 = ((float)i1 + 15.99f) / 256.0f;
	tess_coord_t d2 = (float)j1 / 256.0f;
	tess_coord_t d3 = ((float)j1 + 15.99f) / 256.0f;
	int_t k1 = accessGetBlockMetadata(i, j, k);
	float f1 = 0.0f;
	float f2 = 0.05f;
	if (k1 == 5)
	{
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 1) + f1, (float)(k + 1) + f1, d, d2);
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 0) - f1, (float)(k + 1) + f1, d, d3);
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 0) - f1, (float)(k + 0) - f1, d1, d3);
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 1) + f1, (float)(k + 0) - f1, d1, d2);
	}
	if (k1 == 4)
	{
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 0) - f1, (float)(k + 1) + f1, d1, d3);
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 1) + f1, (float)(k + 1) + f1, d1, d2);
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 1) + f1, (float)(k + 0) - f1, d, d2);
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 0) - f1, (float)(k + 0) - f1, d, d3);
	}
	if (k1 == 3)
	{
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 0) - f1, (float)k + f2, d1, d3);
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 1) + f1, (float)k + f2, d1, d2);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 1) + f1, (float)k + f2, d, d2);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 0) - f1, (float)k + f2, d, d3);
	}
	if (k1 == 2)
	{
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 1) + f1, (float)(k + 1) - f2, d, d2);
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 0) - f1, (float)(k + 1) - f2, d, d3);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 0) - f1, (float)(k + 1) - f2, d1, d3);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 1) + f1, (float)(k + 1) - f2, d1, d2);
	}
	renderBetterSnow(i, j, k, 0.05);
	return true;
}

bool RenderBlocks::renderCrossedSquares(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	int_t l = CustomColorizer::getColorMultiplier(block, blockAccess, i, j, k);
	float f1 = (float)(l >> 16 & 0xff) / 255.0f;
	float f2 = (float)(l >> 8 & 0xff) / 255.0f;
	float f3 = (float)(l & 0xff) / 255.0f;
	if (EntityRenderer::anaglyphEnabled)
	{
		float f4 = (f1 * 30.0f + f2 * 59.0f + f3 * 11.0f) / 100.0f;
		float f5 = (f1 * 30.0f + f2 * 70.0f) / 100.0f;
		float f6 = (f1 * 30.0f + f3 * 70.0f) / 100.0f;
		f1 = f4;
		f2 = f5;
		f3 = f6;
	}
	tessellator->setColorOpaque_F(f1, f2, f3);
	tess_coord_t d = i;
	tess_coord_t d1 = j;
	tess_coord_t d2 = k;
	if (block == static_cast<Block*>(Block::tallGrass))
	{
		const int_t xTerm = JavaArithmetic::intFromBits(static_cast<uint_t>(i) * 0x2fc20fu);
		const ulong_t initialBits = static_cast<ulong_t>(static_cast<long_t>(xTerm))
		                          ^ static_cast<ulong_t>(static_cast<long_t>(k)) * 0x6ebfff5ULL
		                          ^ static_cast<ulong_t>(static_cast<long_t>(j));
		long_t l1 = JavaArithmetic::longFromBits(initialBits);
		const ulong_t l1Bits = static_cast<ulong_t>(l1);
		l1 = JavaArithmetic::longFromBits(l1Bits * l1Bits * 0x285b825ULL + l1Bits * 11ULL);
		d += ((tess_coord_t)((float)(l1 >> 16 & 15LL) / 15.0f) - 0.5f) * 0.5f;
		d1 += ((tess_coord_t)((float)(l1 >> 20 & 15LL) / 15.0f) - 1.0f) * (tess_coord_t)0.20000000000000001;
		d2 += ((tess_coord_t)((float)(l1 >> 24 & 15LL) / 15.0f) - 0.5f) * 0.5f;
	}
	renderCrossedSquares(block, accessGetBlockMetadata(i, j, k), d, d1, d2);
	renderBetterSnow(i, j, k);
	return true;
}

bool RenderBlocks::renderBlockReed(Block *block, int_t i, int_t j, int_t k)
{
	return renderCrossedSquares(block, i, j, k);
}


bool RenderBlocks::renderBlockCrops(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	renderCropsCrossed(block, accessGetBlockMetadata(i, j, k), i, (float)j - 0.0625f, k);
	return true;
}

void RenderBlocks::renderTorchAtAngle(Block *block, double d, double d1, double d2, double d3, double d4)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t i = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	float f = (float)j / 256.0f;
	float f1 = ((float)j + 15.99f) / 256.0f;
	float f2 = (float)k / 256.0f;
	float f3 = ((float)k + 15.99f) / 256.0f;
	// All four offsets are exact multiples of 1/256, so they carry the same value
	// whichever scalar tess_coord_t resolves to.
	tess_coord_t d5 = (tess_coord_t)f + 0.02734375f;
	tess_coord_t d6 = (tess_coord_t)f2 + 0.0234375f;
	tess_coord_t d7 = (tess_coord_t)f + 0.03515625f;
	tess_coord_t d8 = (tess_coord_t)f2 + 0.03125f;
	const tess_coord_t x = (tess_coord_t)d + 0.5f;
	const tess_coord_t y = (tess_coord_t)d1;
	const tess_coord_t z = (tess_coord_t)d2 + 0.5f;
	const tess_coord_t xTilt = (tess_coord_t)d3;
	const tess_coord_t zTilt = (tess_coord_t)d4;
	tess_coord_t d9 = x - 0.5f;
	tess_coord_t d10 = x + 0.5f;
	tess_coord_t d11 = z - 0.5f;
	tess_coord_t d12 = z + 0.5f;
	const tess_coord_t d13 = 0.0625f;
	const tess_coord_t d14 = 0.625f;
	const tess_coord_t topScale = 1.0f - d14;
	tessellator->addVertexWithUV((x + xTilt * topScale) - d13, y + d14, (z + zTilt * topScale) - d13, d5, d6);
	tessellator->addVertexWithUV((x + xTilt * topScale) - d13, y + d14, z + zTilt * topScale + d13, d5, d8);
	tessellator->addVertexWithUV(x + xTilt * topScale + d13, y + d14, z + zTilt * topScale + d13, d7, d8);
	tessellator->addVertexWithUV(x + xTilt * topScale + d13, y + d14, (z + zTilt * topScale) - d13, d7, d6);
	tessellator->addVertexWithUV(x - d13, y + 1.0f, d11, f, f2);
	tessellator->addVertexWithUV((x - d13) + xTilt, y, d11 + zTilt, f, f3);
	tessellator->addVertexWithUV((x - d13) + xTilt, y, d12 + zTilt, f1, f3);
	tessellator->addVertexWithUV(x - d13, y + 1.0f, d12, f1, f2);
	tessellator->addVertexWithUV(x + d13, y + 1.0f, d12, f, f2);
	tessellator->addVertexWithUV(x + xTilt + d13, y, d12 + zTilt, f, f3);
	tessellator->addVertexWithUV(x + xTilt + d13, y, d11 + zTilt, f1, f3);
	tessellator->addVertexWithUV(x + d13, y + 1.0f, d11, f1, f2);
	tessellator->addVertexWithUV(d9, y + 1.0f, z + d13, f, f2);
	tessellator->addVertexWithUV(d9 + xTilt, y, z + d13 + zTilt, f, f3);
	tessellator->addVertexWithUV(d10 + xTilt, y, z + d13 + zTilt, f1, f3);
	tessellator->addVertexWithUV(d10, y + 1.0f, z + d13, f1, f2);
	tessellator->addVertexWithUV(d10, y + 1.0f, z - d13, f, f2);
	tessellator->addVertexWithUV(d10 + xTilt, y, (z - d13) + zTilt, f, f3);
	tessellator->addVertexWithUV(d9 + xTilt, y, (z - d13) + zTilt, f1, f3);
	tessellator->addVertexWithUV(d9, y + 1.0f, z - d13, f1, f2);
}

void RenderBlocks::renderCrossedSquares(Block *block, int_t i, double d, double d1, double d2)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t j = block->getBlockTextureFromSideAndMetadata(0, i);
	if (overrideBlockTexture >= 0)
	{
		j = overrideBlockTexture;
	}
	int_t k = (j & 0xf) << 4;
	int_t l = j & 0xf0;
	tess_coord_t d3 = (float)k / 256.0f;
	tess_coord_t d4 = ((float)k + 15.99f) / 256.0f;
	tess_coord_t d5 = (float)l / 256.0f;
	tess_coord_t d6 = ((float)l + 15.99f) / 256.0f;
	const tess_coord_t x = (tess_coord_t)d;
	const tess_coord_t y = (tess_coord_t)d1;
	const tess_coord_t z = (tess_coord_t)d2;
	const tess_coord_t half = 0.5f;
	const tess_coord_t inset = (tess_coord_t)0.45f;
	tess_coord_t d7 = (x + half) - inset;
	tess_coord_t d8 = x + half + inset;
	tess_coord_t d9 = (z + half) - inset;
	tess_coord_t d10 = z + half + inset;
	tessellator->addVertexWithUV(d7, y + 1.0f, d9, d3, d5);
	tessellator->addVertexWithUV(d7, y, d9, d3, d6);
	tessellator->addVertexWithUV(d8, y, d10, d4, d6);
	tessellator->addVertexWithUV(d8, y + 1.0f, d10, d4, d5);
	tessellator->addVertexWithUV(d8, y + 1.0f, d10, d3, d5);
	tessellator->addVertexWithUV(d8, y, d10, d3, d6);
	tessellator->addVertexWithUV(d7, y, d9, d4, d6);
	tessellator->addVertexWithUV(d7, y + 1.0f, d9, d4, d5);
	tessellator->addVertexWithUV(d7, y + 1.0f, d10, d3, d5);
	tessellator->addVertexWithUV(d7, y, d10, d3, d6);
	tessellator->addVertexWithUV(d8, y, d9, d4, d6);
	tessellator->addVertexWithUV(d8, y + 1.0f, d9, d4, d5);
	tessellator->addVertexWithUV(d8, y + 1.0f, d9, d3, d5);
	tessellator->addVertexWithUV(d8, y, d9, d3, d6);
	tessellator->addVertexWithUV(d7, y, d10, d4, d6);
	tessellator->addVertexWithUV(d7, y + 1.0f, d10, d4, d5);
}

void RenderBlocks::renderCropsCrossed(Block *block, int_t i, double d, double d1, double d2)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t j = block->getBlockTextureFromSideAndMetadata(0, i);
	if (overrideBlockTexture >= 0)
	{
		j = overrideBlockTexture;
	}
	int_t k = (j & 0xf) << 4;
	int_t l = j & 0xf0;
	tess_coord_t d3 = (float)k / 256.0f;
	tess_coord_t d4 = ((float)k + 15.99f) / 256.0f;
	tess_coord_t d5 = (float)l / 256.0f;
	tess_coord_t d6 = ((float)l + 15.99f) / 256.0f;
	const tess_coord_t x = (tess_coord_t)d;
	const tess_coord_t y = (tess_coord_t)d1;
	const tess_coord_t z = (tess_coord_t)d2;
	tess_coord_t d7 = (x + 0.5f) - 0.25f;
	tess_coord_t d8 = x + 0.5f + 0.25f;
	tess_coord_t d9 = (z + 0.5f) - 0.5f;
	tess_coord_t d10 = z + 0.5f + 0.5f;
	tessellator->addVertexWithUV(d7, y + 1.0f, d9, d3, d5);
	tessellator->addVertexWithUV(d7, y, d9, d3, d6);
	tessellator->addVertexWithUV(d7, y, d10, d4, d6);
	tessellator->addVertexWithUV(d7, y + 1.0f, d10, d4, d5);
	tessellator->addVertexWithUV(d7, y + 1.0f, d10, d3, d5);
	tessellator->addVertexWithUV(d7, y, d10, d3, d6);
	tessellator->addVertexWithUV(d7, y, d9, d4, d6);
	tessellator->addVertexWithUV(d7, y + 1.0f, d9, d4, d5);
	tessellator->addVertexWithUV(d8, y + 1.0f, d10, d3, d5);
	tessellator->addVertexWithUV(d8, y, d10, d3, d6);
	tessellator->addVertexWithUV(d8, y, d9, d4, d6);
	tessellator->addVertexWithUV(d8, y + 1.0f, d9, d4, d5);
	tessellator->addVertexWithUV(d8, y + 1.0f, d9, d3, d5);
	tessellator->addVertexWithUV(d8, y, d9, d3, d6);
	tessellator->addVertexWithUV(d8, y, d10, d4, d6);
	tessellator->addVertexWithUV(d8, y + 1.0f, d10, d4, d5);
	d7 = (x + 0.5f) - 0.5f;
	d8 = x + 0.5f + 0.5f;
	d9 = (z + 0.5f) - 0.25f;
	d10 = z + 0.5f + 0.25f;
	tessellator->addVertexWithUV(d7, y + 1.0f, d9, d3, d5);
	tessellator->addVertexWithUV(d7, y, d9, d3, d6);
	tessellator->addVertexWithUV(d8, y, d9, d4, d6);
	tessellator->addVertexWithUV(d8, y + 1.0f, d9, d4, d5);
	tessellator->addVertexWithUV(d8, y + 1.0f, d9, d3, d5);
	tessellator->addVertexWithUV(d8, y, d9, d3, d6);
	tessellator->addVertexWithUV(d7, y, d9, d4, d6);
	tessellator->addVertexWithUV(d7, y + 1.0f, d9, d4, d5);
	tessellator->addVertexWithUV(d8, y + 1.0f, d10, d3, d5);
	tessellator->addVertexWithUV(d8, y, d10, d3, d6);
	tessellator->addVertexWithUV(d7, y, d10, d4, d6);
	tessellator->addVertexWithUV(d7, y + 1.0f, d10, d4, d5);
}

bool RenderBlocks::renderBlockFluids(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = CustomColorizer::getFluidColor(block, blockAccess, i, j, k);
	float f = (float)(l >> 16 & 0xff) / 255.0f;
	float f1 = (float)(l >> 8 & 0xff) / 255.0f;
	float f2 = (float)(l & 0xff) / 255.0f;
	bool flag = shouldRenderFace(block, i, j + 1, k, 1);
	bool flag1 = shouldRenderFace(block, i, j - 1, k, 0);
	bool aflag[4];
	aflag[0] = shouldRenderFace(block, i, j, k - 1, 2);
	aflag[1] = shouldRenderFace(block, i, j, k + 1, 3);
	aflag[2] = shouldRenderFace(block, i - 1, j, k, 4);
	aflag[3] = shouldRenderFace(block, i + 1, j, k, 5);
	if (!flag && !flag1 && !aflag[0] && !aflag[1] && !aflag[2] && !aflag[3])
	{
		return false;
	}
	bool flag2 = false;
	float f3 = 0.5f;
	float f4 = 1.0f;
	float f5 = 0.8f;
	float f6 = 0.6f;
	double d = 0.0;
	double d1 = 1.0;
	Material *material = block->blockMaterial;
	int_t i1 = accessGetBlockMetadata(i, j, k);
	float f7 = getFluidHeight(i, j, k, material);
	float f8 = getFluidHeight(i, j, k + 1, material);
	float f9 = getFluidHeight(i + 1, j, k + 1, material);
	float f10 = getFluidHeight(i + 1, j, k, material);
	if (renderAllFaces || flag)
	{
		flag2 = true;
		int_t j1 = block->getBlockTextureFromSideAndMetadata(1, i1);
#if PLATFORM_FLOAT_FLUID_FLOW
		float f12 = BlockFluid::getFlowDirectionFloat(blockAccess, i, j, k, material);
#else
		float f12 = (float)BlockFluid::getFlowDirection(blockAccess, i, j, k, material);
#endif
		if (f12 > -999.0f)
		{
			j1 = block->getBlockTextureFromSideAndMetadata(2, i1);
		}
		int_t i2 = (j1 & 0xf) << 4;
		int_t k2 = j1 & 0xf0;
		tess_coord_t d2 = ((tess_coord_t)i2 + 8.0f) / 256.0f;
		tess_coord_t d3 = ((tess_coord_t)k2 + 8.0f) / 256.0f;
		if (f12 < -999.0f)
		{
			f12 = 0.0f;
		}
		else
		{
			d2 = (float)(i2 + 16) / 256.0f;
			d3 = (float)(k2 + 16) / 256.0f;
		}
		float f14 = (MathHelper::sin(f12) * 8.0f) / 256.0f;
		float f16 = (MathHelper::cos(f12) * 8.0f) / 256.0f;
		tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j, k));
		tessellator->setColorOpaque_F(f4 * f, f4 * f1, f4 * f2);
		// f14/f16 are floats the casts used to widen. Under the double profile
		// tess_coord_t restores exactly that widening; under the float one both
		// the cast and the software add it fed disappear. This is the flowing-water
		// top face, so it runs for every fluid block of a coastline or river.
		const tess_coord_t rotU = (tess_coord_t)f16;
		const tess_coord_t rotV = (tess_coord_t)f14;
		tessellator->addVertexWithUV(i + 0, (float)j + f7, k + 0, d2 - rotU - rotV, (d3 - rotU) + rotV);
		tessellator->addVertexWithUV(i + 0, (float)j + f8, k + 1, (d2 - rotU) + rotV, d3 + rotU + rotV);
		tessellator->addVertexWithUV(i + 1, (float)j + f9, k + 1, d2 + rotU + rotV, (d3 + rotU) - rotV);
		tessellator->addVertexWithUV(i + 1, (float)j + f10, k + 0, (d2 + rotU) - rotV, d3 - rotU - rotV);
	}
	if (renderAllFaces || flag1)
	{
		tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, i, j - 1, k));
		tessellator->setColorOpaque_F(f3, f3, f3);
		renderBottomFace(block, i, j, k, block->getBlockTextureFromSide(0));
		flag2 = true;
	}
	for (int_t k1 = 0; k1 < 4; k1++)
	{
		int_t l1 = i;
		int_t j2 = j;
		int_t l2 = k;
		if (k1 == 0)
		{
			l2--;
		}
		if (k1 == 1)
		{
			l2++;
		}
		if (k1 == 2)
		{
			l1--;
		}
		if (k1 == 3)
		{
			l1++;
		}
		int_t i3 = block->getBlockTextureFromSideAndMetadata(k1 + 2, i1);
		int_t j3 = (i3 & 0xf) << 4;
		int_t k3 = i3 & 0xf0;
		if (!renderAllFaces && !aflag[k1])
		{
			continue;
		}
		float f13;
		float f15;
		float f17;
		float f19;
		float f20;
		float f21;
		if (k1 == 0)
		{
			f13 = f7;
			f15 = f10;
			f17 = i;
			f20 = i + 1;
			f19 = k;
			f21 = k;
		}
		else if (k1 == 1)
		{
			f13 = f9;
			f15 = f8;
			f17 = i + 1;
			f20 = i;
			f19 = k + 1;
			f21 = k + 1;
		}
		else if (k1 == 2)
		{
			f13 = f8;
			f15 = f7;
			f17 = i;
			f20 = i;
			f19 = k + 1;
			f21 = k;
		}
		else
		{
			f13 = f10;
			f15 = f9;
			f17 = i + 1;
			f20 = i + 1;
			f19 = k;
			f21 = k + 1;
		}
		flag2 = true;
		// d4/d6/d7 already computed in float and were only widened by the
		// assignment; only the type changes, so the double profile keeps the exact
		// values it had. d5/d8 are the two that really were double.
		tess_coord_t d4 = (float)(j3 + 0) / 256.0f;
		tess_coord_t d5 = ((tess_coord_t)(j3 + 16) - kAtlasUvGuard) / 256.0f;
		tess_coord_t d6 = ((float)k3 + (1.0f - f13) * 16.0f) / 256.0f;
		tess_coord_t d7 = ((float)k3 + (1.0f - f15) * 16.0f) / 256.0f;
		tess_coord_t d8 = ((tess_coord_t)(k3 + 16) - kAtlasUvGuard) / 256.0f;
		tessellator->setBrightness(block->getMixedBrightnessForBlock(blockAccess, l1, j2, l2));
		float f22 = 1.0f;
		if (k1 < 2)
		{
			f22 *= f5;
		}
		else
		{
			f22 *= f6;
		}
		tessellator->setColorOpaque_F(f22 * f, f22 * f1, f22 * f2);
		tessellator->addVertexWithUV(f17, (float)j + f13, f19, d4, d6);
		tessellator->addVertexWithUV(f20, (float)j + f15, f21, d5, d7);
		tessellator->addVertexWithUV(f20, j + 0, f21, d5, d8);
		tessellator->addVertexWithUV(f17, j + 0, f19, d4, d8);
	}

	block->minY = d;
	block->maxY = d1;
	return flag2;
}

float RenderBlocks::getFluidHeight(int_t i, int_t j, int_t k, Material *material)
{
	int_t l = 0;
	float f = 0.0f;
	for (int_t i1 = 0; i1 < 4; i1++)
	{
		int_t j1 = i - (i1 & 1);
		int_t k1 = j;
		int_t l1 = k - (i1 >> 1 & 1);
		if (accessGetBlockMaterial(j1, k1 + 1, l1) == material)
		{
			return 1.0f;
		}
		Material *material1 = accessGetBlockMaterial(j1, k1, l1);
		if (material1 == material)
		{
			int_t i2 = accessGetBlockMetadata(j1, k1, l1);
			if (i2 >= 8 || i2 == 0)
			{
				f += BlockFluid::getPercentAir(i2) * 10.0f;
				l += 10;
			}
			f += BlockFluid::getPercentAir(i2);
			l++;
			continue;
		}
		if (!material1->isSolid())
		{
			f++;
			l++;
		}
	}

	return 1.0f - f / (float)l;
}

void RenderBlocks::renderBlockFallingSand(Block *block, World *world, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();
	tessellator->setBrightness(block->getMixedBrightnessForBlock(world, i, j, k));
	tessellator->setColorOpaque_F(0.5f, 0.5f, 0.5f);
	renderBottomFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(0));
	tessellator->setColorOpaque_F(1.0f, 1.0f, 1.0f);
	renderTopFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(1));
	tessellator->setColorOpaque_F(0.8f, 0.8f, 0.8f);
	renderEastFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(2));
	renderWestFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(3));
	tessellator->setColorOpaque_F(0.6f, 0.6f, 0.6f);
	renderNorthFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(4));
	renderSouthFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(5));
	tessellator->draw();
}

bool RenderBlocks::renderStandardBlock(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = CustomColorizer::getColorMultiplier(block, blockAccess, i, j, k);
	float f = (float)(l >> 16 & 0xff) / 255.0f;
		float f1 = (float)(l >> 8 & 0xff) / 255.0f;
	float f2 = (float)(l & 0xff) / 255.0f;
	if (EntityRenderer::anaglyphEnabled)
	{
		float f3 = (f * 30.0f + f1 * 59.0f + f2 * 11.0f) / 100.0f;
		float f4 = (f * 30.0f + f1 * 70.0f) / 100.0f;
		float f5 = (f * 30.0f + f2 * 70.0f) / 100.0f;
		f = f3;
		f1 = f4;
		f2 = f5;
	}
	if (Minecraft::isAmbientOcclusionEnabled() && Block::lightValue[block->blockID] == 0)
	{
		return renderStandardBlockWithAmbientOcclusion(block, i, j, k, f, f1, f2);
	}
	else
	{
		return renderStandardBlockWithColorMultiplier(block, i, j, k, f, f1, f2);
	}
}

int_t RenderBlocks::fixAoSideGrassTexture(int_t texture, int_t x, int_t y, int_t z, int_t side, float red, float green, float blue)
{
	if (overrideBlockTexture >= 0 || !Config::isBetterGrass())
		return texture;

	if (texture == 3 || texture == 77)
	{
		texture = Config::getSideGrassTexture(blockAccess, x, y, z, side, texture);
		if (texture == 0)
		{
			colorRedTopLeft *= red;
			colorRedBottomLeft *= red;
			colorRedBottomRight *= red;
			colorRedTopRight *= red;
			colorGreenTopLeft *= green;
			colorGreenBottomLeft *= green;
			colorGreenBottomRight *= green;
			colorGreenTopRight *= green;
			colorBlueTopLeft *= blue;
			colorBlueBottomLeft *= blue;
			colorBlueBottomRight *= blue;
			colorBlueTopRight *= blue;
		}
	}
	if (texture == 68)
		texture = Config::getSideSnowGrassTexture(blockAccess, x, y, z, side);
	return texture;
}

bool RenderBlocks::renderStandardBlockWithAmbientOcclusion(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2)
{
	const int_t aoBaseI = i;
	const int_t aoBaseJ = j;
	const int_t aoBaseK = k;
#if PLATFORM_REUSE_AO_SCRATCH
	Ps2AoCacheScratch &aoScratch = s_ps2AoCacheScratch;
	const std::uint16_t aoCacheGeneration = aoScratch.beginGeneration();
	float *aoBrightnessCache = aoScratch.brightness;
	int_t *aoPackedBrightnessCache = aoScratch.packedBrightness;
	int_t *aoBlockIdCache = aoScratch.blockId;
#else
	float aoBrightnessCache[27] = {};
	unsigned char aoBrightnessValid[27] = {};
	int_t aoPackedBrightnessCache[27] = {};
	unsigned char aoPackedBrightnessValid[27] = {};
	int_t aoBlockIdCache[27] = {};
	unsigned char aoBlockIdValid[27] = {};
#endif

	auto aoCacheIndex = [aoBaseI, aoBaseJ, aoBaseK](int_t x, int_t y, int_t z) -> int {
		const int dx = static_cast<int>(x - aoBaseI);
		const int dy = static_cast<int>(y - aoBaseJ);
		const int dz = static_cast<int>(z - aoBaseK);
		if (dx < -1 || dx > 1 || dy < -1 || dy > 1 || dz < -1 || dz > 1) return -1;
		return (dy + 1) * 9 + (dz + 1) * 3 + (dx + 1);
	};

	auto aoBrightnessAt = [&](int_t x, int_t y, int_t z) -> float {
		const int idx = aoCacheIndex(x, y, z);
		if (idx < 0) return block->getBlockBrightness(blockAccess, x, y, z);
#if PLATFORM_REUSE_AO_SCRATCH
		if (aoScratch.brightnessStamp[idx] != aoCacheGeneration) {
			aoBrightnessCache[idx] = block->getBlockBrightness(blockAccess, x, y, z);
			aoScratch.brightnessStamp[idx] = aoCacheGeneration;
		}
#else
		if (!aoBrightnessValid[idx]) {
			aoBrightnessCache[idx] = block->getBlockBrightness(blockAccess, x, y, z);
			aoBrightnessValid[idx] = 1;
		}
#endif
		return aoBrightnessCache[idx];
	};

	auto aoPackedBrightnessAt = [&](int_t x, int_t y, int_t z) -> int_t {
		const int idx = aoCacheIndex(x, y, z);
		if (idx < 0) return block->getMixedBrightnessForBlock(blockAccess, x, y, z);
#if PLATFORM_REUSE_AO_SCRATCH
		if (aoScratch.packedBrightnessStamp[idx] != aoCacheGeneration) {
			aoPackedBrightnessCache[idx] = block->getMixedBrightnessForBlock(blockAccess, x, y, z);
			aoScratch.packedBrightnessStamp[idx] = aoCacheGeneration;
		}
#else
		if (!aoPackedBrightnessValid[idx]) {
			aoPackedBrightnessCache[idx] = block->getMixedBrightnessForBlock(blockAccess, x, y, z);
			aoPackedBrightnessValid[idx] = 1;
		}
#endif
		return aoPackedBrightnessCache[idx];
	};

	auto aoBlockIdAt = [&](int_t x, int_t y, int_t z) -> int_t {
		const int idx = aoCacheIndex(x, y, z);
		if (idx < 0) return accessGetBlockId(x, y, z);
#if PLATFORM_REUSE_AO_SCRATCH
		if (aoScratch.blockIdStamp[idx] != aoCacheGeneration) {
			aoBlockIdCache[idx] = accessGetBlockId(x, y, z);
			aoScratch.blockIdStamp[idx] = aoCacheGeneration;
		}
#else
		if (!aoBlockIdValid[idx]) {
			aoBlockIdCache[idx] = accessGetBlockId(x, y, z);
			aoBlockIdValid[idx] = 1;
		}
#endif
		return aoBlockIdCache[idx];
	};

	auto aoCanBlockGrassAt = [&](int_t x, int_t y, int_t z) -> bool {
		return Block::canBlockGrass[aoBlockIdAt(x, y, z)];
	};

	const bool aoFullCubeBounds = block->minX == 0.0 && block->minY == 0.0 && block->minZ == 0.0 &&
		block->maxX == 1.0 && block->maxY == 1.0 && block->maxZ == 1.0;
	auto aoSideVisible = [&](int_t x, int_t y, int_t z, int_t side) -> bool {
		if (aoFullCubeBounds) return !accessIsBlockOpaqueCube(x, y, z);
		return shouldRenderFace(block, x, y, z, side);
	};

	// AO scratch is local so PowerPC can keep hot values in registers.
	float aoLightValueXNeg = 0.0f;
	float aoLightValueYNeg = 0.0f;
	float aoLightValueZNeg = 0.0f;
	float aoLightValueXPos = 0.0f;
	float aoLightValueYPos = 0.0f;
	float aoLightValueZPos = 0.0f;
	float aoLightValueXYZNNN = 0.0f;
	float aoLightValueXNeg2 = 0.0f;
	float aoLightValueXYZNNP = 0.0f;
	float aoLightValueYNeg2 = 0.0f;
	float aoLightValueXYZPNN = 0.0f;
	float aoLightValueXYZPNP = 0.0f;
	float aoLightValueXPos2 = 0.0f;
	float aoLightValueXYZPPP = 0.0f;
	float aoLightValueXYZNNP2 = 0.0f;
	float aoLightValueXPosUp = 0.0f;
	float aoLightValueXYZPNP2 = 0.0f;
	float aoLightValueZNegUp = 0.0f;
	float aoLightValueXYZPPN = 0.0f;
	float aoLightValueXPosUp2 = 0.0f;
	float aoLightValueZPosUp = 0.0f;
	float aoLightValueXYZPPP2 = 0.0f;
	float aoLightValueXNegNorth = 0.0f;
	float aoLightValueXPosNorth = 0.0f;
	float aoLightValueXNegSouth = 0.0f;
	float aoLightValueXPosSouth = 0.0f;

	enableAO = true;
	bool flag = false;
	bool flag1 = true;
	bool flag2 = true;
	bool flag3 = true;
	bool flag4 = true;
	bool flag5 = true;
	bool flag6 = true;
	aoLightValueXNeg = aoBrightnessAt( i - 1, j, k);
	aoLightValueYNeg = aoBrightnessAt( i, j - 1, k);
	aoLightValueZNeg = aoBrightnessAt( i, j, k - 1);
	aoLightValueXPos = aoBrightnessAt( i + 1, j, k);
	aoLightValueYPos = aoBrightnessAt( i, j + 1, k);
	aoLightValueZPos = aoBrightnessAt( i, j, k + 1);
	const int_t packedOwn = aoPackedBrightnessAt(i, j, k);
	const int_t packedXNeg = block->minX <= 0.0 ? aoPackedBrightnessAt(i - 1, j, k) : packedOwn;
	const int_t packedYNeg = block->minY <= 0.0 ? aoPackedBrightnessAt(i, j - 1, k) : packedOwn;
	const int_t packedZNeg = block->minZ <= 0.0 ? aoPackedBrightnessAt(i, j, k - 1) : packedOwn;
	const int_t packedXPos = block->maxX >= 1.0 ? aoPackedBrightnessAt(i + 1, j, k) : packedOwn;
	const int_t packedYPos = block->maxY >= 1.0 ? aoPackedBrightnessAt(i, j + 1, k) : packedOwn;
	const int_t packedZPos = block->maxZ >= 1.0 ? aoPackedBrightnessAt(i, j, k + 1) : packedOwn;
	Tessellator::instance.setBrightness(983055);
	if (block->blockIndexInTexture == 3)
	{
		flag1 = flag3 = flag4 = flag5 = flag6 = false;
	}
	if (overrideBlockTexture >= 0)
	{
		flag1 = flag3 = flag4 = flag5 = flag6 = false;
	}
	if (renderAllFaces || aoSideVisible(i, j - 1, k, 0))
	{
		float f4;
		float f11;
		float f18;
		float f25;
		if (lightingQuality > 0)
		{
			j--;
			aoLightValueXNeg2 = aoBrightnessAt( i - 1, j, k);
			aoLightValueYNeg2 = aoBrightnessAt( i, j, k - 1);
			aoLightValueXYZPNN = aoBrightnessAt( i, j, k + 1);
			aoLightValueXPos2 = aoBrightnessAt( i + 1, j, k);
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK))
			{
				aoLightValueXYZNNN = aoBrightnessAt( i - 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZNNN = aoLightValueXNeg2;
			}
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK))
			{
				aoLightValueXYZNNP = aoBrightnessAt( i - 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZNNP = aoLightValueXNeg2;
			}
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK))
			{
				aoLightValueXYZPNP = aoBrightnessAt( i + 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZPNP = aoLightValueXPos2;
			}
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK))
			{
				aoLightValueXYZPPP = aoBrightnessAt( i + 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZPPP = aoLightValueXPos2;
			}
			j++;
			f4 = (aoLightValueXYZNNP + aoLightValueXNeg2 + aoLightValueXYZPNN + aoLightValueYNeg) / 4.0f;
			f25 = (aoLightValueXYZPNN + aoLightValueYNeg + aoLightValueXYZPPP + aoLightValueXPos2) / 4.0f;
			f18 = (aoLightValueYNeg + aoLightValueYNeg2 + aoLightValueXPos2 + aoLightValueXYZPNP) / 4.0f;
			f11 = (aoLightValueXNeg2 + aoLightValueXYZNNN + aoLightValueYNeg + aoLightValueYNeg2) / 4.0f;
		}
		else
		{
			f4 = f11 = f18 = f25 = aoLightValueYNeg;
		}
		{
			const int_t y = aoBaseJ + (block->minY <= 0.0 ? -1 : 0);
			const int_t xyNN = aoPackedBrightnessAt(aoBaseI - 1, y, aoBaseK);
			const int_t yzNN = aoPackedBrightnessAt(aoBaseI, y, aoBaseK - 1);
			const int_t yzNP = aoPackedBrightnessAt(aoBaseI, y, aoBaseK + 1);
			const int_t xyPN = aoPackedBrightnessAt(aoBaseI + 1, y, aoBaseK);
			const int_t xyzNNN = (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI - 1, y, aoBaseK - 1) : xyNN;
			const int_t xyzNNP = (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI - 1, y, aoBaseK + 1) : xyNN;
			const int_t xyzPNN = (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI + 1, y, aoBaseK - 1) : xyPN;
			const int_t xyzPNP = (aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI + 1, y, aoBaseK + 1) : xyPN;
			if (lightingQuality > 0) {
				brightnessTopLeft = getAoBrightness(xyzNNP, xyNN, yzNP, packedYNeg);
				brightnessTopRight = getAoBrightness(yzNP, xyzPNP, xyPN, packedYNeg);
				brightnessBottomRight = getAoBrightness(yzNN, xyPN, xyzPNN, packedYNeg);
				brightnessBottomLeft = getAoBrightness(xyNN, xyzNNN, yzNN, packedYNeg);
			} else {
				brightnessTopLeft = brightnessBottomLeft = brightnessBottomRight = brightnessTopRight = packedYNeg;
			}
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag1 ? f : 1.0f) * 0.5f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag1 ? f1 : 1.0f) * 0.5f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag1 ? f2 : 1.0f) * 0.5f;
		colorRedTopLeft *= f4;
		colorGreenTopLeft *= f4;
		colorBlueTopLeft *= f4;
		colorRedBottomLeft *= f11;
		colorGreenBottomLeft *= f11;
		colorBlueBottomLeft *= f11;
		colorRedBottomRight *= f18;
		colorGreenBottomRight *= f18;
		colorBlueBottomRight *= f18;
		colorRedTopRight *= f25;
		colorGreenTopRight *= f25;
		colorBlueTopRight *= f25;
		renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
		flag = true;
	}
	if (renderAllFaces || aoSideVisible(i, j + 1, k, 1))
	{
		float f5;
		float f12;
		float f19;
		float f26;
		if (lightingQuality > 0)
		{
			j++;
			aoLightValueXPosUp = aoBrightnessAt( i - 1, j, k);
			aoLightValueXPosUp2 = aoBrightnessAt( i + 1, j, k);
			aoLightValueZNegUp = aoBrightnessAt( i, j, k - 1);
			aoLightValueZPosUp = aoBrightnessAt( i, j, k + 1);
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK))
			{
				aoLightValueXYZNNP2 = aoBrightnessAt( i - 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZNNP2 = aoLightValueXPosUp;
			}
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK))
			{
				aoLightValueXYZPPN = aoBrightnessAt( i + 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZPPN = aoLightValueXPosUp2;
			}
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK))
			{
				aoLightValueXYZPNP2 = aoBrightnessAt( i - 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZPNP2 = aoLightValueXPosUp;
			}
			if (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK))
			{
				aoLightValueXYZPPP2 = aoBrightnessAt( i + 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZPPP2 = aoLightValueXPosUp2;
			}
			j--;
			f26 = (aoLightValueXYZPNP2 + aoLightValueXPosUp + aoLightValueZPosUp + aoLightValueYPos) / 4.0f;
			f5 = (aoLightValueZPosUp + aoLightValueYPos + aoLightValueXYZPPP2 + aoLightValueXPosUp2) / 4.0f;
			f12 = (aoLightValueYPos + aoLightValueZNegUp + aoLightValueXPosUp2 + aoLightValueXYZPPN) / 4.0f;
			f19 = (aoLightValueXPosUp + aoLightValueXYZNNP2 + aoLightValueYPos + aoLightValueZNegUp) / 4.0f;
		}
		else
		{
			f5 = f12 = f19 = f26 = aoLightValueYPos;
		}
		{
			const int_t y = aoBaseJ + (block->maxY >= 1.0 ? 1 : 0);
			const int_t xyNP = aoPackedBrightnessAt(aoBaseI - 1, y, aoBaseK);
			const int_t xyPP = aoPackedBrightnessAt(aoBaseI + 1, y, aoBaseK);
			const int_t yzPN = aoPackedBrightnessAt(aoBaseI, y, aoBaseK - 1);
			const int_t yzPP = aoPackedBrightnessAt(aoBaseI, y, aoBaseK + 1);
			const int_t xyzNPN = (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI - 1, y, aoBaseK - 1) : xyNP;
			const int_t xyzNPP = (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI - 1, y, aoBaseK + 1) : xyNP;
			const int_t xyzPPN = (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI + 1, y, aoBaseK - 1) : xyPP;
			const int_t xyzPPP = (aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK)) ? aoPackedBrightnessAt(aoBaseI + 1, y, aoBaseK + 1) : xyPP;
			if (lightingQuality > 0) {
				brightnessTopRight = getAoBrightness(xyzNPP, xyNP, yzPP, packedYPos);
				brightnessTopLeft = getAoBrightness(yzPP, xyzPPP, xyPP, packedYPos);
				brightnessBottomLeft = getAoBrightness(yzPN, xyPP, xyzPPN, packedYPos);
				brightnessBottomRight = getAoBrightness(xyNP, xyzNPN, yzPN, packedYPos);
			} else {
				brightnessTopLeft = brightnessBottomLeft = brightnessBottomRight = brightnessTopRight = packedYPos;
			}
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = flag2 ? f : 1.0f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = flag2 ? f1 : 1.0f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = flag2 ? f2 : 1.0f;
		colorRedTopLeft *= f5;
		colorGreenTopLeft *= f5;
		colorBlueTopLeft *= f5;
		colorRedBottomLeft *= f12;
		colorGreenBottomLeft *= f12;
		colorBlueBottomLeft *= f12;
		colorRedBottomRight *= f19;
		colorGreenBottomRight *= f19;
		colorBlueBottomRight *= f19;
		colorRedTopRight *= f26;
		colorGreenTopRight *= f26;
		colorBlueTopRight *= f26;
		renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
		flag = true;
	}
	if (renderAllFaces || aoSideVisible(i, j, k - 1, 2))
	{
		float f6;
		float f13;
		float f20;
		float f27;
		if (lightingQuality > 0)
		{
			k--;
			aoLightValueXNegNorth = aoBrightnessAt( i - 1, j, k);
			aoLightValueYNeg2 = aoBrightnessAt( i, j - 1, k);
			aoLightValueZNegUp = aoBrightnessAt( i, j + 1, k);
			aoLightValueXPosNorth = aoBrightnessAt( i + 1, j, k);
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1))
			{
				aoLightValueXYZNNN = aoBrightnessAt( i - 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZNNN = aoLightValueXNegNorth;
			}
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1))
			{
				aoLightValueXYZNNP2 = aoBrightnessAt( i - 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZNNP2 = aoLightValueXNegNorth;
			}
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1))
			{
				aoLightValueXYZPNP = aoBrightnessAt( i + 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZPNP = aoLightValueXPosNorth;
			}
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1))
			{
				aoLightValueXYZPPN = aoBrightnessAt( i + 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZPPN = aoLightValueXPosNorth;
			}
			k++;
			f6 = (aoLightValueXNegNorth + aoLightValueXYZNNP2 + aoLightValueZNeg + aoLightValueZNegUp) / 4.0f;
			f13 = (aoLightValueZNeg + aoLightValueZNegUp + aoLightValueXPosNorth + aoLightValueXYZPPN) / 4.0f;
			f20 = (aoLightValueYNeg2 + aoLightValueZNeg + aoLightValueXYZPNP + aoLightValueXPosNorth) / 4.0f;
			f27 = (aoLightValueXYZNNN + aoLightValueXNegNorth + aoLightValueYNeg2 + aoLightValueZNeg) / 4.0f;
		}
		else
		{
			f6 = f13 = f20 = f27 = aoLightValueZNeg;
		}
		{
			const int_t z = aoBaseK + (block->minZ <= 0.0 ? -1 : 0);
			const int_t xzNN = aoPackedBrightnessAt(aoBaseI - 1, aoBaseJ, z);
			const int_t yzNN = aoPackedBrightnessAt(aoBaseI, aoBaseJ - 1, z);
			const int_t yzPN = aoPackedBrightnessAt(aoBaseI, aoBaseJ + 1, z);
			const int_t xzPN = aoPackedBrightnessAt(aoBaseI + 1, aoBaseJ, z);
			const int_t xyzNNN = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1)) ? aoPackedBrightnessAt(aoBaseI - 1, aoBaseJ - 1, z) : xzNN;
			const int_t xyzNPN = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1)) ? aoPackedBrightnessAt(aoBaseI - 1, aoBaseJ + 1, z) : xzNN;
			const int_t xyzPNN = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK - 1)) ? aoPackedBrightnessAt(aoBaseI + 1, aoBaseJ - 1, z) : xzPN;
			const int_t xyzPPN = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK - 1)) ? aoPackedBrightnessAt(aoBaseI + 1, aoBaseJ + 1, z) : xzPN;
			if (lightingQuality > 0) {
				brightnessTopLeft = getAoBrightness(xzNN, xyzNPN, yzPN, packedZNeg);
				brightnessBottomLeft = getAoBrightness(yzPN, xzPN, xyzPPN, packedZNeg);
				brightnessBottomRight = getAoBrightness(yzNN, xyzPNN, xzPN, packedZNeg);
				brightnessTopRight = getAoBrightness(xyzNNN, xzNN, yzNN, packedZNeg);
			} else {
				brightnessTopLeft = brightnessBottomLeft = brightnessBottomRight = brightnessTopRight = packedZNeg;
			}
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag3 ? f : 1.0f) * 0.8f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag3 ? f1 : 1.0f) * 0.8f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag3 ? f2 : 1.0f) * 0.8f;
		colorRedTopLeft *= f6;
		colorGreenTopLeft *= f6;
		colorBlueTopLeft *= f6;
		colorRedBottomLeft *= f13;
		colorGreenBottomLeft *= f13;
		colorBlueBottomLeft *= f13;
		colorRedBottomRight *= f20;
		colorGreenBottomRight *= f20;
		colorBlueBottomRight *= f20;
		colorRedTopRight *= f27;
		colorGreenTopRight *= f27;
		colorBlueTopRight *= f27;
		int_t l = block->getBlockTexture(blockAccess, i, j, k, 2);
		l = fixAoSideGrassTexture(l, i, j, k, 2, f, f1, f2);
		renderEastFace(block, i, j, k, l);
		if (fancyGrass && l == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderEastFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || aoSideVisible(i, j, k + 1, 3))
	{
		float f7;
		float f14;
		float f21;
		float f28;
		if (lightingQuality > 0)
		{
			k++;
			aoLightValueXNegSouth = aoBrightnessAt( i - 1, j, k);
			aoLightValueXPosSouth = aoBrightnessAt( i + 1, j, k);
			aoLightValueXYZPNN = aoBrightnessAt( i, j - 1, k);
			aoLightValueZPosUp = aoBrightnessAt( i, j + 1, k);
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1))
			{
				aoLightValueXYZNNP = aoBrightnessAt( i - 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZNNP = aoLightValueXNegSouth;
			}
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1))
			{
				aoLightValueXYZPNP2 = aoBrightnessAt( i - 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZPNP2 = aoLightValueXNegSouth;
			}
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1))
			{
				aoLightValueXYZPPP = aoBrightnessAt( i + 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZPPP = aoLightValueXPosSouth;
			}
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1))
			{
				aoLightValueXYZPPP2 = aoBrightnessAt( i + 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZPPP2 = aoLightValueXPosSouth;
			}
			k--;
			f7 = (aoLightValueXNegSouth + aoLightValueXYZPNP2 + aoLightValueZPos + aoLightValueZPosUp) / 4.0f;
			f28 = (aoLightValueZPos + aoLightValueZPosUp + aoLightValueXPosSouth + aoLightValueXYZPPP2) / 4.0f;
			f21 = (aoLightValueXYZPNN + aoLightValueZPos + aoLightValueXYZPPP + aoLightValueXPosSouth) / 4.0f;
			f14 = (aoLightValueXYZNNP + aoLightValueXNegSouth + aoLightValueXYZPNN + aoLightValueZPos) / 4.0f;
		}
		else
		{
			f7 = f14 = f21 = f28 = aoLightValueZPos;
		}
		{
			const int_t z = aoBaseK + (block->maxZ >= 1.0 ? 1 : 0);
			const int_t xzNP = aoPackedBrightnessAt(aoBaseI - 1, aoBaseJ, z);
			const int_t xzPP = aoPackedBrightnessAt(aoBaseI + 1, aoBaseJ, z);
			const int_t yzNP = aoPackedBrightnessAt(aoBaseI, aoBaseJ - 1, z);
			const int_t yzPP = aoPackedBrightnessAt(aoBaseI, aoBaseJ + 1, z);
			const int_t xyzNNP = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1)) ? aoPackedBrightnessAt(aoBaseI - 1, aoBaseJ - 1, z) : xzNP;
			const int_t xyzNPP = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1)) ? aoPackedBrightnessAt(aoBaseI - 1, aoBaseJ + 1, z) : xzNP;
			const int_t xyzPNP = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ - 1, aoBaseK + 1)) ? aoPackedBrightnessAt(aoBaseI + 1, aoBaseJ - 1, z) : xzPP;
			const int_t xyzPPP = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI, aoBaseJ + 1, aoBaseK + 1)) ? aoPackedBrightnessAt(aoBaseI + 1, aoBaseJ + 1, z) : xzPP;
			if (lightingQuality > 0) {
				brightnessTopLeft = getAoBrightness(xzNP, xyzNPP, yzPP, packedZPos);
				brightnessTopRight = getAoBrightness(yzPP, xzPP, xyzPPP, packedZPos);
				brightnessBottomRight = getAoBrightness(yzNP, xyzPNP, xzPP, packedZPos);
				brightnessBottomLeft = getAoBrightness(xyzNNP, xzNP, yzNP, packedZPos);
			} else {
				brightnessTopLeft = brightnessBottomLeft = brightnessBottomRight = brightnessTopRight = packedZPos;
			}
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag4 ? f : 1.0f) * 0.8f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag4 ? f1 : 1.0f) * 0.8f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag4 ? f2 : 1.0f) * 0.8f;
		colorRedTopLeft *= f7;
		colorGreenTopLeft *= f7;
		colorBlueTopLeft *= f7;
		colorRedBottomLeft *= f14;
		colorGreenBottomLeft *= f14;
		colorBlueBottomLeft *= f14;
		colorRedBottomRight *= f21;
		colorGreenBottomRight *= f21;
		colorBlueBottomRight *= f21;
		colorRedTopRight *= f28;
		colorGreenTopRight *= f28;
		colorBlueTopRight *= f28;
		int_t i1 = block->getBlockTexture(blockAccess, i, j, k, 3);
		i1 = fixAoSideGrassTexture(i1, i, j, k, 3, f, f1, f2);
		renderWestFace(block, i, j, k, i1);
		if (fancyGrass && i1 == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderWestFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || aoSideVisible(i - 1, j, k, 4))
	{
		float f8;
		float f15;
		float f22;
		float f29;
		if (lightingQuality > 0)
		{
			i--;
			aoLightValueXNeg2 = aoBrightnessAt( i, j - 1, k);
			aoLightValueXNegNorth = aoBrightnessAt( i, j, k - 1);
			aoLightValueXNegSouth = aoBrightnessAt( i, j, k + 1);
			aoLightValueXPosUp = aoBrightnessAt( i, j + 1, k);
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK))
			{
				aoLightValueXYZNNN = aoBrightnessAt( i, j - 1, k - 1);
			}
			else
			{
				aoLightValueXYZNNN = aoLightValueXNegNorth;
			}
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK))
			{
				aoLightValueXYZNNP = aoBrightnessAt( i, j - 1, k + 1);
			}
			else
			{
				aoLightValueXYZNNP = aoLightValueXNegSouth;
			}
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK))
			{
				aoLightValueXYZNNP2 = aoBrightnessAt( i, j + 1, k - 1);
			}
			else
			{
				aoLightValueXYZNNP2 = aoLightValueXNegNorth;
			}
			if (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK))
			{
				aoLightValueXYZPNP2 = aoBrightnessAt( i, j + 1, k + 1);
			}
			else
			{
				aoLightValueXYZPNP2 = aoLightValueXNegSouth;
			}
			i++;
			f29 = (aoLightValueXNeg2 + aoLightValueXYZNNP + aoLightValueXNeg + aoLightValueXNegSouth) / 4.0f;
			f8 = (aoLightValueXNeg + aoLightValueXNegSouth + aoLightValueXPosUp + aoLightValueXYZPNP2) / 4.0f;
			f15 = (aoLightValueXNegNorth + aoLightValueXNeg + aoLightValueXYZNNP2 + aoLightValueXPosUp) / 4.0f;
			f22 = (aoLightValueXYZNNN + aoLightValueXNeg2 + aoLightValueXNegNorth + aoLightValueXNeg) / 4.0f;
		}
		else
		{
			f8 = f15 = f22 = f29 = aoLightValueXNeg;
		}
		{
			const int_t x = aoBaseI + (block->minX <= 0.0 ? -1 : 0);
			const int_t xyNN = aoPackedBrightnessAt(x, aoBaseJ - 1, aoBaseK);
			const int_t xzNN = aoPackedBrightnessAt(x, aoBaseJ, aoBaseK - 1);
			const int_t xzNP = aoPackedBrightnessAt(x, aoBaseJ, aoBaseK + 1);
			const int_t xyNP = aoPackedBrightnessAt(x, aoBaseJ + 1, aoBaseK);
			const int_t xyzNNN = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK)) ? aoPackedBrightnessAt(x, aoBaseJ - 1, aoBaseK - 1) : xzNN;
			const int_t xyzNNP = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ - 1, aoBaseK)) ? aoPackedBrightnessAt(x, aoBaseJ - 1, aoBaseK + 1) : xzNP;
			const int_t xyzNPN = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK - 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK)) ? aoPackedBrightnessAt(x, aoBaseJ + 1, aoBaseK - 1) : xzNN;
			const int_t xyzNPP = (aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ, aoBaseK + 1) || aoCanBlockGrassAt(aoBaseI - 1, aoBaseJ + 1, aoBaseK)) ? aoPackedBrightnessAt(x, aoBaseJ + 1, aoBaseK + 1) : xzNP;
			if (lightingQuality > 0) {
				brightnessTopRight = getAoBrightness(xyNN, xyzNNP, xzNP, packedXNeg);
				brightnessTopLeft = getAoBrightness(xzNP, xyNP, xyzNPP, packedXNeg);
				brightnessBottomLeft = getAoBrightness(xzNN, xyzNPN, xyNP, packedXNeg);
				brightnessBottomRight = getAoBrightness(xyzNNN, xyNN, xzNN, packedXNeg);
			} else {
				brightnessTopLeft = brightnessBottomLeft = brightnessBottomRight = brightnessTopRight = packedXNeg;
			}
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag5 ? f : 1.0f) * 0.6f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag5 ? f1 : 1.0f) * 0.6f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag5 ? f2 : 1.0f) * 0.6f;
		colorRedTopLeft *= f8;
		colorGreenTopLeft *= f8;
		colorBlueTopLeft *= f8;
		colorRedBottomLeft *= f15;
		colorGreenBottomLeft *= f15;
		colorBlueBottomLeft *= f15;
		colorRedBottomRight *= f22;
		colorGreenBottomRight *= f22;
		colorBlueBottomRight *= f22;
		colorRedTopRight *= f29;
		colorGreenTopRight *= f29;
		colorBlueTopRight *= f29;
		int_t j1 = block->getBlockTexture(blockAccess, i, j, k, 4);
		j1 = fixAoSideGrassTexture(j1, i, j, k, 4, f, f1, f2);
		renderNorthFace(block, i, j, k, j1);
		if (fancyGrass && j1 == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderNorthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || aoSideVisible(i + 1, j, k, 5))
	{
		float f9;
		float f16;
		float f23;
		float f30;
		if (lightingQuality > 0)
		{
			i++;
			aoLightValueXPos2 = aoBrightnessAt( i, j - 1, k);
			aoLightValueXPosNorth = aoBrightnessAt( i, j, k - 1);
			aoLightValueXPosSouth = aoBrightnessAt( i, j, k + 1);
			aoLightValueXPosUp2 = aoBrightnessAt( i, j + 1, k);
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1))
			{
				aoLightValueXYZPNP = aoBrightnessAt( i, j - 1, k - 1);
			}
			else
			{
				aoLightValueXYZPNP = aoLightValueXPosNorth;
			}
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1))
			{
				aoLightValueXYZPPP = aoBrightnessAt( i, j - 1, k + 1);
			}
			else
			{
				aoLightValueXYZPPP = aoLightValueXPosSouth;
			}
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1))
			{
				aoLightValueXYZPPN = aoBrightnessAt( i, j + 1, k - 1);
			}
			else
			{
				aoLightValueXYZPPN = aoLightValueXPosNorth;
			}
			if (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1))
			{
				aoLightValueXYZPPP2 = aoBrightnessAt( i, j + 1, k + 1);
			}
			else
			{
				aoLightValueXYZPPP2 = aoLightValueXPosSouth;
			}
			i--;
			f9 = (aoLightValueXPos2 + aoLightValueXYZPPP + aoLightValueXPos + aoLightValueXPosSouth) / 4.0f;
			f30 = (aoLightValueXPos + aoLightValueXPosSouth + aoLightValueXPosUp2 + aoLightValueXYZPPP2) / 4.0f;
			f23 = (aoLightValueXPosNorth + aoLightValueXPos + aoLightValueXYZPPN + aoLightValueXPosUp2) / 4.0f;
			f16 = (aoLightValueXYZPNP + aoLightValueXPos2 + aoLightValueXPosNorth + aoLightValueXPos) / 4.0f;
		}
		else
		{
			f9 = f16 = f23 = f30 = aoLightValueXPos;
		}
		{
			const int_t x = aoBaseI + (block->maxX >= 1.0 ? 1 : 0);
			const int_t xyPN = aoPackedBrightnessAt(x, aoBaseJ - 1, aoBaseK);
			const int_t xzPN = aoPackedBrightnessAt(x, aoBaseJ, aoBaseK - 1);
			const int_t xzPP = aoPackedBrightnessAt(x, aoBaseJ, aoBaseK + 1);
			const int_t xyPP = aoPackedBrightnessAt(x, aoBaseJ + 1, aoBaseK);
			const int_t xyzPNN = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1)) ? aoPackedBrightnessAt(x, aoBaseJ - 1, aoBaseK - 1) : xzPN;
			const int_t xyzPNP = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ - 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1)) ? aoPackedBrightnessAt(x, aoBaseJ - 1, aoBaseK + 1) : xzPP;
			const int_t xyzPPN = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK - 1)) ? aoPackedBrightnessAt(x, aoBaseJ + 1, aoBaseK - 1) : xzPN;
			const int_t xyzPPP = (aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ + 1, aoBaseK) || aoCanBlockGrassAt(aoBaseI + 1, aoBaseJ, aoBaseK + 1)) ? aoPackedBrightnessAt(x, aoBaseJ + 1, aoBaseK + 1) : xzPP;
			if (lightingQuality > 0) {
				brightnessTopLeft = getAoBrightness(xyPN, xyzPNP, xzPP, packedXPos);
				brightnessTopRight = getAoBrightness(xzPP, xyPP, xyzPPP, packedXPos);
				brightnessBottomRight = getAoBrightness(xzPN, xyzPPN, xyPP, packedXPos);
				brightnessBottomLeft = getAoBrightness(xyzPNN, xyPN, xzPN, packedXPos);
			} else {
				brightnessTopLeft = brightnessBottomLeft = brightnessBottomRight = brightnessTopRight = packedXPos;
			}
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag6 ? f : 1.0f) * 0.6f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag6 ? f1 : 1.0f) * 0.6f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag6 ? f2 : 1.0f) * 0.6f;
		colorRedTopLeft *= f9;
		colorGreenTopLeft *= f9;
		colorBlueTopLeft *= f9;
		colorRedBottomLeft *= f16;
		colorGreenBottomLeft *= f16;
		colorBlueBottomLeft *= f16;
		colorRedBottomRight *= f23;
		colorGreenBottomRight *= f23;
		colorBlueBottomRight *= f23;
		colorRedTopRight *= f30;
		colorGreenTopRight *= f30;
		colorBlueTopRight *= f30;
		int_t k1 = block->getBlockTexture(blockAccess, i, j, k, 5);
		k1 = fixAoSideGrassTexture(k1, i, j, k, 5, f, f1, f2);
		renderSouthFace(block, i, j, k, k1);
		if (fancyGrass && k1 == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderSouthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	enableAO = false;
	return flag;
}

int_t RenderBlocks::getAoBrightness(int_t a, int_t b, int_t c, int_t base) const
{
	if (a == 0) a = base;
	if (b == 0) b = base;
	if (c == 0) c = base;
	return ((a + b + c + base) >> 2) & 0x00ff00ff;
}

bool RenderBlocks::renderStandardBlockWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2)
{
	enableAO = false;
	Tessellator *tessellator = &Tessellator::instance;
	bool flag = false;
	float f3 = 0.5f;
	float f4 = 1.0f;
	float f5 = 0.8f;
	float f6 = 0.6f;
	float f7 = f4 * f;
	float f8 = f4 * f1;
	float f9 = f4 * f2;
	float f10 = f3;
	float f11 = f5;
	float f12 = f6;
	float f13 = f3;
	float f14 = f5;
	float f15 = f6;
	float f16 = f3;
	float f17 = f5;
	float f18 = f6;
	if (block != static_cast<Block*>(Block::grass))
	{
		f10 *= f;
		f11 *= f;
		f12 *= f;
		f13 *= f1;
		f14 *= f1;
		f15 *= f1;
		f16 *= f2;
		f17 *= f2;
		f18 *= f2;
	}
	float f19 = block->getBlockBrightness(blockAccess, i, j, k);
	(void)f19;
	const int_t packedBrightness = block->getMixedBrightnessForBlock(blockAccess, i, j, k);
	if (renderAllFaces || shouldRenderFace(block, i, j - 1, k, 0))
	{
		float f20 = block->getBlockBrightness(blockAccess, i, j - 1, k);
		(void)f20;
		tessellator->setBrightness(block->minY > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j - 1, k));
		tessellator->setColorOpaque_F(f10, f13, f16);
		renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i, j + 1, k, 1))
	{
		float f21 = block->getBlockBrightness(blockAccess, i, j + 1, k);
		(void)f21;
		tessellator->setBrightness(block->maxY < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j + 1, k));
		tessellator->setColorOpaque_F(f7, f8, f9);
		renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i, j, k - 1, 2))
	{
		float f22 = block->getBlockBrightness(blockAccess, i, j, k - 1);
		(void)f22;
		tessellator->setBrightness(block->minZ > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k - 1));
		tessellator->setColorOpaque_F(f11, f14, f17);
		int_t l = block->getBlockTexture(blockAccess, i, j, k, 2);
		if (overrideBlockTexture < 0 && Config::isBetterGrass())
		{
			if (l == 3 || l == 77)
			{
				l = Config::getSideGrassTexture(blockAccess, i, j, k, 2, l);
				if (l == 0)
					tessellator->setColorOpaque_F(f11 * f, f14 * f1, f17 * f2);
			}
			if (l == 68)
				l = Config::getSideSnowGrassTexture(blockAccess, i, j, k, 2);
		}
		renderEastFace(block, i, j, k, l);
		if (fancyGrass && l == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			tessellator->setColorOpaque_F(f11 * f, f14 * f1, f17 * f2);
			renderEastFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i, j, k + 1, 3))
	{
		float f23 = block->getBlockBrightness(blockAccess, i, j, k + 1);
		(void)f23;
		tessellator->setBrightness(block->maxZ < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k + 1));
		tessellator->setColorOpaque_F(f11, f14, f17);
		int_t i1 = block->getBlockTexture(blockAccess, i, j, k, 3);
		if (overrideBlockTexture < 0 && Config::isBetterGrass())
		{
			if (i1 == 3 || i1 == 77)
			{
				i1 = Config::getSideGrassTexture(blockAccess, i, j, k, 3, i1);
				if (i1 == 0)
					tessellator->setColorOpaque_F(f11 * f, f14 * f1, f17 * f2);
			}
			if (i1 == 68)
				i1 = Config::getSideSnowGrassTexture(blockAccess, i, j, k, 3);
		}
		renderWestFace(block, i, j, k, i1);
		if (fancyGrass && i1 == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			tessellator->setColorOpaque_F(f11 * f, f14 * f1, f17 * f2);
			renderWestFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i - 1, j, k, 4))
	{
		float f24 = block->getBlockBrightness(blockAccess, i - 1, j, k);
		(void)f24;
		tessellator->setBrightness(block->minX > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i - 1, j, k));
		tessellator->setColorOpaque_F(f12, f15, f18);
		int_t j1 = block->getBlockTexture(blockAccess, i, j, k, 4);
		if (overrideBlockTexture < 0 && Config::isBetterGrass())
		{
			if (j1 == 3 || j1 == 77)
			{
				j1 = Config::getSideGrassTexture(blockAccess, i, j, k, 4, j1);
				if (j1 == 0)
					tessellator->setColorOpaque_F(f12 * f, f15 * f1, f18 * f2);
			}
			if (j1 == 68)
				j1 = Config::getSideSnowGrassTexture(blockAccess, i, j, k, 4);
		}
		renderNorthFace(block, i, j, k, j1);
		if (fancyGrass && j1 == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			tessellator->setColorOpaque_F(f12 * f, f15 * f1, f18 * f2);
			renderNorthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i + 1, j, k, 5))
	{
		float f25 = block->getBlockBrightness(blockAccess, i + 1, j, k);
		(void)f25;
		tessellator->setBrightness(block->maxX < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i + 1, j, k));
		tessellator->setColorOpaque_F(f12, f15, f18);
		int_t k1 = block->getBlockTexture(blockAccess, i, j, k, 5);
		if (overrideBlockTexture < 0 && Config::isBetterGrass())
		{
			if (k1 == 3 || k1 == 77)
			{
				k1 = Config::getSideGrassTexture(blockAccess, i, j, k, 5, k1);
				if (k1 == 0)
					tessellator->setColorOpaque_F(f12 * f, f15 * f1, f18 * f2);
			}
			if (k1 == 68)
				k1 = Config::getSideSnowGrassTexture(blockAccess, i, j, k, 5);
		}
		renderSouthFace(block, i, j, k, k1);
		if (fancyGrass && k1 == 3 && overrideBlockTexture < 0)
		{
			// The grass side overlay (atlas tile 38) is an alpha-cut texture on a
			// block that IS an opaque cube, so the isOpaqueCube() test in
			// renderBlockByRenderType() does not catch it.
			usedAlphaTestedTexture = true;
			tessellator->setColorOpaque_F(f12 * f, f15 * f1, f18 * f2);
			renderSouthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	return flag;
}

bool RenderBlocks::renderBlockCactus(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = CustomColorizer::getColorMultiplier(block, blockAccess, i, j, k);
	float f = (float)(l >> 16 & 0xff) / 255.0f;
	float f1 = (float)(l >> 8 & 0xff) / 255.0f;
	float f2 = (float)(l & 0xff) / 255.0f;
	if (EntityRenderer::anaglyphEnabled)
	{
		float f3 = (f * 30.0f + f1 * 59.0f + f2 * 11.0f) / 100.0f;
		float f4 = (f * 30.0f + f1 * 70.0f) / 100.0f;
		float f5 = (f * 30.0f + f2 * 70.0f) / 100.0f;
		f = f3;
		f1 = f4;
		f2 = f5;
	}
	return renderCactusWithColorMultiplier(block, i, j, k, f, f1, f2);
}

bool RenderBlocks::renderCactusWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2)
{
	Tessellator *tessellator = &Tessellator::instance;
	bool flag = false;
	float f3 = 0.5f;
	float f4 = 1.0f;
	float f5 = 0.8f;
	float f6 = 0.6f;
	float f7 = f3 * f;
	float f8 = f4 * f;
	float f9 = f5 * f;
	float f10 = f6 * f;
	float f11 = f3 * f1;
	float f12 = f4 * f1;
	float f13 = f5 * f1;
	float f14 = f6 * f1;
	float f15 = f3 * f2;
	float f16 = f4 * f2;
	float f17 = f5 * f2;
	float f18 = f6 * f2;
	float f19 = 0.0625f;
	const int_t packedBrightness = block->getMixedBrightnessForBlock(blockAccess, i, j, k);
	if (renderAllFaces || shouldRenderFace(block, i, j - 1, k, 0))
	{
		tessellator->setBrightness(block->minY > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j - 1, k));
		tessellator->setColorOpaque_F(f7, f11, f15);
		renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i, j + 1, k, 1))
	{
		tessellator->setBrightness(block->maxY < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j + 1, k));
		tessellator->setColorOpaque_F(f8, f12, f16);
		renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i, j, k - 1, 2))
	{
		tessellator->setBrightness(block->minZ > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k - 1));
		tessellator->setColorOpaque_F(f9, f13, f17);
		tessellator->setTranslationF(0.0f, 0.0f, f19);
		renderEastFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 2));
		tessellator->setTranslationF(0.0f, 0.0f, -f19);
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i, j, k + 1, 3))
	{
		tessellator->setBrightness(block->maxZ < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k + 1));
		tessellator->setColorOpaque_F(f9, f13, f17);
		tessellator->setTranslationF(0.0f, 0.0f, -f19);
		renderWestFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 3));
		tessellator->setTranslationF(0.0f, 0.0f, f19);
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i - 1, j, k, 4))
	{
		tessellator->setBrightness(block->minX > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i - 1, j, k));
		tessellator->setColorOpaque_F(f10, f14, f18);
		tessellator->setTranslationF(f19, 0.0f, 0.0f);
		renderNorthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 4));
		tessellator->setTranslationF(-f19, 0.0f, 0.0f);
		flag = true;
	}
	if (renderAllFaces || shouldRenderFace(block, i + 1, j, k, 5))
	{
		tessellator->setBrightness(block->maxX < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i + 1, j, k));
		tessellator->setColorOpaque_F(f10, f14, f18);
		tessellator->setTranslationF(-f19, 0.0f, 0.0f);
		renderSouthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 5));
		tessellator->setTranslationF(f19, 0.0f, 0.0f);
		flag = true;
	}
	return flag;
}

bool RenderBlocks::renderBlockFence(Block *block, int_t i, int_t j, int_t k)
{
	BlockFence *blockFence = static_cast<BlockFence *>(block);
	bool rendered = false;
	float minRail = 6.0f / 16.0f;
	float maxRail = 10.0f / 16.0f;
	block->setBlockBounds(minRail, 0.0f, minRail, maxRail, 1.0f, maxRail);
	renderStandardBlock(block, i, j, k);
	rendered = true;

	const bool west = blockFence->canConnectFenceTo(blockAccess, i - 1, j, k);
	const bool east = blockFence->canConnectFenceTo(blockAccess, i + 1, j, k);
	const bool north = blockFence->canConnectFenceTo(blockAccess, i, j, k - 1);
	const bool south = blockFence->canConnectFenceTo(blockAccess, i, j, k + 1);
	bool connectX = west || east;
	bool connectZ = north || south;
	if (!connectX && !connectZ)
		connectX = true;

	minRail = 7.0f / 16.0f;
	maxRail = 9.0f / 16.0f;
	float railMinY = 12.0f / 16.0f;
	float railMaxY = 15.0f / 16.0f;
	const float minX = west ? 0.0f : minRail;
	const float maxX = east ? 1.0f : maxRail;
	const float minZ = north ? 0.0f : minRail;
	const float maxZ = south ? 1.0f : maxRail;
	if (connectX)
	{
		block->setBlockBounds(minX, railMinY, minRail, maxX, railMaxY, maxRail);
		renderStandardBlock(block, i, j, k);
		rendered = true;
	}
	if (connectZ)
	{
		block->setBlockBounds(minRail, railMinY, minZ, maxRail, railMaxY, maxZ);
		renderStandardBlock(block, i, j, k);
		rendered = true;
	}

	railMinY = 6.0f / 16.0f;
	railMaxY = 9.0f / 16.0f;
	if (connectX)
	{
		block->setBlockBounds(minX, railMinY, minRail, maxX, railMaxY, maxRail);
		renderStandardBlock(block, i, j, k);
		rendered = true;
	}
	if (connectZ)
	{
		block->setBlockBounds(minRail, railMinY, minZ, maxRail, railMaxY, maxZ);
		renderStandardBlock(block, i, j, k);
		rendered = true;
	}

	block->setBlockBoundsBasedOnState(blockAccess, i, j, k);
	renderBetterSnow(i, j, k);
	return rendered;
}

bool RenderBlocks::renderBlockStairs(Block *block, int_t i, int_t j, int_t k)
{
	const int_t metadata = accessGetBlockMetadata(i, j, k);
	const int_t direction = metadata & 3;
	float slabMinY = 0.0f;
	float slabMaxY = 0.5f;
	float stepMinY = 0.5f;
	float stepMaxY = 1.0f;
	if ((metadata & 4) != 0)
	{
		slabMinY = 0.5f;
		slabMaxY = 1.0f;
		stepMinY = 0.0f;
		stepMaxY = 0.5f;
	}

	block->setBlockBounds(0.0f, slabMinY, 0.0f, 1.0f, slabMaxY, 1.0f);
	renderStandardBlock(block, i, j, k);
	if (direction == 0)
	{
		block->setBlockBounds(0.5f, stepMinY, 0.0f, 1.0f, stepMaxY, 1.0f);
		renderStandardBlock(block, i, j, k);
	}
	else if (direction == 1)
	{
		block->setBlockBounds(0.0f, stepMinY, 0.0f, 0.5f, stepMaxY, 1.0f);
		renderStandardBlock(block, i, j, k);
	}
	else if (direction == 2)
	{
		block->setBlockBounds(0.0f, stepMinY, 0.5f, 1.0f, stepMaxY, 1.0f);
		renderStandardBlock(block, i, j, k);
	}
	else if (direction == 3)
	{
		block->setBlockBounds(0.0f, stepMinY, 0.0f, 1.0f, stepMaxY, 0.5f);
		renderStandardBlock(block, i, j, k);
	}

	block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool RenderBlocks::renderBlockDoor(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	BlockDoor *blockdoor = (BlockDoor *)block;
	bool flag = false;
	float f = 0.5f;
	float f1 = 1.0f;
	float f2 = 0.8f;
	float f3 = 0.6f;
	float f4 = block->getBlockBrightness(blockAccess, i, j, k);
	(void)f4;
	float f5 = block->getBlockBrightness(blockAccess, i, j - 1, k);
	const int_t packedBrightness = block->getMixedBrightnessForBlock(blockAccess, i, j, k);
	(void)f5;
	tessellator->setBrightness(block->minY > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j - 1, k));
	tessellator->setColorOpaque_F(f, f, f);
	renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
	flag = true;
	f5 = block->getBlockBrightness(blockAccess, i, j + 1, k);
	(void)f5;
	tessellator->setBrightness(block->maxY < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j + 1, k));
	tessellator->setColorOpaque_F(f1, f1, f1);
	renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
	flag = true;
	f5 = block->getBlockBrightness(blockAccess, i, j, k - 1);
	(void)f5;
	tessellator->setBrightness(block->minZ > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k - 1));
	tessellator->setColorOpaque_F(f2, f2, f2);
	int_t l = block->getBlockTexture(blockAccess, i, j, k, 2);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderEastFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	f5 = block->getBlockBrightness(blockAccess, i, j, k + 1);
	(void)f5;
	tessellator->setBrightness(block->maxZ < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i, j, k + 1));
	tessellator->setColorOpaque_F(f2, f2, f2);
	l = block->getBlockTexture(blockAccess, i, j, k, 3);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderWestFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	f5 = block->getBlockBrightness(blockAccess, i - 1, j, k);
	(void)f5;
	tessellator->setBrightness(block->minX > 0.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i - 1, j, k));
	tessellator->setColorOpaque_F(f3, f3, f3);
	l = block->getBlockTexture(blockAccess, i, j, k, 4);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderNorthFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	f5 = block->getBlockBrightness(blockAccess, i + 1, j, k);
	(void)f5;
	tessellator->setBrightness(block->maxX < 1.0 ? packedBrightness : block->getMixedBrightnessForBlock(blockAccess, i + 1, j, k));
	tessellator->setColorOpaque_F(f3, f3, f3);
	l = block->getBlockTexture(blockAccess, i, j, k, 5);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderSouthFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	return flag;
}

int_t &RenderBlocks::getFaceRotation(int_t side)
{
	switch (side)
	{
	case 0: return bottomFaceRotation;
	case 1: return topFaceRotation;
	case 2: return eastFaceRotation;
	case 3: return westFaceRotation;
	case 4: return northFaceRotation;
	default: return southFaceRotation;
	}
}

void RenderBlocks::restoreNaturalTextureTransform()
{
	if (!naturalTextureTransformActive)
		return;

	getFaceRotation(naturalTextureTransformSide) = naturalSavedFaceRotation;
	flipTexture = naturalSavedFlipTexture;
	naturalTextureTransformActive = false;
}

void RenderBlocks::applyNaturalTextureTransform(int_t side, int_t rotation, bool flip)
{
	// RenderBlocks also uses these members for vanilla block orientation. Save
	// that state instead of resetting it, so Natural Textures remains strictly
	// face-local and cannot break beds, logs, portals or inventory rendering.
	restoreNaturalTextureTransform();
	naturalTextureTransformActive = true;
	naturalTextureTransformSide = side;
	naturalSavedFaceRotation = getFaceRotation(side);
	naturalSavedFlipTexture = flipTexture;
	getFaceRotation(side) = rotation;
	flipTexture = flip;
}

Tessellator *RenderBlocks::resolveFaceTexture(Block *block, int_t side, int_t x, int_t y, int_t z, int_t &tile)
{
	restoreNaturalTextureTransform();
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0 || blockAccess == nullptr || block == nullptr)
		return tessellator;

	int_t textureId = 0;
	if (Config::isConnectedTextures())
	{
		const int_t connected = ConnectedTextures::getConnectedTexture(blockAccess, block, x, y, z, side, tile);
		if (connected >= 0)
		{
			textureId = connected / 256;
			tile = connected % 256;
			tessellator = Tessellator::instance.getSubTessellator(textureId);
		}
	}

	if (Config::isNaturalTextures())
	{
		const NaturalProperties *natural = NaturalTextures::getNaturalProperties(textureId, tile);
		if (natural != nullptr)
		{
			const int_t random = Config::getRandom(x, y, z, side);
			int_t rotation = 0;
			if (natural->rotation > 1)
				rotation = random & 3;
			if (natural->rotation == 2)
				rotation = (rotation / 2) * 3;
			const bool naturalFlip = natural->flip && (random & 4) != 0;
			applyNaturalTextureTransform(side, rotation, naturalFlip);
		}
	}
	return tessellator;
}

void RenderBlocks::renderBottomFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	else
	{
		tessellator = resolveFaceTexture(block, 0, (int_t)d, (int_t)d1, (int_t)d2, i);
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	tess_coord_t d3 = ((tess_coord_t)j + (tess_coord_t)block->minX * 16.0f) / 256.0f;
	tess_coord_t d4 = (((tess_coord_t)j + (tess_coord_t)block->maxX * 16.0f) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d5 = ((tess_coord_t)k + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
	tess_coord_t d6 = (((tess_coord_t)k + (tess_coord_t)block->maxZ * 16.0f) - kAtlasUvGuard) / 256.0f;
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	if (flipTexture)
	{
		tess_coord_t temp = d3; d3 = d4; d4 = temp;
	}
	tess_coord_t d7 = d4;
	tess_coord_t d8 = d3;
	tess_coord_t d9 = d5;
	tess_coord_t d10 = d6;
	if (bottomFaceRotation == 2)
	{
		d3 = ((tess_coord_t)j + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)j + (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d7 = d3;
		d8 = d4;
		d5 = d6;
		d6 = d9;
	}
	else if (bottomFaceRotation == 1)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)k + (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d3 = d7;
		d4 = d8;
		d9 = d6;
		d10 = d5;
	}
	else if (bottomFaceRotation == 3)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxX * 16.0f - kAtlasUvGuard) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxZ * 16.0f - kAtlasUvGuard) / 256.0f;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
	}
	tess_coord_t d11 = (tess_coord_t)d + (tess_coord_t)block->minX;
	tess_coord_t d12 = (tess_coord_t)d + (tess_coord_t)block->maxX;
	tess_coord_t d13 = (tess_coord_t)d1 + (tess_coord_t)block->minY;
	tess_coord_t d14 = (tess_coord_t)d2 + (tess_coord_t)block->minZ;
	tess_coord_t d15 = (tess_coord_t)d2 + (tess_coord_t)block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->setBrightness(brightnessTopLeft);
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->setBrightness(brightnessBottomLeft);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->setBrightness(brightnessBottomRight);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->setBrightness(brightnessTopRight);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
	}
	else
	{
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
	}
	restoreNaturalTextureTransform();
}

void RenderBlocks::renderTopFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	else
	{
		tessellator = resolveFaceTexture(block, 1, (int_t)d, (int_t)d1, (int_t)d2, i);
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	tess_coord_t d3 = ((tess_coord_t)j + (tess_coord_t)block->minX * 16.0f) / 256.0f;
	tess_coord_t d4 = (((tess_coord_t)j + (tess_coord_t)block->maxX * 16.0f) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d5 = ((tess_coord_t)k + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
	tess_coord_t d6 = (((tess_coord_t)k + (tess_coord_t)block->maxZ * 16.0f) - kAtlasUvGuard) / 256.0f;
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	if (flipTexture)
	{
		tess_coord_t temp = d3; d3 = d4; d4 = temp;
	}
	tess_coord_t d7 = d4;
	tess_coord_t d8 = d3;
	tess_coord_t d9 = d5;
	tess_coord_t d10 = d6;
	if (topFaceRotation == 1)
	{
		d3 = ((tess_coord_t)j + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)j + (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d7 = d3;
		d8 = d4;
		d5 = d6;
		d6 = d9;
	}
	else if (topFaceRotation == 2)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)k + (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d3 = d7;
		d4 = d8;
		d9 = d6;
		d10 = d5;
	}
	else if (topFaceRotation == 3)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxX * 16.0f - kAtlasUvGuard) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxZ * 16.0f - kAtlasUvGuard) / 256.0f;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
	}
	tess_coord_t d11 = (tess_coord_t)d + (tess_coord_t)block->minX;
	tess_coord_t d12 = (tess_coord_t)d + (tess_coord_t)block->maxX;
	tess_coord_t d13 = (tess_coord_t)d1 + (tess_coord_t)block->maxY;
	tess_coord_t d14 = (tess_coord_t)d2 + (tess_coord_t)block->minZ;
	tess_coord_t d15 = (tess_coord_t)d2 + (tess_coord_t)block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->setBrightness(brightnessTopLeft);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->setBrightness(brightnessBottomLeft);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->setBrightness(brightnessBottomRight);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->setBrightness(brightnessTopRight);
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
	}
	restoreNaturalTextureTransform();
}

void RenderBlocks::renderEastFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	else
	{
		tessellator = resolveFaceTexture(block, 2, (int_t)d, (int_t)d1, (int_t)d2, i);
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	tess_coord_t d3 = ((tess_coord_t)j + (tess_coord_t)block->minX * 16.0f) / 256.0f;
	tess_coord_t d4 = (((tess_coord_t)j + (tess_coord_t)block->maxX * 16.0f) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
	tess_coord_t d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minY * 16.0f - kAtlasUvGuard) / 256.0f;
	if (flipTexture)
	{
		tess_coord_t d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	tess_coord_t d8 = d4;
	tess_coord_t d9 = d3;
	tess_coord_t d10 = d5;
	tess_coord_t d11 = d6;
	if (eastFaceRotation == 2)
	{
		d3 = ((tess_coord_t)j + (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)j + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (eastFaceRotation == 1)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)k + (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (eastFaceRotation == 3)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxX * 16.0f - kAtlasUvGuard) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d6 = (((tess_coord_t)k + (tess_coord_t)block->minY * 16.0f) - kAtlasUvGuard) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	tess_coord_t d12 = (tess_coord_t)d + (tess_coord_t)block->minX;
	tess_coord_t d13 = (tess_coord_t)d + (tess_coord_t)block->maxX;
	tess_coord_t d14 = (tess_coord_t)d1 + (tess_coord_t)block->minY;
	tess_coord_t d15 = (tess_coord_t)d1 + (tess_coord_t)block->maxY;
	tess_coord_t d16 = (tess_coord_t)d2 + (tess_coord_t)block->minZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->setBrightness(brightnessTopLeft);
		tessellator->addVertexWithUV(d12, d15, d16, d8, d10);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->setBrightness(brightnessBottomLeft);
		tessellator->addVertexWithUV(d13, d15, d16, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->setBrightness(brightnessBottomRight);
		tessellator->addVertexWithUV(d13, d14, d16, d9, d11);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->setBrightness(brightnessTopRight);
		tessellator->addVertexWithUV(d12, d14, d16, d4, d6);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d15, d16, d8, d10);
		tessellator->addVertexWithUV(d13, d15, d16, d3, d5);
		tessellator->addVertexWithUV(d13, d14, d16, d9, d11);
		tessellator->addVertexWithUV(d12, d14, d16, d4, d6);
	}
	restoreNaturalTextureTransform();
}

void RenderBlocks::renderWestFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	else
	{
		tessellator = resolveFaceTexture(block, 3, (int_t)d, (int_t)d1, (int_t)d2, i);
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	tess_coord_t d3 = ((tess_coord_t)j + (tess_coord_t)block->minX * 16.0f) / 256.0f;
	tess_coord_t d4 = (((tess_coord_t)j + (tess_coord_t)block->maxX * 16.0f) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
	tess_coord_t d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minY * 16.0f - kAtlasUvGuard) / 256.0f;
	if (flipTexture)
	{
		tess_coord_t d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	tess_coord_t d8 = d4;
	tess_coord_t d9 = d3;
	tess_coord_t d10 = d5;
	tess_coord_t d11 = d6;
	if (westFaceRotation == 1)
	{
		d3 = ((tess_coord_t)j + (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)j + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (westFaceRotation == 2)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)k + (tess_coord_t)block->maxX * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (westFaceRotation == 3)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minX * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxX * 16.0f - kAtlasUvGuard) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d6 = (((tess_coord_t)k + (tess_coord_t)block->minY * 16.0f) - kAtlasUvGuard) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	tess_coord_t d12 = (tess_coord_t)d + (tess_coord_t)block->minX;
	tess_coord_t d13 = (tess_coord_t)d + (tess_coord_t)block->maxX;
	tess_coord_t d14 = (tess_coord_t)d1 + (tess_coord_t)block->minY;
	tess_coord_t d15 = (tess_coord_t)d1 + (tess_coord_t)block->maxY;
	tess_coord_t d16 = (tess_coord_t)d2 + (tess_coord_t)block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->setBrightness(brightnessTopLeft);
		tessellator->addVertexWithUV(d12, d15, d16, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->setBrightness(brightnessBottomLeft);
		tessellator->addVertexWithUV(d12, d14, d16, d9, d11);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->setBrightness(brightnessBottomRight);
		tessellator->addVertexWithUV(d13, d14, d16, d4, d6);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->setBrightness(brightnessTopRight);
		tessellator->addVertexWithUV(d13, d15, d16, d8, d10);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d15, d16, d3, d5);
		tessellator->addVertexWithUV(d12, d14, d16, d9, d11);
		tessellator->addVertexWithUV(d13, d14, d16, d4, d6);
		tessellator->addVertexWithUV(d13, d15, d16, d8, d10);
	}
	restoreNaturalTextureTransform();
}

void RenderBlocks::renderNorthFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	else
	{
		tessellator = resolveFaceTexture(block, 4, (int_t)d, (int_t)d1, (int_t)d2, i);
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	tess_coord_t d3 = ((tess_coord_t)j + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
	tess_coord_t d4 = (((tess_coord_t)j + (tess_coord_t)block->maxZ * 16.0f) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
	tess_coord_t d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minY * 16.0f - kAtlasUvGuard) / 256.0f;
	if (flipTexture)
	{
		tess_coord_t d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	tess_coord_t d8 = d4;
	tess_coord_t d9 = d3;
	tess_coord_t d10 = d5;
	tess_coord_t d11 = d6;
	if (northFaceRotation == 1)
	{
		d3 = ((tess_coord_t)j + (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)j + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (northFaceRotation == 2)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)k + (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (northFaceRotation == 3)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxZ * 16.0f - kAtlasUvGuard) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d6 = (((tess_coord_t)k + (tess_coord_t)block->minY * 16.0f) - kAtlasUvGuard) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	tess_coord_t d12 = (tess_coord_t)d + (tess_coord_t)block->minX;
	tess_coord_t d13 = (tess_coord_t)d1 + (tess_coord_t)block->minY;
	tess_coord_t d14 = (tess_coord_t)d1 + (tess_coord_t)block->maxY;
	tess_coord_t d15 = (tess_coord_t)d2 + (tess_coord_t)block->minZ;
	tess_coord_t d16 = (tess_coord_t)d2 + (tess_coord_t)block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->setBrightness(brightnessTopLeft);
		tessellator->addVertexWithUV(d12, d14, d16, d8, d10);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->setBrightness(brightnessBottomLeft);
		tessellator->addVertexWithUV(d12, d14, d15, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->setBrightness(brightnessBottomRight);
		tessellator->addVertexWithUV(d12, d13, d15, d9, d11);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->setBrightness(brightnessTopRight);
		tessellator->addVertexWithUV(d12, d13, d16, d4, d6);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d14, d16, d8, d10);
		tessellator->addVertexWithUV(d12, d14, d15, d3, d5);
		tessellator->addVertexWithUV(d12, d13, d15, d9, d11);
		tessellator->addVertexWithUV(d12, d13, d16, d4, d6);
	}
	restoreNaturalTextureTransform();
}

void RenderBlocks::renderSouthFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	else
	{
		tessellator = resolveFaceTexture(block, 5, (int_t)d, (int_t)d1, (int_t)d2, i);
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	tess_coord_t d3 = ((tess_coord_t)j + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
	tess_coord_t d4 = (((tess_coord_t)j + (tess_coord_t)block->maxZ * 16.0f) - kAtlasUvGuard) / 256.0f;
	tess_coord_t d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
	tess_coord_t d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minY * 16.0f - kAtlasUvGuard) / 256.0f;
	if (flipTexture)
	{
		tess_coord_t d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	tess_coord_t d8 = d4;
	tess_coord_t d9 = d3;
	tess_coord_t d10 = d5;
	tess_coord_t d11 = d6;
	if (southFaceRotation == 2)
	{
		d3 = ((tess_coord_t)j + (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)j + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)(k + 16) - (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (southFaceRotation == 1)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->maxZ * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minY * 16.0f) / 256.0f;
		d6 = ((tess_coord_t)k + (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (southFaceRotation == 3)
	{
		d3 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->minZ * 16.0f) / 256.0f;
		d4 = ((tess_coord_t)(j + 16) - (tess_coord_t)block->maxZ * 16.0f - kAtlasUvGuard) / 256.0f;
		d5 = ((tess_coord_t)k + (tess_coord_t)block->maxY * 16.0f) / 256.0f;
		d6 = (((tess_coord_t)k + (tess_coord_t)block->minY * 16.0f) - kAtlasUvGuard) / 256.0f;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	tess_coord_t d12 = (tess_coord_t)d + (tess_coord_t)block->maxX;
	tess_coord_t d13 = (tess_coord_t)d1 + (tess_coord_t)block->minY;
	tess_coord_t d14 = (tess_coord_t)d1 + (tess_coord_t)block->maxY;
	tess_coord_t d15 = (tess_coord_t)d2 + (tess_coord_t)block->minZ;
	tess_coord_t d16 = (tess_coord_t)d2 + (tess_coord_t)block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->setBrightness(brightnessTopLeft);
		tessellator->addVertexWithUV(d12, d13, d16, d9, d11);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->setBrightness(brightnessBottomLeft);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->setBrightness(brightnessBottomRight);
		tessellator->addVertexWithUV(d12, d14, d15, d8, d10);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->setBrightness(brightnessTopRight);
		tessellator->addVertexWithUV(d12, d14, d16, d3, d5);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d13, d16, d9, d11);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->addVertexWithUV(d12, d14, d15, d8, d10);
		tessellator->addVertexWithUV(d12, d14, d16, d3, d5);
	}
	restoreNaturalTextureTransform();
}

void RenderBlocks::renderBlockOnInventory(Block *block, int_t i, float f)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (field_31088_b)
	{
		int_t j = block->getRenderColor(i);
		if (block->blockID == Block::grass->blockID)
			j = 0xffffff;
		float f1 = (float)(j >> 16 & 0xff) / 255.0f;
		float f3 = (float)(j >> 8 & 0xff) / 255.0f;
		float f5 = (float)(j & 0xff) / 255.0f;
		renderColor4f(f1 * f, f3 * f, f5 * f, 1.0f);
	}
	int_t k = block->getRenderType();
	if (k == 0 || k == 16)
	{
		if (k == 16)
		{
			i = 1;
		}
		block->setBlockBoundsForItemRender();
		renderTranslate(-0.5f, -0.5f, -0.5f);
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(0, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 1.0f, 0.0f);
		renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(1, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, -1.0f);
		renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(2, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, 1.0f);
		renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(3, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(-1.0f, 0.0f, 0.0f);
		renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(4, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(1.0f, 0.0f, 0.0f);
		renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(5, i));
		tessellator->draw();
		renderTranslate(0.5f, 0.5f, 0.5f);
	}
	else if (k == 1)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderCrossedSquares(block, i, -0.5, -0.5, -0.5);
		tessellator->draw();
	}
	else if (k == 19)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		block->setBlockBoundsForItemRender();
		renderBlockStemSmall(block, i, block->maxY, -0.5, -0.5, -0.5);
		tessellator->draw();
	}
	else if (k == 23)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		block->setBlockBoundsForItemRender();
		tessellator->draw();
	}
	else if (k == 13)
	{
		block->setBlockBoundsForItemRender();
		renderTranslate(-0.5f, -0.5f, -0.5f);
		float f2 = 0.0625f;
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 1.0f, 0.0f);
		renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, -1.0f);
		tessellator->setTranslationF(0.0f, 0.0f, f2);
		renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
		tessellator->setTranslationF(0.0f, 0.0f, -f2);
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, 1.0f);
		tessellator->setTranslationF(0.0f, 0.0f, -f2);
		renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
		tessellator->setTranslationF(0.0f, 0.0f, f2);
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(-1.0f, 0.0f, 0.0f);
		tessellator->setTranslationF(f2, 0.0f, 0.0f);
		renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
		tessellator->setTranslationF(-f2, 0.0f, 0.0f);
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(1.0f, 0.0f, 0.0f);
		tessellator->setTranslationF(-f2, 0.0f, 0.0f);
		renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
		tessellator->setTranslationF(f2, 0.0f, 0.0f);
		tessellator->draw();
		renderTranslate(0.5f, 0.5f, 0.5f);
	}
	else if (k == 6)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderCropsCrossed(block, i, -0.5, -0.5, -0.5);
		tessellator->draw();
	}
	else if (k == 2)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderTorchAtAngle(block, -0.5, -0.5, -0.5, 0.0, 0.0);
		tessellator->draw();
	}
	else if (k == 10)
	{
		for (int_t l = 0; l < 2; l++)
		{
			if (l == 0)
			{
				block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
			}
			if (l == 1)
			{
				block->setBlockBounds(0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f);
			}
			renderTranslate(-0.5f, -0.5f, -0.5f);
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, -1.0f, 0.0f);
			renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 1.0f, 0.0f);
			renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, -1.0f);
			renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, 1.0f);
			renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(-1.0f, 0.0f, 0.0f);
			renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(1.0f, 0.0f, 0.0f);
			renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
			tessellator->draw();
			renderTranslate(0.5f, 0.5f, 0.5f);
		}
	}
	else if (k == 27)
	{
		int_t accumulatedHeight = 0;
		renderTranslate(-0.5f, -0.5f, -0.5f);
		tessellator->startDrawingQuads();
		for (int_t layer = 0; layer < 8; ++layer)
		{
			int_t radiusUnits = 0;
			int_t heightUnits = 1;
			switch (layer)
			{
			case 0: radiusUnits = 2; break;
			case 1: radiusUnits = 3; break;
			case 2: radiusUnits = 4; break;
			case 3: radiusUnits = 5; heightUnits = 2; break;
			case 4: radiusUnits = 6; heightUnits = 3; break;
			case 5: radiusUnits = 7; heightUnits = 5; break;
			case 6: radiusUnits = 6; heightUnits = 2; break;
			case 7: radiusUnits = 3; break;
			}
			const float radius = static_cast<float>(radiusUnits) / 16.0f;
			const float top = 1.0f - static_cast<float>(accumulatedHeight) / 16.0f;
			const float bottom = 1.0f - static_cast<float>(accumulatedHeight + heightUnits) / 16.0f;
			accumulatedHeight += heightUnits;
			block->setBlockBounds(0.5f - radius, bottom, 0.5f - radius,
				0.5f + radius, top, 0.5f + radius);
			tessellator->setNormal(0.0f, -1.0f, 0.0f);
			renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
			tessellator->setNormal(0.0f, 1.0f, 0.0f);
			renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
			tessellator->setNormal(0.0f, 0.0f, -1.0f);
			renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
			tessellator->setNormal(0.0f, 0.0f, 1.0f);
			renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
			tessellator->setNormal(-1.0f, 0.0f, 0.0f);
			renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
			tessellator->setNormal(1.0f, 0.0f, 0.0f);
			renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
		}
		tessellator->draw();
		renderTranslate(0.5f, 0.5f, 0.5f);
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (k == 11)
	{
		for (int_t i1 = 0; i1 < 4; i1++)
		{
			float f4 = 0.125f;
			if (i1 == 0)
			{
				block->setBlockBounds(0.5f - f4, 0.0f, 0.0f, 0.5f + f4, 1.0f, f4 * 2.0f);
			}
			if (i1 == 1)
			{
				block->setBlockBounds(0.5f - f4, 0.0f, 1.0f - f4 * 2.0f, 0.5f + f4, 1.0f, 1.0f);
			}
			f4 = 0.0625f;
			if (i1 == 2)
			{
				block->setBlockBounds(0.5f - f4, 1.0f - f4 * 3.0f, -f4 * 2.0f, 0.5f + f4, 1.0f - f4, 1.0f + f4 * 2.0f);
			}
			if (i1 == 3)
			{
				block->setBlockBounds(0.5f - f4, 0.5f - f4 * 3.0f, -f4 * 2.0f, 0.5f + f4, 0.5f - f4, 1.0f + f4 * 2.0f);
			}
			renderTranslate(-0.5f, -0.5f, -0.5f);
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, -1.0f, 0.0f);
			renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 1.0f, 0.0f);
			renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, -1.0f);
			renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, 1.0f);
			renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(-1.0f, 0.0f, 0.0f);
			renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(1.0f, 0.0f, 0.0f);
			renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
			tessellator->draw();
			renderTranslate(0.5f, 0.5f, 0.5f);
		}
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (k == 21)
	{
		for (int_t part = 0; part < 3; ++part)
		{
			float inset = 1.0f / 16.0f;
			if (part == 0)
				block->setBlockBounds(0.5f - inset, 0.3f, 0.0f, 0.5f + inset, 1.0f, inset * 2.0f);
			if (part == 1)
				block->setBlockBounds(0.5f - inset, 0.3f, 1.0f - inset * 2.0f, 0.5f + inset, 1.0f, 1.0f);
			if (part == 2)
				block->setBlockBounds(0.5f - inset, 0.5f, 0.0f, 0.5f + inset, 1.0f - inset, 1.0f);

			renderTranslate(-0.5f, -0.5f, -0.5f);
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, -1.0f, 0.0f);
			renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 1.0f, 0.0f);
			renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, -1.0f);
			renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, 1.0f);
			renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(-1.0f, 0.0f, 0.0f);
			renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(1.0f, 0.0f, 0.0f);
			renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
			tessellator->draw();
			renderTranslate(0.5f, 0.5f, 0.5f);
		}
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (k == 22)
	{
		ChestItemRenderHelper::instance.renderChest(block, i, f);
		renderEnable(RenderCapability::RescaleNormal);
	}
}

bool RenderBlocks::renderItemIn3d(int_t i)
{
	if (i == 0)
	{
		return true;
	}
	if (i == 13)
	{
		return true;
	}
	if (i == 10)
	{
		return true;
	}
	if (i == 11 || i == 22 || i == 21 || i == 27)
	{
		return true;
	}
	return i == 16;
}
