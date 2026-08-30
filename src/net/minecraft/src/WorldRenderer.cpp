#include "WorldRenderer.h"
#include "java/Arithmetic.h"

#include "platform/RenderAPI.h"
#include "platform/RenderTerrainAPI.h"
#include "World.h"
#include "Config.h"
#include "ConnectedTextures.h"
#include "ICamera.h"
#include "Entity.h"
#include "MathHelper.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "RenderItem.h"
#include "Tessellator.h"
#include "Chunk.h"
#include "ChunkCache.h"
#include "TileEntity.h"
#include "TileEntityRenderer.h"
#include "AxisAlignedBB.h"

#include <algorithm>
#include <cstdint>
#include <utility>

// Not PS2-only: PLATFORM_RENDERER_AABB_MARGIN is used unconditionally further
// down, and this header is the platform-neutral place it comes from (it only
// pulls in ps2/render/Ps2Tuning.h when PLATFORM_PS2 is set). It reached the PC build
// transitively via ChunkProvider.h, which is luck rather than design.
#include "platform/PlatformTuning.h"
#include "platform/PlatformCompat.h"
#ifdef PS2_PLATFORM
#include "platform/RenderTerrainStaging.h"
#endif



int_t WorldRenderer::chunksUpdated = 0;


namespace
{
	static float rendererAabbMargin()
	{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
		// Consoles use a tuned conservative margin because their terrain clip paths
		// and fixed renderer grids differ from desktop Advanced OpenGL.
		return PLATFORM_RENDERER_AABB_MARGIN;
#else
		// OptiFine C6 uses the exact 16x16x16 section box for both frustum and
		// occlusion tests. A zero margin also makes Fancy Occlusion's "fully in
		// frustum" classification useful instead of inflating every section.
		return 0.0f;
#endif
	}
}

void WorldRenderer::eraseAllTileEntityRefs(std::vector<TileEntity *> *list, TileEntity *te)
{
	if (list == nullptr || te == nullptr)
		return;
	list->erase(std::remove(list->begin(), list->end(), te), list->end());
}

void WorldRenderer::pushUniqueTileEntityRef(std::vector<TileEntity *> *list, TileEntity *te)
{
	if (list == nullptr || te == nullptr)
		return;
	if (std::find(list->begin(), list->end(), te) == list->end())
		list->push_back(te);
}

