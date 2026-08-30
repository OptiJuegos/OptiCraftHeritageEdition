#ifdef WII_PLATFORM

#include "net/minecraft/src/WorldRenderer.h"
#include "java/Arithmetic.h"

#include "platform/RenderAPI.h"
#include "platform/RenderTerrainAPI.h"
#include "net/minecraft/src/World.h"
#include "net/minecraft/src/Config.h"
#include "net/minecraft/src/ConnectedTextures.h"
#include "net/minecraft/src/Block.h"
#include "net/minecraft/src/RenderBlocks.h"
#include "net/minecraft/src/Tessellator.h"
#include "net/minecraft/src/Chunk.h"
#include "net/minecraft/src/ExtendedBlockStorage.h"
#include "net/minecraft/src/ChunkCache.h"
#include "net/minecraft/src/TileEntity.h"
#include "net/minecraft/src/TileEntityRenderer.h"
#include "platform/PlatformTuning.h"
#include "platform/PlatformCompat.h"
#include "wii/render/WiiNativeDraw.h"
#include "wii/render/WiiMeshSort.h"

#include <algorithm>
#include <cstdint>
#include <utility>

void WorldRenderer::updateRenderer()
{
    if (!needsUpdate)
        return;

    wiiBuildRendererStep(PLATFORM_WII_CHUNK_BUILD_BLOCKS_PER_STEP);
}

void WorldRenderer::wiiResetBuildState()
{
	wiiBuildActive = false;
	wiiBuildSourceAvailability = 0u;
	wiiBuildSourceAvailabilityValid = false;
	wiiBuildPass = 0;
	wiiBuildCursor = 0;
	wiiBuildHasPass1 = false;
	wiiBuildChunkLit = false;
	wiiStepDidWork = false;
	wiiBuildTileEntityRenderers.clear();
	for (int_t p = 0; p < 2; ++p)
	{
		std::vector<int_t>().swap(wiiBuildRawBuffer[p]);
		wiiBuildVertexCount[p] = 0;
		wiiBuildHasTexture[p] = false;
		wiiBuildHasColor[p] = false;
		wiiBuildDrew[p] = false;
		wiiBuildNeedsAlphaTest[p] = false;
		wiiBuildExtraTextureMeshes[p].clear();
	}
}

void WorldRenderer::wiiBeginBuildState()
{
	wiiResetBuildState();
	wiiBuildActive = true;
	renderTerrainChunkHandlesClearStaging(terrainChunkHandles);
}

bool WorldRenderer::isTerrainBuildInProgress() const
{
	return wiiBuildActive;
}

bool WorldRenderer::lastTerrainBuildStepDidWork() const
{
	return wiiStepDidWork;
}

