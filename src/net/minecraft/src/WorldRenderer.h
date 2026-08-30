#pragma once

#include <vector>
#include "java/Type.h"

#include "platform/RenderTerrainAPI.h"
#include "platform/PlatformConfig.h"
#include "Tessellator.h"

class World;
class TileEntity;
class ICamera;
class Entity;
class AxisAlignedBB;

// net.minecraft.src.WorldRenderer
class WorldRenderer
{
public:
	WorldRenderer(World *world, std::vector<TileEntity *> *tileEntities, int_t posX, int_t posY, int_t posZ, int_t size, int_t glListId);
	~WorldRenderer();

	void setPosition(int_t x, int_t y, int_t z);
	void updateInFrustrum(ICamera *icamera);
	void updateRenderer();
	void markDirty();
#ifdef WII_PLATFORM
	// Wii mesh-cache eviction. Returns true only if this renderer was holding
	// recorded terrain geometry that has now been released.
	bool releaseDisplayListsForCache();

	// Whether releaseDisplayListsForCache() would free anything, without the
	// side effects of calling it. The eviction scan has to ask before it picks a
	// victim, because it keeps only the farthest candidate and gives up when
	// that one turns out to have nothing to release.
	//
	// Stricter than releaseDisplayListsForCache()'s own guard by the
	// isInitialized term: a renderer that has never been meshed has both
	// _skipRenderPass entries still false from the constructor, so it reads as
	// holding geometry and releasing it only marks it dirty again. That is a
	// rebuild of an empty distant section bought with an eviction slot and no
	// memory returned. setDontDraw() wants that clearing behaviour and keeps
	// calling releaseDisplayListsForCache() directly, so its guard is unchanged.
	bool holdsRecordedTerrain() const;
#endif
#if PLATFORM_PC
	void callOcclusionQueryList();
	int_t getGLCallListForPass(int_t pass);
#endif
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM)
	void renderExtraTerrainMeshes(int_t pass);
	bool isTerrainBuildInProgress() const;
	bool lastTerrainBuildStepDidWork() const;
#ifdef PS2_PLATFORM
	bool terrainSourcesReady() const;
	bool needsRebuildForPublishedChunk(int_t chunkX, int_t chunkZ) const;
#endif
#endif
#ifdef WII_PLATFORM
	int_t getTerrainHandleForPass(int_t pass) const;

	// Whether this section's mesh for that pass can contain texels the alpha
	// test rejects, as reported by RenderBlocks while it was built. False means
	// the pass may run with the alpha test off, which is what lets GX reject
	// fragments against Z before texturing them. Conservative: an unbuilt or
	// out-of-range pass answers true.
	bool terrainPassNeedsAlphaTest(int_t pass) const;
#endif

#ifdef PS2_PLATFORM
	void renderPassImmediate(int_t pass);
	void renderPassCached(int_t pass);
	// Interpolated eye position for this terrain pass, in world coordinates. It
	// feeds both face-bucket culling and the section-to-camera translation. The
	// native backend also snapshots projection*modelview here once, before the
	// visible-section loop, so individual chunks never touch the GL matrix stack.
	static void setTerrainViewerPosition(double x, double y, double z);

	// True when the last completed build step actually meshed blocks. False
	// means it returned early on the generation gate (a source chunk is still
	// missing), which costs almost nothing and must not consume a slot of the
	// per-frame meshing budget. See RenderGlobal::updateRenderers.
	// Bytes this renderer holds for mesh storage, counting CAPACITY rather than
	// size: setDontDraw() and the completion swap both deliberately retain
	// capacity to avoid re-allocating on a heap that fragments, so capacity is
	// what the allocator is really keeping out of circulation.
	size_t terrainMeshRamBytes() const;

	// Adds this renderer's four owned buckets to out; does not clear it, so a
	// caller can accumulate over the grid. The summed figure could not answer
	// the question it was added for -- it tracked the whole of the arena's
	// growth without saying whether that was geometry (scales with vertex
	// count) or face-group ranges (scale with atlas fragmentation).
	// RenderTerrainMeshRam::stagingPool is pool-wide and is filled by
	// RenderGlobal, never here.
	void addTerrainMeshRam(RenderTerrainMeshRam &out) const;
	static size_t sharedOpaquePublishScratchRamBytes();