WorldRenderer::WorldRenderer(World *world, std::vector<TileEntity *> *tileEntitiesIn, int_t posX, int_t posY, int_t posZ, int_t size, int_t glListId)
{
	worldObj      = world;
	tileEntities  = tileEntitiesIn;
	sizeWidth = sizeHeight = sizeDepth = size;
#if PLATFORM_PC
	glRenderList = glListId;
#else
	(void)glListId;
#endif
#ifdef WII_PLATFORM
	renderTerrainChunkHandlesCreate(terrainChunkHandles);
#endif
#ifdef WII_PLATFORM
	for (int_t p = 0; p < 2; ++p)
	{
		wiiBuildVertexCount[p] = 0;
		wiiBuildHasTexture[p] = false;
		wiiBuildHasColor[p] = false;
		wiiBuildDrew[p] = false;
		wiiPassNeedsAlphaTest[p] = true;
		wiiBuildNeedsAlphaTest[p] = false;
	}
	wiiBuildActive = false;
	wiiBuildSourceAvailability = 0u;
	wiiBuildSourceAvailabilityValid = false;
	wiiBuildPass = 0;
	wiiBuildCursor = 0;
	wiiBuildHasPass1 = false;
	wiiBuildChunkLit = false;
	wiiStepDidWork = false;
#endif
	needsUpdate    = false;
	isChunkLit     = false;
#if PLATFORM_PC
	isWaitingOnOcclusionQuery = false;
#endif
	isVisible      = true;
	isInFrustum    = false;
#if PLATFORM_PC || PLATFORM_PS2
	isFullyInFrustum = false;
#endif
#if PLATFORM_PC
	isVisibleFromPosition = false;
	visibleFromX = 0.0;
	visibleFromY = 0.0;
	visibleFromZ = 0.0;
	needsOcclusionBoxUpdate = false;
	glOcclusionQuery = 0;
#endif
	chunkIndex     = 0;
	isInitialized  = false;
	_skipRenderPass[0] = false;
	_skipRenderPass[1] = false;
	rendererBoundingBox = nullptr;

	this->posX = -999;
	setPosition(posX, posY, posZ);
	needsUpdate = false;
	queuedForUpdate = false;

#ifdef PS2_PLATFORM
	for (int_t p = 0; p < 2; ++p)
	{
		ps2VertexCount[p] = 0;
		ps2DrawMode[p] = 7;
		ps2HasTexture[p] = false;
		ps2HasColor[p] = false;
		ps2HasNormals[p] = false;
		ps2BuildVertexCount[p] = 0;
		ps2BuildDrawMode[p] = 7;
		ps2BuildHasTexture[p] = false;
		ps2BuildHasColor[p] = false;
		ps2BuildHasNormals[p] = false;
		ps2BuildDrew[p] = false;
	}
	ps2BuildActive = false;
	ps2BuildStagingSlot = RENDER_TERRAIN_STAGING_INVALID_SLOT;
	ps2BuildSourceAvailability = 0u;
	ps2BuildSourceAvailabilityValid = false;
	ps2MissingNeighbourMask = 0u;
	ps2BuildPass = 0;
	ps2BuildCursor = 0;
	ps2BuildGreedyFace = 0;
	ps2BuildGreedySlice = 0;
	ps2BuildHasPass1 = false;
	ps2StepDidWork = false;
	renderTerrainCacheReset(ps2TerrainCache);
#endif
}

WorldRenderer::~WorldRenderer()
{
	cleanup();
#ifdef PS2_PLATFORM
	renderTerrainCacheDestroy(ps2TerrainCache);
#endif
}

void WorldRenderer::removeTileEntityRenderersFromGlobalList()
{
	// RenderGlobal::tileEntities is a non-owning render list.  A WorldRenderer is
	// the owner of the references it contributed to that list.  When the renderer
	// is recycled, disabled or destroyed, those references must be removed before
	// the world/chunk code can invalidate and delete the TileEntity objects.
	for (TileEntity *te : tileEntityRenderers)
		eraseAllTileEntityRefs(tileEntities, te);
	tileEntityRenderers.clear();
#ifdef PS2_PLATFORM
	ps2BuildTileEntityRenderers.clear();
#endif
#ifdef WII_PLATFORM
	wiiBuildTileEntityRenderers.clear();
#endif
}

void WorldRenderer::cleanup()
{
#ifdef WII_PLATFORM
	// Native GX handles are renderer-owned. Any compatibility list block was
	// allocated lazily by this renderer and is released below after contents are
	// detached from the active world position.
#else
	// Display lists and occlusion queries come from ranges owned by
	// RenderGlobal. Deleting individual entries frees names that are reused by
	// new WorldRenderers and may then collide with model or sky display lists.
#endif
	setDontDraw();

	// rendererBoundingBox is created with AxisAlignedBB::getBoundingBox(), which
	// returns a heap allocation. Repositioning a renderer used to overwrite this
	// pointer and leak one AABB every time chunks were recycled around the player.
	delete rendererBoundingBox;
	rendererBoundingBox = nullptr;

	worldObj = nullptr;
	queuedForUpdate = false;

#ifdef PS2_PLATFORM
	// Abandon any build in progress first: the staging pair is pool storage, so
	// it must go back rather than be freed with the renderer.
	ps2ResetBuildState();
	for (int_t p = 0; p < 2; ++p)
	{
		std::vector<int_t>().swap(ps2RawBuffer[p]);
		ps2VertexCount[p] = 0;
	}
	renderTerrainCacheRelease(ps2TerrainCache);
#endif

#ifdef WII_PLATFORM
	renderTerrainChunkHandlesDestroy(terrainChunkHandles);
#endif

#if PLATFORM_PC
	glRenderList = 0;
	glOcclusionQuery = 0;
#endif
}

