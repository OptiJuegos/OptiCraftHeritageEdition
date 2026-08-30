#pragma once

#include <cstdint>
#if !defined(PS2_PLATFORM)
#include <unordered_set>
#endif
#include <vector>
#include <string>

#include "platform/PlatformConfig.h" // PLATFORM_PS2, for the RAM reporter below
#include "platform/RenderTerrainAPI.h" // RenderTerrainMeshRam, likewise
#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"
#include "IWorldAccess.h"

class Minecraft;
class RenderEngine;
class World;
class RenderBlocks;
class WorldRenderer;
class Entity;
class EntityLiving;
class EntityPlayer;
class Vec3D;
class ICamera;
class TileEntity;
class Tessellator;
class MovingObjectPosition;
class ItemStack;
class RenderList;
class AxisAlignedBB;
class EntityFX;

// net.minecraft.src.RenderGlobal
// Implementa IWorldAccess para recibir callbacks del mundo
class RenderGlobal : public IWorldAccess
{
public:
	RenderGlobal(Minecraft *minecraft, RenderEngine *renderengine);
	~RenderGlobal();

	// IWorldAccess implementation
	void markBlockAndNeighborsNeedsUpdate(int_t i, int_t j, int_t k) override;
	void markBlockRangeNeedsUpdate(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1) override;
	void onChunkPublished(int_t chunkX, int_t chunkZ) override;
	void playSound(const jstring &s, double d, double d1, double d2, float f, float f1) override;
	void spawnParticle(const jstring &s, double d, double d1, double d2, double d3, double d4, double d5) override;
	void obtainEntitySkin(Entity *entity) override;
	void releaseEntitySkin(Entity *entity) override;
	void updateAllRenderers() override;
	void playRecord(const jstring &s, int_t i, int_t j, int_t k) override;
	void doNothingWithTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity) override;
	void playAuxSFX(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1) override;

	// RenderGlobal methods
	void changeWorld(World *world);
	void loadRenderers();
	void setAllRenderersVisible();
	void renderEntities(Vec3D *vec3d, ICamera *icamera, float f);
	jstring getDebugInfoRenders();
	jstring getDebugInfoEntities();

#if PLATFORM_PS2
	// Total bytes the chunk-section meshes are holding, summed over every
	// WorldRenderer in the grid. Reported in the RAM breakdown because these
	// buffers deliberately retain capacity across rebuilds and repositions (to
	// keep the fragmenting 32MB heap from re-allocating odd tens-of-KB blocks
	// every frame), which makes them a large, permanently-held, and otherwise
	// completely invisible slice of the arena.
	size_t terrainMeshRamBytes() const;
	// The same storage attributed to its five buckets. Adds to out, so a caller
	// can accumulate; see RenderTerrainMeshRam for what each bucket scales with.
	void terrainMeshRamBreakdown(RenderTerrainMeshRam &out) const;
#endif
	int_t sortAndRender(EntityLiving *entityliving, int_t i, double d);
	void renderSky(float f);
	void renderClouds(float f);
	void renderCloudsFancy(float f);
	bool hasCloudFog(double d, double d1, double d2, float f);
	bool updateRenderers(EntityLiving *entityliving, bool flag);
	void drawBlockBreaking(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f);
	void drawSelectionBox(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f);
	void updateClouds();
	void clipRenderersByFrustrum(ICamera *icamera, float f);
#if !PLATFORM_PS2
	void renderAllRenderLists(int_t i, double d);
#endif
	bool isCloudFog(double x, double y, double z, float partialTicks);  // hasCloudFog
	// func_28137_f — clears/resets all world renderers (called on shutdown/world change)
	void clearWorldRenderers();

	// LOCAL MODIFICATION (BetaPlusPlus Wii port): how many WorldRenderers are
	// still queued for a mesh rebuild. Exposed so Minecraft::changeWorld()'s
	// loading-screen warmup can log how much of its work it actually finished
	// within its time budget, instead of only ever reporting "capped" with no
	// sense of scale (see the warmup loop's PLATFORM_RAMLOG call).
	int_t pendingRendererUpdateCount() const { return (int_t)worldRenderersToUpdate.size(); }

	// Public fields
	std::vector<TileEntity *> tileEntities;
	float damagePartialTime = 0.0f;