bool WorldRenderer::wiiBuildRendererStep(int_t blockBudget)
{
	wiiStepDidWork = false;
	if (worldObj == nullptr)
	{
		wiiResetBuildState();
		needsUpdate = false;
		return true;
	}

	const int_t x0 = posX;
	const int_t y0 = posY;
	const int_t z0 = posZ;
	const int_t x1 = posX + sizeWidth;
	const int_t y1 = posY + sizeHeight;
	const int_t z1 = posZ + sizeDepth;

	// Do not bake temporary air into a partial mesh. Snapshot every source column
	// sampled by ChunkCache so a streamed neighbour changing between two bounded
	// steps restarts only this staging mesh. Request at most one missing in-radius
	// source per step and leave the currently published list untouched meanwhile.
	unsigned int sourceAvailability = 0u;
	{
		const int_t ccx0 = JavaArithmetic::intShr(x0 - 1, 4);
		const int_t ccx1 = JavaArithmetic::intShr(x1 + 1, 4);
		const int_t ccz0 = JavaArithmetic::intShr(z0 - 1, 4);
		const int_t ccz1 = JavaArithmetic::intShr(z1 + 1, 4);
		unsigned int sourceBit = 1u;
		for (int_t ccx = ccx0; ccx <= ccx1; ++ccx)
		{
			for (int_t ccz = ccz0; ccz <= ccz1; ++ccz, sourceBit <<= 1)
			{
				if (worldObj->chunkExists(ccx, ccz))
					sourceAvailability |= sourceBit;
			}
		}

		if (wiiBuildSourceAvailabilityValid &&
			wiiBuildSourceAvailability != sourceAvailability)
		{
			wiiBeginBuildState();
			wiiBuildSourceAvailability = sourceAvailability;
			wiiBuildSourceAvailabilityValid = true;
			return false;
		}

		int_t requestedDependencies = 0;
		sourceBit = 1u;
		for (int_t ccx = ccx0; ccx <= ccx1; ++ccx)
		{
			for (int_t ccz = ccz0; ccz <= ccz1; ++ccz, sourceBit <<= 1)
			{
				if ((sourceAvailability & sourceBit) != 0u ||
					!worldObj->isChunkInLoadRadius(ccx, ccz))
					continue;

				worldObj->getChunkFromChunkCoords(ccx, ccz);
				++requestedDependencies;
				if (requestedDependencies >= PLATFORM_WII_RENDERER_DEPENDENCY_REQUESTS_PER_STEP)
					return false;
			}
		}
		if (requestedDependencies > 0)
			return false;
	}

	const int_t centerChunkX = JavaArithmetic::intShr(x0, 4);
	const int_t centerChunkZ = JavaArithmetic::intShr(z0, 4);
	Chunk *centerChunk = worldObj->getChunkFromChunkCoords(centerChunkX, centerChunkZ);
	ExtendedBlockStorage *centerStorage = centerChunk != nullptr
		? centerChunk->getBlockStorage(JavaArithmetic::intShr(y0, 4))
		: nullptr;
	const bool emptySection = centerStorage == nullptr || centerStorage->getIsEmpty();

	if (!wiiBuildActive || emptySection)
	{
		wiiBeginBuildState();
		wiiBuildSourceAvailability = sourceAvailability;
		wiiBuildSourceAvailabilityValid = true;
	}

	if (emptySection)
	{
		// An empty 16^3 storage cannot contribute geometry to this renderer.
		// Publish empty staging handles so a previously non-empty live mesh is
		// retired atomically, then reuse the normal completion path below.
		wiiBuildPass = 2;
		wiiBuildCursor = 0;
	}

	const int_t totalBlocks = sizeWidth * sizeHeight * sizeDepth;
	if (blockBudget <= 0)
		blockBudget = totalBlocks;

	const uint64_t stepStartUs = PlatformCompat::getMonotonicMicros();
	int_t processed = 0;

	while (wiiBuildPass < 2)
	{
		if (wiiBuildPass == 1 && !wiiBuildHasPass1)
		{
			wiiBuildPass = 2;
			wiiBuildCursor = 0;
			break;
		}

		Chunk::isLit = false;
		ChunkCache chunkcache(worldObj, x0 - 1, y0 - 1, z0 - 1, x1 + 1, y1 + 1, z1 + 1);
		RenderBlocks renderblocks(&chunkcache);
		Tessellator *tessellator = &Tessellator::instance;
		tessellator->startDrawingQuads();
		tessellator->setTranslationD(-(double)posX, -(double)posY, -(double)posZ);

		bool stepDrew = false;
		while (wiiBuildCursor < totalBlocks && processed < blockBudget)
		{
			const int_t cursor = wiiBuildCursor++;
			const int_t lx = cursor % sizeWidth;
			const int_t yz = cursor / sizeWidth;
			const int_t lz = yz % sizeDepth;
			const int_t ly = yz / sizeDepth;
			const int_t x = x0 + lx;
			const int_t y = y0 + ly;
			const int_t z = z0 + lz;
			++processed;

			const int_t id = chunkcache.getBlockId(x, y, z);
			if (id > 0)
			{
				if (wiiBuildPass == 0 && Block::isBlockContainer[id])
				{
					TileEntity *te = chunkcache.getBlockTileEntity(x, y, z);
					if (te != nullptr && TileEntityRenderer::instance.hasSpecialRenderer(te) &&
						std::find(wiiBuildTileEntityRenderers.begin(), wiiBuildTileEntityRenderers.end(), te) == wiiBuildTileEntityRenderers.end())
						wiiBuildTileEntityRenderers.push_back(te);
				}

				Block *block = Block::blocksList[id];
				const int_t blockPass = block->getRenderBlockPass();
				if (wiiBuildPass == 0 && blockPass != 0)
					wiiBuildHasPass1 = true;
				if (blockPass == wiiBuildPass)
					stepDrew |= renderblocks.renderBlockByRenderType(block, x, y, z);
			}

			if ((processed & 15) == 0 && PLATFORM_WII_CHUNK_BUILD_STEP_US > 0)
			{
				const uint64_t nowUs = PlatformCompat::getMonotonicMicros();
				if (nowUs > stepStartUs && nowUs - stepStartUs >= (uint64_t)PLATFORM_WII_CHUNK_BUILD_STEP_US)
					break;
			}
		}

		tessellator->captureTextureGroups(wiiBuildExtraTextureMeshes[wiiBuildPass], true);
		wiiBuildDrew[wiiBuildPass] = wiiBuildDrew[wiiBuildPass] || stepDrew;
		// RenderBlocks is constructed per step, so this has to be collected before
		// it goes out of scope; the accumulator spans the whole staging mesh.
		wiiBuildNeedsAlphaTest[wiiBuildPass] |= renderblocks.usedAlphaTestedTexture;

		static RenderCapturedMesh stepMesh;
		stepMesh.clear();
		if (tessellator->capture(stepMesh))
		{
			const int_t stepVertices = stepMesh.vertexCount;
			wiiBuildRawBuffer[wiiBuildPass].insert(wiiBuildRawBuffer[wiiBuildPass].end(),
				stepMesh.raw.begin(), stepMesh.raw.end());
			wiiBuildVertexCount[wiiBuildPass] += stepVertices;
			wiiBuildHasTexture[wiiBuildPass] |= stepMesh.hasTexture;
			wiiBuildHasColor[wiiBuildPass] |= stepMesh.hasColor;
			wiiBuildDrew[wiiBuildPass] |= stepDrew;
		}
		tessellator->setTranslationD(0.0, 0.0, 0.0);
		wiiBuildChunkLit |= Chunk::isLit;
		wiiStepDidWork |= processed > 0;

		if (wiiBuildCursor < totalBlocks)
			return false;

		if (wiiBuildVertexCount[wiiBuildPass] > 0 && !wiiBuildRawBuffer[wiiBuildPass].empty())
		{
			const RenderPrimitive primitive = Tessellator::convertQuadsToTriangles
				? RenderPrimitive::Triangles : RenderPrimitive::Quads;

			const std::vector<int_t>* compileBuffer = &wiiBuildRawBuffer[wiiBuildPass];
#if PLATFORM_MESH_FACE_SORT
			// Only the opaque pass. Pass 1 is blended and depends on the
			// back-to-front order it was emitted in, which a reorder destroys.
			//
			// One shared reorder buffer for every section, deliberately static:
			// the alternative is a section-sized allocation and free on every
			// completed build, which is the allocator traffic the chunk buffer
			// pool exists to avoid. It settles at the largest section the sort
			// accepted (PLATFORM_MESH_SORT_MAX_QUADS bounds that at 4096 quads,
			// 512 KB worst case) and stays there.
			static std::vector<int_t> sortedOpaque;
			RenderTerrainFaceGroups faceGroups;
			const RenderTerrainFaceGroups* faceGroupsPtr = nullptr;
			sortedOpaque.clear();
			if (wiiBuildPass == 0 && primitive == RenderPrimitive::Quads &&
				(wiiBuildVertexCount[0] & 3) == 0)
			{
				WiiFaceGroupRanges ranges;
				if (wii_sort_opaque_faces(wiiBuildRawBuffer[0], sortedOpaque,
				                          wiiBuildVertexCount[0] / 4,
				                          PLATFORM_MESH_SORT_MAX_QUADS, &ranges))
				{
					compileBuffer = &sortedOpaque;
					static_assert(WiiFaceGroupRanges::kGroupCount == RenderTerrainFaceGroups::kGroupCount,
					              "sort and compile face group counts must agree");
					for (int g = 0; g < RenderTerrainFaceGroups::kGroupCount; ++g)
					{
						faceGroups.quadCount[g] = (int)ranges.quadCount[g];
						faceGroups.planeMin[g] = ranges.planeMin[g];
						faceGroups.planeMax[g] = ranges.planeMax[g];
					}
					faceGroupsPtr = &faceGroups;
				}
			}
#endif

			RenderInterleavedMesh mesh;
			mesh.data = compileBuffer->data();
			mesh.stride = 32;
			mesh.count = wiiBuildVertexCount[wiiBuildPass];
			mesh.primitive = primitive;
			mesh.hasTexture = wiiBuildHasTexture[wiiBuildPass];
			mesh.texCoordOffset = 12;
			mesh.hasColor = wiiBuildHasColor[wiiBuildPass];
			mesh.colorOffset = 20;
			RenderTerrainCompileInfo compileInfo;
			compileInfo.translateX = (float)posXClip;
			compileInfo.translateY = (float)posYClip;
			compileInfo.translateZ = (float)posZClip;
			compileInfo.sectionSize = (float)sizeDepth;
#if PLATFORM_MESH_FACE_SORT
			compileInfo.faceGroups = faceGroupsPtr;
			// The section's true world corner. The tessellator translated the
			// vertices by exactly this (setTranslationD above), so it is what
			// puts the face planes and the eye in one space.
			compileInfo.worldOriginX = (float)posX;
			compileInfo.worldOriginY = (float)posY;
			compileInfo.worldOriginZ = (float)posZ;
#endif
			if (!renderCompileTerrainMesh(terrainChunkHandles.staging[wiiBuildPass], mesh, compileInfo))
			{
				needsUpdate = true;
				return false;
			}
		}
		else
		{
			renderTerrainClearChunkHandle(terrainChunkHandles.staging[wiiBuildPass]);
		}

		std::vector<int_t>().swap(wiiBuildRawBuffer[wiiBuildPass]);
		wiiBuildPass++;
		wiiBuildCursor = 0;
		if (processed >= blockBudget)
			return false;
		if (PLATFORM_WII_CHUNK_BUILD_STEP_US > 0)
		{
			const uint64_t nowUs = PlatformCompat::getMonotonicMicros();
			if (nowUs > stepStartUs && nowUs - stepStartUs >= (uint64_t)PLATFORM_WII_CHUNK_BUILD_STEP_US)
				return false;
		}
	}

	// Linear scans instead of hash sets — see the PS2 build path above for why.
	for (TileEntity *te : wiiBuildTileEntityRenderers)
	{
		if (std::find(tileEntityRenderers.begin(), tileEntityRenderers.end(), te) == tileEntityRenderers.end())
			pushUniqueTileEntityRef(tileEntities, te);
	}
	for (TileEntity *te : tileEntityRenderers)
	{
		if (std::find(wiiBuildTileEntityRenderers.begin(), wiiBuildTileEntityRenderers.end(), te) == wiiBuildTileEntityRenderers.end())
			eraseAllTileEntityRefs(tileEntities, te);
	}

	renderTerrainChunkHandlesPublish(terrainChunkHandles);
	for (int_t p = 0; p < 2; ++p)
	{
		extraTextureMeshes[p].swap(wiiBuildExtraTextureMeshes[p]);
		wiiBuildExtraTextureMeshes[p].clear();
		wiiPassNeedsAlphaTest[p] = wiiBuildNeedsAlphaTest[p];
		_skipRenderPass[p] = !(wiiBuildDrew[p] &&
			(wiiBuildVertexCount[p] > 0 || !extraTextureMeshes[p].empty()));
	}
	tileEntityRenderers = wiiBuildTileEntityRenderers;
	isChunkLit = wiiBuildChunkLit;
	isInitialized = true;
	needsUpdate = false;
	chunksUpdated++;
	wiiResetBuildState();
	return true;
}