void WorldRenderer::setPosition(int_t x, int_t y, int_t z)
{
	if (x == posX && y == posY && z == posZ)
		return;

	setDontDraw();
	posX = x;
	posY = y;
	posZ = z;
	posXPlus = x + sizeWidth  / 2;
	posYPlus = y + sizeHeight / 2;
	posZPlus = z + sizeDepth  / 2;
	posXClip = x & 0x3ff;
	posYClip = y;
	posZClip = z & 0x3ff;
	posXMinus = x - posXClip;
	posYMinus = y - posYClip;
	posZMinus = z - posZClip;

	// Desktop C6 tests the exact section bounds so Fancy Occlusion can classify
	// fully-contained chunks accurately. Console backends keep their tuned
	// conservative margin because their fixed grids and clip paths are different.
	const float f = rendererAabbMargin();
	if (rendererBoundingBox == nullptr)
	{
		rendererBoundingBox = AxisAlignedBB::getBoundingBox(
			(float)x - f, (float)y - f, (float)z - f,
			(float)(x + sizeWidth) + f, (float)(y + sizeHeight) + f, (float)(z + sizeDepth) + f);
	}
	else
	{
		rendererBoundingBox->setBounds(
			(float)x - f, (float)y - f, (float)z - f,
			(float)(x + sizeWidth) + f, (float)(y + sizeHeight) + f, (float)(z + sizeDepth) + f);
	}

	// OptiFine C6 defers rebuilding the occlusion AABB list until this section
	// actually rebuilds. Repositioning a renderer grid can touch hundreds of
	// sections at once; compiling a list for every moved section here creates a
	// large synchronous spike before any useful terrain work begins.
#if PLATFORM_PC
	needsOcclusionBoxUpdate = true;
	isVisibleFromPosition = false;
#endif

	markDirty();
}

#if PLATFORM_PC
void WorldRenderer::updateOcclusionBox()
{
	if (!needsOcclusionBoxUpdate)
		return;

	const float f = rendererAabbMargin();
	renderBeginDisplayList(glRenderList + 2);
	RenderItem::renderAABB(AxisAlignedBB::getBoundingBoxFromPool(
		(float)posXClip - f, (float)posYClip - f, (float)posZClip - f,
		(float)(posXClip + sizeWidth) + f, (float)(posYClip + sizeHeight) + f, (float)(posZClip + sizeDepth) + f));
	renderEndDisplayList();
	needsOcclusionBoxUpdate = false;
}
#endif

void WorldRenderer::updateInFrustrum(ICamera *icamera)
{
#if PLATFORM_PS2
	// PS2 needs the three-state result for its per-section clip fast path.
	const int cls = icamera->classifyBoundingBox(rendererBoundingBox);
	isInFrustum = (cls != 0);
	isFullyInFrustum = (cls == 2);
#elif PLATFORM_WII
	// GX has no occlusion queries, so the stronger fully-inside classification
	// is dead work here. A plain frustum test is the complete Wii contract.
	isInFrustum = icamera->isBoundingBoxInFrustum(rendererBoundingBox);
#else
	if (Config::isOcclusionFancy())
	{
		const int cls = icamera->classifyBoundingBox(rendererBoundingBox);
		isInFrustum = (cls != 0);
		isFullyInFrustum = (cls == 2);
	}
	else
	{
		isInFrustum = icamera->isBoundingBoxInFrustum(rendererBoundingBox);
		isFullyInFrustum = false;
	}
#endif
}




