#pragma once

#include "java/Type.h"
#include "Tessellator.h"

class IBlockAccess;
class ChunkCache;
class Block;
class BlockRail;
class World;
class Material;
class Vec3D;

// net.minecraft.src.RenderBlocks
class RenderBlocks
{
public:
	RenderBlocks(IBlockAccess *iblockaccess);
	RenderBlocks();

	void renderBlockUsingTexture(Block *block, int_t i, int_t j, int_t k, int_t l);
	void clearOverrideBlockTexture() { overrideBlockTexture = -1; }
	void renderBlockAllFaces(Block *block, int_t i, int_t j, int_t k); // func_31075_a
	bool renderBlockByRenderType(Block *block, int_t i, int_t j, int_t k);

	// Set as soon as a block rendered through this instance can have put texels
	// the alpha test rejects into the mesh. Sticky: the caller clears it, this
	// class only ever raises it.
	//
	// It exists for the Wii terrain path, which uses it to decide whether a
	// section's display list may be drawn with GX's early depth compare (GX can
	// only reject before the TEV stages when the alpha test is off -- see
	// GX_SetZCompLoc in WiiNativeDraw.cpp). It is deliberately conservative:
	// every non-opaque-cube block raises it whether or not its texture actually
	// has holes, because being wrong the other way would draw transparent texels
	// as solid.
	bool usedAlphaTestedTexture;