bool WorldRenderer::holdsRecordedTerrain() const
{
	return isInitialized && !(_skipRenderPass[0] && _skipRenderPass[1]);
}

bool WorldRenderer::releaseDisplayListsForCache()
{
	// Intentionally not holdsRecordedTerrain(): setDontDraw() calls this to clear
	// a renderer that is being recycled to a new world position, and it relies on
	// the never-meshed case still running the body below to set the skip flags --
	// which is what the non-Wii branch of setDontDraw() does inline.
	if (_skipRenderPass[0] && _skipRenderPass[1])
		return false;

	renderTerrainChunkHandlesClearLive(terrainChunkHandles);
	_skipRenderPass[0] = true;
	_skipRenderPass[1] = true;
	needsUpdate = true;
	isInitialized = false;
	return true;
}

void WorldRenderer::renderExtraTerrainMeshes(int_t pass)
{
	if (pass < 0 || pass > 1 || _skipRenderPass[pass] || extraTextureMeshes[pass].empty())
		return;

	renderPushMatrix();
	// RenderList already translated by posMinus-eye. Captured CTM vertices are
	// section-local, so add the same clip-origin translation baked into native GX.
	renderTranslate((float)posXClip, (float)posYClip, (float)posZClip);
	for (const TessellatorTextureMesh &group : extraTextureMeshes[pass])
	{
		renderBindTexture(group.textureId);
		(void)renderDrawCaptured(group.mesh);
	}
	renderPopMatrix();

	// Captured CTM meshes use the generic native GX path, which replaces the
	// terrain vertex descriptor/state. Force the next chunk batch to prepare
	// its terrain signature again instead of replaying a display list against
	// the generic descriptor.
	wii_native_invalidate_terrain_pass();

	// Terrain state assumes /terrain.png is still bound after each section.
	renderBindTexture(ConnectedTextures::getTerrainTextureId());
}

int_t WorldRenderer::getTerrainHandleForPass(int_t pass) const
{
	if (pass < 0 || pass > 1 || !isInFrustum || _skipRenderPass[pass])
		return 0;
	return terrainChunkHandles.live[pass];
}

bool WorldRenderer::terrainPassNeedsAlphaTest(int_t pass) const
{
	if (pass < 0 || pass > 1)
		return true;
	return wiiPassNeedsAlphaTest[pass];
}


#endif // WII_PLATFORM