#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
void WorldRenderer::updateRenderer()
{
	if (!needsUpdate)
		return;

	updateOcclusionBox();
	isVisibleFromPosition = false;


	// Vanilla ChunkCache synchronously requested every source chunk. This port's
	// shared ChunkCache deliberately treats missing chunks as air for pathfinding,
	// so the desktop renderer must restore that loading contract explicitly. A
	// provider that cannot supply the chunk yet (for example multiplayer) leaves
	// this renderer dirty and it will retry after the real column is published.
	{
		const int_t ccx0 = JavaArithmetic::intShr(posX - 1, 4);
		const int_t ccx1 = JavaArithmetic::intShr(posX + sizeWidth + 1, 4);
		const int_t ccz0 = JavaArithmetic::intShr(posZ - 1, 4);
		const int_t ccz1 = JavaArithmetic::intShr(posZ + sizeDepth + 1, 4);
		for (int_t ccx = ccx0; ccx <= ccx1; ++ccx)
		{
			for (int_t ccz = ccz0; ccz <= ccz1; ++ccz)
			{
				if (!worldObj->chunkExists(ccx, ccz))
					worldObj->getChunkFromChunkCoords(ccx, ccz);
				if (!worldObj->chunkExists(ccx, ccz))
					return;
			}
		}
	}

	chunksUpdated++;

	int_t x0 = posX,             y0 = posY,              z0 = posZ;
	int_t x1 = posX + sizeWidth, y1 = posY + sizeHeight, z1 = posZ + sizeDepth;


	
	std::vector<TessellatorTextureMesh> stagedExtraTextureMeshes[2];
	for (int_t k1 = 0; k1 < 2; k1++)
		_skipRenderPass[k1] = true;

	Chunk::isLit = false;

	// tileEntityRenderers still holds the old list here (reassigned at the end
	// of this function), so it doubles as the "old" side of the diff below —
	// no need to copy it into a set.
	std::vector<TileEntity *> rebuiltTileEntityRenderers;

	int_t margin = 1;
	ChunkCache chunkcache(worldObj, x0 - margin, y0 - margin, z0 - margin,
	                                x1 + margin, y1 + margin, z1 + margin);

	RenderBlocks renderblocks(&chunkcache);

	Tessellator *tessellator = &Tessellator::instance;

	for (int_t pass = 0; pass < 2; )
	{
		bool hasOtherPass = false;
		bool drewAnything = false;
		bool listOpen = false;

		for (int_t y = y0; y < y1; y++)
		{
			for (int_t z = z0; z < z1; z++)
			{
				for (int_t x = x0; x < x1; x++)
				{
					int_t id = chunkcache.getBlockId(x, y, z);
					if (id <= 0)
						continue;

					if (!listOpen)
					{
						listOpen = true;
						renderBeginDisplayList(glRenderList + pass);
						renderPushMatrix();
						// Translate to the clip-space origin of this chunk
						renderTranslate((float)posXClip, (float)posYClip, (float)posZClip);
						// Slight scale to avoid z-fighting on chunk borders
						float f = 1.000001f;
						renderTranslate(-(float)sizeDepth / 2.0f, -(float)sizeHeight / 2.0f, -(float)sizeDepth / 2.0f);
						renderScale(f, f, f);
						renderTranslate( (float)sizeDepth / 2.0f,  (float)sizeHeight / 2.0f,  (float)sizeDepth / 2.0f);
						tessellator->startDrawingQuads();
						tessellator->setTranslationD(-(double)posX, -(double)posY, -(double)posZ);
					}

					// Collect tile-entity special renderers on pass 0
					if (pass == 0 && Block::isBlockContainer[id])
					{
						TileEntity *te = chunkcache.getBlockTileEntity(x, y, z);
						if (te != nullptr && TileEntityRenderer::instance.hasSpecialRenderer(te) &&
							std::find(rebuiltTileEntityRenderers.begin(), rebuiltTileEntityRenderers.end(), te) == rebuiltTileEntityRenderers.end())
							rebuiltTileEntityRenderers.push_back(te);
					}

					Block *block = Block::blocksList[id];
					int_t blockPass = block->getRenderBlockPass();

					if (blockPass != pass)
					{
						hasOtherPass = true;
						continue;
					}

					drewAnything |= renderblocks.renderBlockByRenderType(block, x, y, z);
				}
			}
		}

		if (listOpen)
		{
			tessellator->captureTextureGroups(stagedExtraTextureMeshes[pass]);
			tessellator->draw();
			for (const TessellatorTextureMesh &group : stagedExtraTextureMeshes[pass])
			{
				renderBindTexture(group.textureId);
				(void)renderDrawCaptured(group.mesh);
			}
			if (!stagedExtraTextureMeshes[pass].empty())
				renderBindTexture(ConnectedTextures::getTerrainTextureId());
			renderPopMatrix();
			renderEndDisplayList();
			tessellator->setTranslationD(0.0, 0.0, 0.0);
		}
		else
		{
			drewAnything = false;
		}

			if (drewAnything || !stagedExtraTextureMeshes[pass].empty())
		{
			_skipRenderPass[pass] = false;
		}

		if (!hasOtherPass)
			break;

		pass++;
	}


	// Propagate tile-entity changes to the global list (linear scans — see the
	// PS2 build path earlier in this file for why hash sets aren't worth it).
	// Added: in the rebuilt list but not the old one
	for (TileEntity *te : rebuiltTileEntityRenderers)
	{
		if (std::find(tileEntityRenderers.begin(), tileEntityRenderers.end(), te) == tileEntityRenderers.end())
			pushUniqueTileEntityRef(tileEntities, te);
	}
	// Removed: in the old list but not the rebuilt one
	for (TileEntity *te : tileEntityRenderers)
	{
		if (std::find(rebuiltTileEntityRenderers.begin(), rebuiltTileEntityRenderers.end(), te) == rebuiltTileEntityRenderers.end())
		{
			eraseAllTileEntityRefs(tileEntities, te);
		}
	}

	isChunkLit  = Chunk::isLit;
	isInitialized = true;
	tileEntityRenderers = rebuiltTileEntityRenderers;
	needsUpdate = false;
}
#endif