	// Renombrado de func_31078_d: renderiza base de pistón con todas las caras
	void renderPistonBaseAllFaces(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_31079_a: renderiza extensión de pistón con todas las caras
	void renderPistonExtensionAllFaces(Block *block, int_t i, int_t j, int_t k, bool flag);

private:
	// Devirtualised block reads.
	//
	// Every one of these used to be blockAccess->something(), an indirect call
	// through the IBlockAccess vtable. That costs more on the consoles than the
	// call itself: neither the R5900 nor the Broadway predicts an indirect jump,
	// so each one drains the pipeline, and a section build issues roughly 25k of
	// them because RenderBlocks probes six neighbours per block (see the note on
	// getBlockId in ChunkCache.h).
	//
	// On the meshing path the object behind the interface is always a ChunkCache,
	// so blockAccessCache below holds it pre-resolved and these forward with the
	// virtual dispatch suppressed. When it is null -- the inventory and held-item
	// renderers pass a World, or nothing at all -- the original virtual call is
	// still what runs, so behaviour is identical either way.
	//
	// Note this only removes the OUTER dispatch. ChunkCache::getBlockId then
	// calls Chunk::getBlockID, which is itself virtual because EmptyChunk
	// overrides it; PLATFORM_FAST_CHUNK_BLOCK_READS removes that inner one too,
	// by resolving each source chunk's raw arrays once per ChunkCache instead of
	// dispatching per access.
	int_t accessGetBlockId(int_t i, int_t j, int_t k);
	int_t accessGetBlockMetadata(int_t i, int_t j, int_t k);
	Material *accessGetBlockMaterial(int_t i, int_t j, int_t k);
	bool accessIsBlockNormalCube(int_t i, int_t j, int_t k);
	bool accessIsBlockOpaqueCube(int_t i, int_t j, int_t k);
	bool accessIsAirBlock(int_t i, int_t j, int_t k);

	// Devirtualised face culling. block->shouldSideBeRendered(blockAccess, ...)
	// is itself a second indirect call on top of the ones above, and for the
	// ~90% of blocks that never override it (see Block::usesDefaultFaceCulling),
	// its body reduces to the neighbour-opacity check already devirtualised
	// above. Skip straight to that for those blocks; anything that overrides
	// the rule (fluids, ice, leaves, stairs/slabs, snow, portal, breakable,
	// redstone repeater) still goes through the real virtual call unchanged.
	bool shouldRenderFace(Block *block, int_t i, int_t j, int_t k, int_t side);
	int_t fixAoSideGrassTexture(int_t texture, int_t x, int_t y, int_t z, int_t side, float red, float green, float blue);
	bool hasSnowNeighbours(int_t x, int_t y, int_t z);
	void renderBetterSnow(int_t x, int_t y, int_t z, double maxY = -1.0);

	bool renderBlockTorch(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockBed(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockRepeater(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_31074_b: renderiza base de pistón (no extendido)
	bool renderPistonBase(Block *block, int_t i, int_t j, int_t k, bool flag);
	// Renombrado de func_31076_a: renderiza brazo de pistón en X
	void renderPistonArmX(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6);
	// Renombrado de func_31081_b: renderiza brazo de pistón en Y
	void renderPistonArmY(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6);
	// Renombrado de func_31077_c: renderiza brazo de pistón en Z
	void renderPistonArmZ(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6);
	void renderPistonRodUD(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6) { renderPistonArmX(d, d1, d2, d3, d4, d5, f, d6); }
	void renderPistonRodSN(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6) { renderPistonArmY(d, d1, d2, d3, d4, d5, f, d6); }
	void renderPistonRodEW(float d, float d1, float d2, float d3, float d4, float d5, float f, float d6) { renderPistonArmZ(d, d1, d2, d3, d4, d5, f, d6); }
	int_t getAoBrightness(int_t a, int_t b, int_t c, int_t base) const;
	// Renombrado de func_31080_c: renderiza extensión de pistón
	bool renderPistonExtension(Block *block, int_t i, int_t j, int_t k, bool flag);

public:
	bool renderBlockLever(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockFire(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockRedstoneWire(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockMinecartTrack(BlockRail *blockrail, int_t i, int_t j, int_t k);
	bool renderBlockLadder(Block *block, int_t i, int_t j, int_t k);
	bool renderCrossedSquares(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockReed(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockCrops(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockStem(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockPane(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockVine(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockFenceGate(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockLilyPad(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockCauldron(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockBrewingStand(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockEndPortalFrame(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockDragonEgg(Block *block, int_t i, int_t j, int_t k);
	void renderBlockStemSmall(Block *block, int_t metadata, double height, double x, double y, double z);
	void renderBlockStemBig(Block *block, int_t metadata, int_t direction, double height, double x, double y, double z);
	void renderTorchAtAngle(Block *block, double d, double d1, double d2, double d3, double d4);
	void renderCrossedSquares(Block *block, int_t i, double d, double d1, double d2);
	void drawCrossedSquares(Block *block, int_t i, double d, double d1, double d2) { renderCrossedSquares(block, i, d, d1, d2); }
	// Renombrado de func_1245_b: renderiza cultivos cruzados (crops)
	void renderCropsCrossed(Block *block, int_t i, double d, double d1, double d2);
	void renderBlockCropsImpl(Block *block, int_t i, double d, double d1, double d2) { renderCropsCrossed(block, i, d, d1, d2); }
	bool renderBlockFluids(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_1224_a: calcula altura de fluido
	float getFluidHeight(int_t i, int_t j, int_t k, Material *material);
	void renderBlockFallingSand(Block *block, World *world, int_t i, int_t j, int_t k);
	bool renderStandardBlock(Block *block, int_t i, int_t j, int_t k);
	bool renderStandardBlockWithAmbientOcclusion(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2);
	bool renderStandardBlockWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2);
	bool renderBlockCactus(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_1230_b: renderiza cactus con multiplicador de color
	bool renderCactusWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2);
	bool renderBlockCactusImpl(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2) { return renderCactusWithColorMultiplier(block, i, j, k, f, f1, f2); }
	bool renderBlockFence(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockStairs(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockDoor(Block *block, int_t i, int_t j, int_t k);
	void renderBottomFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i);
	void renderTopFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i);
	void renderEastFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i);
	void renderWestFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i);
	void renderNorthFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i);
	void renderSouthFace(Block *block, tess_coord_t d, tess_coord_t d1, tess_coord_t d2, int_t i);
	Tessellator *resolveFaceTexture(Block *block, int_t side, int_t x, int_t y, int_t z, int_t &tile);
	void applyNaturalTextureTransform(int_t side, int_t rotation, bool flip);
	void restoreNaturalTextureTransform();
	int_t &getFaceRotation(int_t side);
	void renderBlockOnInventory(Block *block, int_t i, float f);
	void renderBlockAsItem(Block *block, int_t i, float f) { renderBlockOnInventory(block, i, f); }

	static bool renderItemIn3d(int_t i);

	IBlockAccess *blockAccess;
	// blockAccess resolved to its concrete type once at construction, or null
	// when it is not a ChunkCache. See the accessors above.
	ChunkCache *blockAccessCache;
	int_t overrideBlockTexture;
	bool flipTexture;
	bool renderAllFaces;
	bool field_31088_b;
	// Renombrado de field_31087_g: rotación de textura para cara este
	int_t eastFaceRotation;
	// Renombrado de field_31086_h: rotación de textura para cara oeste
	int_t westFaceRotation;
	// Renombrado de field_31085_i: rotación de textura para cara sur
	int_t southFaceRotation;
	// Renombrado de field_31084_j: rotación de textura para cara norte
	int_t northFaceRotation;
	// Renombrado de field_31083_k: rotación de textura para cara superior
	int_t topFaceRotation;
	// Renombrado de field_31082_l: rotación de textura para cara inferior
	int_t bottomFaceRotation;
	bool naturalTextureTransformActive;
	bool naturalSavedFlipTexture;
	int_t naturalSavedFaceRotation;
	int_t naturalTextureTransformSide;
	// Renombrado de field_22352_G: flag de calidad de iluminación (1 = mejor calidad)
	int_t lightingQuality;
	bool enableAO;
	int_t brightnessTopLeft;
	int_t brightnessBottomLeft;
	int_t brightnessBottomRight;
	int_t brightnessTopRight;
	float colorRedTopLeft;
	float colorRedBottomLeft;
	float colorRedBottomRight;
	float colorRedTopRight;
	float colorGreenTopLeft;
	float colorGreenBottomLeft;
	float colorGreenBottomRight;
	float colorGreenTopRight;
	float colorBlueTopLeft;
	float colorBlueBottomLeft;
	float colorBlueBottomRight;
	float colorBlueTopRight;

	static bool fancyGrass;
};