private:
	// func_949_a — marks all WorldRenderers whose chunk coords fall in [i..l, j..i1, k..j1] dirty
	void markRenderersInRange(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	void renderStars();
	void markRenderersForNewPosition(int_t i, int_t j, int_t k);
#ifdef WII_PLATFORM
	void evictWiiMeshCache(EntityLiving *entityliving);
#endif
	void enqueueRendererUpdate(WorldRenderer *worldrenderer);
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	void enqueueRendererUpdatePriority(WorldRenderer *worldrenderer);
#endif
	void dequeueRendererUpdate(WorldRenderer *worldrenderer);
	void compactRendererUpdateQueue();
	bool isRendererUpdateMoving(EntityLiving *entityliving);
	bool isRendererUpdateActing(EntityLiving *entityliving) const;
#if PLATFORM_PC
	void checkOcclusionQueryResult(int_t i, int_t j, double playerX, double playerY, double playerZ);
#endif
	int_t renderSortedRenderers(int_t i, int_t j, int_t k, double d);
	void drawOutlinedBoundingBox(AxisAlignedBB *axisalignedbb);
	EntityFX *spawnParticleEffect(const jstring &s, double d, double d1, double d2, double d3, double d4, double d5);

	// Private fields
	World *worldObj = nullptr;
	RenderEngine *renderEngine = nullptr;
	std::vector<WorldRenderer *> worldRenderersToUpdate;
#if !defined(PS2_PLATFORM)
	std::unordered_set<WorldRenderer *> worldRenderersQueuedForUpdate;
#endif
	std::vector<WorldRenderer *> rendererUpdateCandidates;
	WorldRenderer **sortedWorldRenderers = nullptr;
	WorldRenderer **worldRenderers = nullptr;
	int_t renderChunksWide = 0;
	int_t renderChunksTall = 0;
	int_t renderChunksDeep = 0;
	// World Y section (block>>4) occupied by vertical renderer slot 0. With the
	// centered vertical window (PLATFORM_CENTER_VERTICAL_RENDERERS) the slot/section
	// mapping is slot = section - verticalStartSection, not section % renderChunksTall.
	// markRenderersInRange must use this so block edits dirty the right renderer.
	int_t verticalStartSection = 0;
#if PLATFORM_PC
	int_t glRenderListBase = 0;
#endif
	Minecraft *mc = nullptr;
	RenderBlocks *globalRenderBlocks = nullptr;
#if PLATFORM_PC
	std::vector<int_t> glOcclusionQueryBase;
	bool occlusionEnabled = false;
#endif
	int_t cloudOffsetX = 0;
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	RenderStaticMesh starMesh;
	RenderStaticMesh skyMesh;
	RenderStaticMesh skyMesh2;
#else
	int_t starGLCallList = 0;
	int_t glSkyList = 0;
	int_t glSkyList2 = 0;
#endif
	int_t minBlockX = 0;
	int_t minBlockY = 0;
	int_t minBlockZ = 0;
	int_t maxBlockX = 0;
	int_t maxBlockY = 0;
	int_t maxBlockZ = 0;
	int_t renderDistance = -1;
	int_t renderEntitiesStartupCounter = 2;
	int_t countEntitiesTotal = 0;
	int_t countEntitiesRendered = 0;
	int_t countEntitiesHidden = 0;
#if PLATFORM_PC
	std::vector<int_t> occlusionResult;
#endif
	int_t renderersLoaded = 0;
	int_t renderersBeingClipped = 0;
	int_t renderersBeingOccluded = 0;
	int_t renderersBeingRendered = 0;
	int_t renderersSkippingRenderPass = 0;
	int_t worldRenderersCheckIndex = 0;
	std::vector<WorldRenderer *> renderBatchRenderers;
#if !PLATFORM_PS2
	RenderList *allRenderLists[4];
#endif
	double prevSortX = -9999.0;
	double prevSortY = -9999.0;
	double prevSortZ = -9999.0;
	double prevReposX = -9999.0;
	double prevReposY = -9999.0;
	double prevReposZ = -9999.0;
	uint64_t lastRendererMoveTimeMs = 0;
	int_t frustrumCheckOffset = 0;
};