void WorldRenderer::markDirty()
{
#ifdef PS2_PLATFORM
	ps2ResetBuildState();
#endif
#ifdef WII_PLATFORM
	wiiResetBuildState();
	renderTerrainChunkHandlesClearStaging(terrainChunkHandles);
#endif
	needsUpdate = true;
}

void WorldRenderer::setDontDraw()
{
	removeTileEntityRenderersFromGlobalList();
#ifdef WII_PLATFORM
	// A renderer is about to be recycled for another world position. Keeping its
	// old, now invisible lists until the replacement can be built creates a
	// deadlock at the GX cap: no new list fits because stale lists own the cache,
	// yet no stale list is released because rebuilding is blocked. Release the
	// commands now while retaining the IDs for this renderer.
	releaseDisplayListsForCache();
	wiiResetBuildState();
	renderTerrainChunkHandlesClearStaging(terrainChunkHandles);
#else
	_skipRenderPass[0] = true;
	_skipRenderPass[1] = true;
#endif
#ifdef PS2_PLATFORM
	// clear(), NOT a swap-to-empty.
	//
	// This runs from setPosition(), i.e. every time the sliding renderer window
	// re-targets a section. On the 5x3x5 grid a horizontal chunk crossing can
	// recycle one 5x3 renderer plane at once.
	// Releasing the storage there and letting the rebuild grow it back by
	// doubling emitted a continuous stream of odd tens-of-KB malloc/free pairs,
	// which is the fragmentation this was supposed to avoid: measured
	// 2026-07-27, mallocFree sat at a steady ~4.5MB of free-but-unusable blocks
	// while the arena kept sbrk'ing (heap 0x01cd1000 -> 0x01f9b000) until
	// `free` hit 0 and the game hit Out of memory -- with mallocUsed itself only
	// up ~2MB over the same window. The memory was there; it was in the wrong
	// shaped holes.
	//
	// Normal capacities remain reusable because each slot keeps its vertical
	// band across repositions. Only an unusually large high-water allocation is
	// released here; that prevents one dense section from permanently inflating
	// the 75-renderer grid without returning to per-rebuild shrink/copy churn.
	// detachFromWorld() still releases every remaining allocation.
	// The staging pair is not cleared here: ps2ResetBuildState() below hands it
	// back to the pool, which empties it. Retention applies to the live buffers
	// this renderer owns, never to borrowed ones.
	for (int_t p = 0; p < 2; ++p)
	{
		const size_t retainedBytes = ps2RawBuffer[p].capacity() * sizeof(int_t);
		if (retainedBytes > PS2_MAX_RETAINED_RAW_MESH_BYTES)
			std::vector<int_t>().swap(ps2RawBuffer[p]);
		else
			ps2RawBuffer[p].clear();
	}

	if (renderTerrainCacheRamBytes(ps2TerrainCache) > PS2_MAX_RETAINED_PACKED_MESH_BYTES)
		renderTerrainCacheRelease(ps2TerrainCache);
	else
		renderTerrainCacheReset(ps2TerrainCache);
	ps2VertexCount[0] = 0;
	ps2VertexCount[1] = 0;
	ps2MissingNeighbourMask = 0u;
	// The face ranges describe a mesh that no longer exists. Leaving them valid
	// would let a repositioned renderer index into the next build's buffer with
	// the previous section's offsets.
	ps2ResetBuildState();
#endif
	isInFrustum = false;
#if PLATFORM_PC || PLATFORM_PS2
	isFullyInFrustum = false;
#endif
#if PLATFORM_PC
	isVisibleFromPosition = false;
#endif
	isInitialized = false;
}