#endif

	bool skipAllRenderPasses();
	bool skipRenderPass(int_t pass);
	float distanceToEntitySquared(Entity *entity);
	void setDontDraw();
	void detachFromWorld();
	void cleanup();

	static int_t chunksUpdated;

	// Public fields matching Java
	World *worldObj;
	bool needsUpdate;
	bool queuedForUpdate = false;
	bool isChunkLit;
#if PLATFORM_PC
	bool isWaitingOnOcclusionQuery;
#endif
	bool isVisible;
	bool isInFrustum;
#if PLATFORM_PC || PLATFORM_PS2
	// Stronger than isInFrustum: PS2 uses it for its clip fast path and desktop
	// Fancy Occlusion uses it to avoid querying boxes that cross a frustum plane.
	bool isFullyInFrustum;
#endif
#if PLATFORM_PC
	bool isVisibleFromPosition;
	double visibleFromX;
	double visibleFromY;
	double visibleFromZ;
	int_t glOcclusionQuery;
#endif
	int_t chunkIndex;

	int_t posX;
	int_t posY;
	int_t posZ;
	int_t sizeWidth;
	int_t sizeHeight;
	int_t sizeDepth;

	// Center positions (posX + size/2) for distance calc
	int_t posXPlus;
	int_t posYPlus;
	int_t posZPlus;

	// Clip positions (i & 0x3ff)
	int_t posXClip;
	int_t posYClip;
	int_t posZClip;

	// Offsets (i - posXClip) — used by RenderGlobal for translation
	int_t posXMinus;
	int_t posYMinus;
	int_t posZMinus;

	AxisAlignedBB *rendererBoundingBox;

	// Tile entities with special renderers collected during updateRenderer()
	std::vector<TileEntity *> tileEntityRenderers;

private:
#if PLATFORM_PC
	int_t glRenderList;
	bool needsOcclusionBoxUpdate;
	void updateOcclusionBox();
#endif
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM)
	// OptiFine CTM atlases are kept as backend-neutral captured meshes. The
	// normal terrain mesh remains on /terrain.png; these groups are replayed
	// after it with their own texture binding.
	std::vector<TessellatorTextureMesh> extraTextureMeshes[2];
#endif
#ifdef WII_PLATFORM
	// Native GX geometry uses an opaque handle namespace independent from
	// OpenGL display-list names. Two handles are live and two are staging so a
	// rebuild can publish atomically without touching the currently visible mesh.
	RenderTerrainChunkHandles terrainChunkHandles;

	// Published alongside terrainChunkHandles.live, and staged next to the mesh
	// it describes so the two can never disagree about the same geometry.
	bool wiiPassNeedsAlphaTest[2];
	bool wiiBuildNeedsAlphaTest[2];

	// Incremental Wii mesh build state. CPU-side vertices accumulate over bounded
	// block steps while the currently published GX lists remain visible. Staging
	// lists are compiled only after a whole pass is complete and swapped atomically.
	std::vector<int_t> wiiBuildRawBuffer[2];
	std::vector<TessellatorTextureMesh> wiiBuildExtraTextureMeshes[2];
	int_t wiiBuildVertexCount[2];
	bool wiiBuildHasTexture[2];
	bool wiiBuildHasColor[2];
	bool wiiBuildDrew[2];
	bool wiiBuildActive;
	// Snapshot of the ChunkCache source columns used by the staging mesh. If a
	// streamed neighbour appears or disappears between incremental steps, the
	// partial mesh must restart instead of mixing both source states.
	unsigned int wiiBuildSourceAvailability;
	bool wiiBuildSourceAvailabilityValid;
	int_t wiiBuildPass;
	int_t wiiBuildCursor;
	bool wiiBuildHasPass1;
	bool wiiBuildChunkLit;
	bool wiiStepDidWork;
	std::vector<TileEntity *> wiiBuildTileEntityRenderers;

	void wiiResetBuildState();
	void wiiBeginBuildState();
	bool wiiBuildRendererStep(int_t blockBudget);