void WorldRenderer::detachFromWorld()
{
	setDontDraw();
#ifdef PS2_PLATFORM
	// Leaving the world for good. setDontDraw() deliberately keeps the mesh
	// storage so a repositioned renderer can reuse it; here there is nothing
	// left to reuse it for, so hand it back. The staging pair was already
	// returned to the pool by setDontDraw()'s ps2ResetBuildState().
	std::vector<int_t>().swap(ps2RawBuffer[0]);
	std::vector<int_t>().swap(ps2RawBuffer[1]);
	renderTerrainCacheRelease(ps2TerrainCache);
#endif
	worldObj = nullptr;
}

#if PLATFORM_PC
void WorldRenderer::callOcclusionQueryList()
{
	renderCallDisplayList(glRenderList + 2);
}

int_t WorldRenderer::getGLCallListForPass(int_t pass)
{
	if (!isInFrustum)
		return -1;
	if (!_skipRenderPass[pass])
		return glRenderList + pass;
	return -1;
}
#endif


bool WorldRenderer::skipAllRenderPasses()
{
	if (!isInitialized)
		return false;
	return _skipRenderPass[0] && _skipRenderPass[1];
}

bool WorldRenderer::skipRenderPass(int_t pass)
{
	if (pass < 0 || pass > 1)
		return true;
	return _skipRenderPass[pass];
}

// Narrow first, subtract second. The result only ever ranks renderers (the
// EntitySorter comparator, the mesh-budget sort) or meets a 256.0f threshold,
// and it is already returned as a float, so the double subtraction bought
// nothing -- while costing three libgcc calls per axis on a CPU with no double
// FPU, several hundred times per frame from inside a sort comparator.

float WorldRenderer::distanceToEntitySquared(Entity *entity)
{
	if (entity == nullptr)
		return 0.0f;
	float dx = (float)entity->posX - (float)posXPlus;
	float dy = (float)entity->posY - (float)posYPlus;
	float dz = (float)entity->posZ - (float)posZPlus;
	return dx * dx + dy * dy + dz * dz;
}