#endif
	bool isInitialized;
	bool _skipRenderPass[2];
#ifdef PS2_PLATFORM
	std::vector<int_t> ps2RawBuffer[2];
	int_t ps2VertexCount[2];
	int_t ps2DrawMode[2];
	bool ps2HasTexture[2];
	bool ps2HasColor[2];
	bool ps2HasNormals[2];

	// Incremental PS2 mesh build state.  Active cache remains visible while the
	// staging buffers are filled over multiple frames.
	//
	// The staging pair is leased from terrain staging pool rather than owned here:
	// only a renderer with a build in progress needs one, and the grid is far
	// larger than the number of concurrent builds. ps2BuildStagingSlot is
	// RENDER_TERRAIN_STAGING_INVALID_SLOT whenever no build is active, and every path
	// that abandons a build must return the slot or terrain streaming stalls
	// for good. ps2BuildBuffers() is null exactly when no slot is held, so a
	// missing lease faults at the use site instead of corrupting another
	// renderer's staging mesh.
	int ps2BuildStagingSlot;
	std::vector<int_t> *ps2BuildBuffers() const;
	std::vector<TessellatorTextureMesh> ps2BuildExtraTextureMeshes[2];
	int_t ps2BuildVertexCount[2];
	int_t ps2BuildDrawMode[2];
	bool ps2BuildHasTexture[2];
	bool ps2BuildHasColor[2];
	bool ps2BuildHasNormals[2];
	bool ps2BuildDrew[2];
	bool ps2BuildActive;
	// Bitset of the 2x2--3x3 ChunkCache source columns that existed when the
	// current staging mesh began. A change invalidates only this active build.
	unsigned int ps2BuildSourceAvailability;
	bool ps2BuildSourceAvailabilityValid;
	unsigned int ps2MissingNeighbourMask;
	int_t ps2BuildPass;
	int_t ps2BuildCursor;
	// Which greedy face direction the opaque pass will sweep next, 0..6.
	// PS2_GREEDY_FACE_COUNT means the greedy stage of this build is finished and
	// the per-block loop owns the remaining budget.
	int_t ps2BuildGreedyFace;
	// First unprocessed plane (0..15) inside ps2BuildGreedyFace. Greedy merging
	// never crosses face planes, so this can time-slice a direction exactly.
	int_t ps2BuildGreedySlice;
	std::vector<TileEntity *> ps2BuildTileEntityRenderers;

	// Atlas-tile / face-direction / spatial-cluster ranges of the live opaque
	// mesh, produced by ps2_mesh_sort_faces at build completion. Only pass 0 is
	// reordered; pass 1 is blended and needs its original back-to-front order.
	RenderTerrainBackendCache ps2TerrainCache;

	// Set during the pass-0 sweep when any block in the section renders in pass
	// 1. When it stays false the pass-1 sweep is skipped entirely instead of
	// walking all 4096 blocks to emit nothing -- which is what the FRAME log's
	// "terrain pass1 withGeom=0" meant it was doing on every section.
	bool ps2BuildHasPass1;

	// See lastTerrainBuildStepDidWork().
	bool ps2StepDidWork;

	void ps2ResetBuildState();
	// False when the staging pool is full. The caller must not treat the build
	// as started in that case; it retries on a later frame.
	bool ps2BeginBuildState();
	bool ps2BuildRendererStep(int_t blockBudget);
#endif

	static void eraseAllTileEntityRefs(std::vector<TileEntity *> *list, TileEntity *te);
	static void pushUniqueTileEntityRef(std::vector<TileEntity *> *list, TileEntity *te);
	void removeTileEntityRenderersFromGlobalList();
	std::vector<TileEntity *> *tileEntities;
};
