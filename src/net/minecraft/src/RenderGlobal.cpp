#include "platform/WorkProfiler.h"
#include "platform/ExtendedProfiler.h"
#include "RenderGlobal.h"
#include "platform/Log.h"
#include "java/Arithmetic.h"
#include "java/Math.h"
#include "java/String.h"
#include "platform/PlatformTuning.h"
#if PLATFORM_PC_LEGACY
#include "pc/render/PcLegacyMeshScheduler.h"
#endif

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#if PLATFORM_PS2
#include <cstdio>
#include "ps2/render/Ps2Vu0MeshFinalize.h"
#include "ps2/render/Ps2GsQueue.h"
#endif

#include "platform/RenderAPI.h"
#if PLATFORM_PC_LEGACY
#include "pc/render/PcLegacyStaticTileEntityMesh.h"
#endif
#include "platform/RenderTerrainAPI.h"
#include "platform/RenderTerrainStaging.h"
#include "client/Minecraft.h"
#include "Config.h"
#include "CustomColorizer.h"
#include "Block.h"
#include "BlockLeaves.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "EntityRenderer.h"
#include "EffectRenderer.h"
#include "EntityBubbleFX.h"
#include "EntitySmokeFX.h"
#include "EntityNoteFX.h"
#include "EntityPortalFX.h"
#include "EntityExplodeFX.h"
#include "EntityFlameFX.h"
#include "EntityLavaFX.h"
#include "EntityFootStepFX.h"
#include "EntitySplashFX.h"
#include "EntityReddustFX.h"
#include "EntitySnowShovelFX.h"
#include "EntityHeartFX.h"
#include "EntityAuraFX.h"
#include "EntityBreakingFX.h"
#include "EntityCloudFX.h"
#include "EntityCritFX.h"
#include "EntityDropParticleFX.h"
#include "EntityDiggingFX.h"
#include "EntityEnchantmentTableParticleFX.h"
#include "EntityHugeExplodeFX.h"
#include "EntityLargeExplodeFX.h"
#include "EntitySpellParticleFX.h"
#include "EntitySuspendFX.h"
#include "EnumMovingObjectType.h"
#include "FontRenderer.h"
#include "Frustrum.h"
#include "GameSettings.h"
#include "legacy/LegacyLook.h"
#if PLATFORM_PC
#include "GLAllocation.h"
#endif
#include "GuiIngame.h"
#include "ImageBufferDownload.h"
#include "Item.h"
#include "ItemRecord.h"
#include "ItemPotion.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "RenderBlocks.h"
#include "RenderEngine.h"
#include "RenderHelper.h"
#if !PLATFORM_PS2
#include "RenderList.h"
#endif
#include "RenderManager.h"
#include "RenderSorter.h"
#include "EntitySorter.h"
#include "SoundManager.h"
#include "StepSound.h"
#include "Tessellator.h"

#include "TileEntity.h"
#include "TileEntityRenderer.h"
#include "Vec3D.h"
#include "World.h"
#include "WorldProvider.h"
#include "WorldHeight.h"

#include "WorldRenderer.h"
#if PLATFORM_PS2
#include "java/System.h"
#endif
#include "platform/PlatformCompat.h"
#include "platform/Profiler.h"

namespace
{
inline void applyPs2LegacyAtmosphereRgb(Minecraft *mc, float &red, float &green, float &blue)
{
#if PLATFORM_PS2
	if (mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyLook)
		legacyLookRgb(red, green, blue);
#else
	(void)mc;
	(void)red;
	(void)green;
	(void)blue;
#endif
}
}

RenderGlobal::RenderGlobal(Minecraft *minecraft, RenderEngine *renderengine)
{
#if PLATFORM_PS2
	MC_LOG_INFO("ps2", "RenderGlobal: constructor entered\n");
#endif
	tileEntities = std::vector<TileEntity *>();
	worldRenderersToUpdate = std::vector<WorldRenderer *>();
#if PLATFORM_PC
	occlusionEnabled = false;
#endif
	cloudOffsetX = 0;
	renderDistance = -1;
	renderEntitiesStartupCounter = 2;
#if PLATFORM_PS2
	MC_LOG_INFO("ps2", "RenderGlobal: constructor buffers ready\n");
#endif
#if PLATFORM_PC
	occlusionResult = std::vector<int_t>(64);
#endif
	renderBatchRenderers = std::vector<WorldRenderer *>();
	prevSortX = -9999.0;
	prevSortY = -9999.0;
	prevSortZ = -9999.0;
	prevReposX = -9999.0;
	prevReposY = -9999.0;
	prevReposZ = -9999.0;
	lastRendererMoveTimeMs = PlatformCompat::getMonotonicMicros() / 1000ULL;
	frustrumCheckOffset = 0;
	mc = minecraft;
	renderEngine = renderengine;

#if PLATFORM_PC
	occlusionEnabled = !PLATFORM_PC_LEGACY && renderSupportsFeature(RenderFeature::OcclusionQuery);
	// Desktop 1.2.5 retains three GL lists per WorldRenderer (two terrain passes
	// plus the occlusion box). Legacy PC uses a fixed low-end grid, so reserve only
	// the namespace that grid can address instead of the desktop maximum.
#if PLATFORM_PC_LEGACY
	constexpr int_t maxWorldRenderers = PLATFORM_VISIBLE_CHUNK_DIAMETER * PLATFORM_VERTICAL_CHUNK_COUNT * PLATFORM_VISIBLE_CHUNK_DIAMETER;
#else
	constexpr int_t maxChunksWide = 400 / 16 + 1;
	constexpr int_t maxChunksTall = WorldHeight::SECTION_COUNT;
	constexpr int_t maxWorldRenderers = maxChunksWide * maxChunksTall * maxChunksWide;
#endif
	glRenderListBase = GLAllocation::generateDisplayLists(maxWorldRenderers * 3);
	if (occlusionEnabled)
	{
		glOcclusionQueryBase = std::vector<int_t>(maxWorldRenderers);
		renderGenerateOcclusionQueries((int)glOcclusionQueryBase.size(), glOcclusionQueryBase.data());
	}
#endif

#if PLATFORM_PS2
	MC_LOG_INFO("ps2", "RenderGlobal: allocating sky meshes\n");
#endif
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	renderStaticMeshCreate(starMesh);
	renderStaticMeshCreate(skyMesh);
	renderStaticMeshCreate(skyMesh2);

#if !PLATFORM_PS2 || PS2_ENABLE_SKY_STARS
	renderStars();
#endif

	Tessellator *tessellator = &Tessellator::instance;
	const byte_t byte1 = 64;
	const int_t i = 256 / byte1 + 2;
	float f = 16.0f;

	tessellator->startDrawingQuads();
	for (int_t j = -byte1 * i; j <= byte1 * i; j += byte1)
	{
		for (int_t l = -byte1 * i; l <= byte1 * i; l += byte1)
		{
			tessellator->addVertex(j + 0, f, l + 0);
			tessellator->addVertex(j + byte1, f, l + 0);
			tessellator->addVertex(j + byte1, f, l + byte1);
			tessellator->addVertex(j + 0, f, l + byte1);
		}
	}
	tessellator->finishStaticMesh(skyMesh);

	f = -16.0f;
	tessellator->startDrawingQuads();
	for (int_t k = -byte1 * i; k <= byte1 * i; k += byte1)
	{
		for (int_t l = -byte1 * i; l <= byte1 * i; l += byte1)
		{
			tessellator->addVertex(k + byte1, f, l + 0);
			tessellator->addVertex(k + 0, f, l + 0);
			tessellator->addVertex(k + 0, f, l + byte1);
			tessellator->addVertex(k + byte1, f, l + byte1);
		}
	}
	tessellator->finishStaticMesh(skyMesh2);

#if PLATFORM_PS2
	MC_LOG_INFO("ps2", "RenderGlobal: sky meshes %d+%d+%d verts, %d KB\n",
		(int)starMesh.captured.vertexCount, (int)skyMesh.captured.vertexCount,
		(int)skyMesh2.captured.vertexCount,
		(int)((starMesh.captured.byteSize() + skyMesh.captured.byteSize() + skyMesh2.captured.byteSize()) / 1024));
#endif
#else
	starGLCallList = GLAllocation::generateDisplayLists(3);
	glSkyList = starGLCallList + 1;
	glSkyList2 = starGLCallList + 2;

	renderPushMatrix();
	renderBeginDisplayList(starGLCallList);
	renderStars();
	renderEndDisplayList();
	renderPopMatrix();

	Tessellator *tessellator = &Tessellator::instance;
	renderBeginDisplayList(glSkyList);

	byte_t byte1 = 64;
	int_t i = 256 / byte1 + 2;
	float f = 16.0f;

	for (int_t j = -byte1 * i; j <= byte1 * i; j += byte1)
	{
		for (int_t l = -byte1 * i; l <= byte1 * i; l += byte1)
		{
			tessellator->startDrawingQuads();
			tessellator->addVertex(j + 0, f, l + 0);
			tessellator->addVertex(j + byte1, f, l + 0);
			tessellator->addVertex(j + byte1, f, l + byte1);
			tessellator->addVertex(j + 0, f, l + byte1);
			tessellator->draw();
		}
	}

	renderEndDisplayList();

	renderBeginDisplayList(glSkyList2);
	f = -16.0f;
	tessellator->startDrawingQuads();

	for (int_t k = -byte1 * i; k <= byte1 * i; k += byte1)
	{
		for (int_t l = -byte1 * i; l <= byte1 * i; l += byte1)
		{
			tessellator->addVertex(k + byte1, f, l + 0);
			tessellator->addVertex(k + 0, f, l + 0);
			tessellator->addVertex(k + 0, f, l + byte1);
			tessellator->addVertex(k + byte1, f, l + byte1);
		}
	}

	tessellator->draw();
	renderEndDisplayList();
#endif

#if !PLATFORM_PS2
	// Desktop batches GL lists; Wii batches native GX renderer handles. PS2 draws
	// visible terrain sections directly and must not allocate these 64K-ID lists.
	for (int_t i = 0; i < 4; i++)
		allRenderLists[i] = new RenderList();
#endif
#if PLATFORM_PS2
	MC_LOG_INFO("ps2", "RenderGlobal: native terrain batching ready\n");
#endif
}

RenderGlobal::~RenderGlobal()
{
	changeWorld(nullptr);
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	renderStaticMeshDestroy(starMesh);
	renderStaticMeshDestroy(skyMesh);
	renderStaticMeshDestroy(skyMesh2);
#endif
#if !PLATFORM_PS2
	for (RenderList *&renderList : allRenderLists)
	{
		delete renderList;
		renderList = nullptr;
	}
#endif
}

void RenderGlobal::renderStars()
{
	if (!Config::isStarsEnabled()) // OptiFine: Stars OFF
		return;

	Random random(10842LL);
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();

	for (int_t i = 0; i < 1500; i++)
	{
		tess_coord_t d = random.nextFloat() * 2.0f - 1.0f;
		tess_coord_t d1 = random.nextFloat() * 2.0f - 1.0f;
		tess_coord_t d2 = random.nextFloat() * 2.0f - 1.0f;
		tess_coord_t d3 = 0.25f + random.nextFloat() * 0.25f;
#if PLATFORM_FLOAT_VERTEX_MATH
		const float lengthSq = static_cast<float>(d) * static_cast<float>(d) +
		                       static_cast<float>(d1) * static_cast<float>(d1) +
		                       static_cast<float>(d2) * static_cast<float>(d2);

		if (lengthSq >= 1.0f || lengthSq <= 0.01f)
			continue;

		const float invLength = 1.0f / std::sqrt(lengthSq);
		d = static_cast<tess_coord_t>(static_cast<float>(d) * invLength);
		d1 = static_cast<tess_coord_t>(static_cast<float>(d1) * invLength);
		d2 = static_cast<tess_coord_t>(static_cast<float>(d2) * invLength);
#else
		const double lengthSq = static_cast<double>(d) * static_cast<double>(d) +
		                        static_cast<double>(d1) * static_cast<double>(d1) +
		                        static_cast<double>(d2) * static_cast<double>(d2);

		if (lengthSq >= 1.0 || lengthSq <= 0.01)
			continue;

		const double invLength = 1.0 / JavaMath::sqrt(lengthSq);
		d = static_cast<tess_coord_t>(static_cast<double>(d) * invLength);
		d1 = static_cast<tess_coord_t>(static_cast<double>(d1) * invLength);
		d2 = static_cast<tess_coord_t>(static_cast<double>(d2) * invLength);
#endif

		tess_coord_t d5 = d * static_cast<tess_coord_t>(100.0);
		tess_coord_t d6 = d1 * static_cast<tess_coord_t>(100.0);
		tess_coord_t d7 = d2 * static_cast<tess_coord_t>(100.0);

#if PLATFORM_FLOAT_VERTEX_MATH
		const float d8 = std::atan2(static_cast<float>(d), static_cast<float>(d2));
		tess_coord_t d9 = static_cast<tess_coord_t>(std::sin(d8));
		tess_coord_t d10 = static_cast<tess_coord_t>(std::cos(d8));
		const float d11 = std::atan2(std::sqrt(static_cast<float>(d * d + d2 * d2)), static_cast<float>(d1));
		tess_coord_t d12 = static_cast<tess_coord_t>(std::sin(d11));
		tess_coord_t d13 = static_cast<tess_coord_t>(std::cos(d11));
		const float d14 = random.nextDoubleFloat() * 3.1415926535897931f * 2.0f;
		tess_coord_t d15 = static_cast<tess_coord_t>(std::sin(d14));
		tess_coord_t d16 = static_cast<tess_coord_t>(std::cos(d14));
#else
		double d8 = JavaMath::atan2(d, d2);
		tess_coord_t d9 = static_cast<tess_coord_t>(JavaMath::sin(d8));
		tess_coord_t d10 = static_cast<tess_coord_t>(JavaMath::cos(d8));
		double d11 = JavaMath::atan2(JavaMath::sqrt(d * d + d2 * d2), d1);
		tess_coord_t d12 = static_cast<tess_coord_t>(JavaMath::sin(d11));
		tess_coord_t d13 = static_cast<tess_coord_t>(JavaMath::cos(d11));
		double d14 = random.nextDouble() * 3.1415926535897931 * 2.0;
		tess_coord_t d15 = static_cast<tess_coord_t>(JavaMath::sin(d14));
		tess_coord_t d16 = static_cast<tess_coord_t>(JavaMath::cos(d14));
#endif

		for (int_t j = 0; j < 4; j++)
		{
			tess_coord_t d17 = 0.0;
			tess_coord_t d18 = static_cast<tess_coord_t>((j & 2) - 1) * d3;
			tess_coord_t d19 = static_cast<tess_coord_t>((j + 1 & 2) - 1) * d3;
			tess_coord_t d20 = d17;
			tess_coord_t d21 = d18 * d16 - d19 * d15;
			tess_coord_t d22 = d19 * d16 + d18 * d15;
			tess_coord_t d23 = d22;
			tess_coord_t d24 = d21 * d12 + d20 * d13;
			tess_coord_t d25 = d20 * d12 - d21 * d13;
			tess_coord_t d26 = d25 * d9 - d23 * d10;
			tess_coord_t d27 = d24;
			tess_coord_t d28 = d23 * d9 + d25 * d10;

			tessellator->addVertex(d5 + d26, d6 + d27, d7 + d28);
		}
	}

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	tessellator->finishStaticMesh(starMesh);
#else
	tessellator->draw();
#endif
}

void RenderGlobal::changeWorld(World *world)
{
	if (worldObj != nullptr)
		worldObj->removeWorldAccess(this);

	prevSortX = -9999.0;
	prevSortY = -9999.0;
	prevSortZ = -9999.0;

	if (RenderManager::instance != nullptr)
		RenderManager::instance->setWorld(world);
	clearWorldRenderers();
	delete globalRenderBlocks;
	globalRenderBlocks = nullptr;
	worldObj = world;

	if (world != nullptr)
	{
		globalRenderBlocks = new RenderBlocks(world);
		world->addWorldAccess(this);
		loadRenderers();
	}
}

void RenderGlobal::loadRenderers()
{
	Block::leaves->setGraphicsLevel(Config::isTreesFancy());
	renderDistance = mc->gameSettings->renderDistance;
	worldRenderersToUpdate.clear();
#if (PLATFORM_PS2 || PLATFORM_WII) && PLATFORM_CENTER_VERTICAL_RENDERERS
	verticalWindowInitialized = false;
#endif
#if !defined(PS2_PLATFORM)
	worldRenderersQueuedForUpdate.clear();
#endif
	tileEntities.clear();

	if (worldRenderers != nullptr)
	{
		for (int_t i = 0; i < renderChunksWide * renderChunksTall * renderChunksDeep; i++)
		{
			if (worldRenderers[i] != nullptr)
				delete worldRenderers[i];
		}
		delete[] worldRenderers;
		delete[] sortedWorldRenderers;
	}

#if PLATFORM_CONSOLE_LOW || PLATFORM_PC_LEGACY
	// Fixed low-end profile: use a small horizontal/vertical renderer grid.
	// Do not derive this from the normal desktop render-distance table.
	renderChunksWide = PLATFORM_VISIBLE_CHUNK_DIAMETER;
	renderChunksTall = PLATFORM_VERTICAL_CHUNK_COUNT;
	renderChunksDeep = PLATFORM_VISIBLE_CHUNK_DIAMETER;
#else
	int_t j = 2 * Config::getRenderDistanceFine();
	if (Config::isLoadChunksFar() && j < 512)
		j = 512;
	j += Config::getPreloadedChunks() * 2 * 16;
	const int_t rendererGridLimit = Config::getRenderDistanceFine() > 256 ? 1024 : 400;
	if (j > rendererGridLimit)
		j = rendererGridLimit;

	prevReposX = -9999.0;
	prevReposY = -9999.0;
	prevReposZ = -9999.0;
	renderChunksWide = j / 16 + 1;
#if PLATFORM_CENTER_VERTICAL_RENDERERS
	// The moving vertical window is not part of the low-console profile. A
	// platform can derive its horizontal grid from the render-distance table
	// above and still refuse to hold a renderer slot for every section of the
	// world column; markRenderersForNewPosition() centers the window on the
	// player either way.
	renderChunksTall = PLATFORM_VERTICAL_CHUNK_COUNT;
#else
	renderChunksTall = WorldHeight::SECTION_COUNT;
#endif
	renderChunksDeep = j / 16 + 1;
#endif

	int_t totalRenderers = JavaArithmetic::intMul(JavaArithmetic::intMul(renderChunksWide, renderChunksTall), renderChunksDeep);
	if (totalRenderers <= 0)
		throw std::length_error("Invalid renderer grid size");
	worldRenderersToUpdate.reserve(totalRenderers);
#if !defined(PS2_PLATFORM)
	worldRenderersQueuedForUpdate.reserve(totalRenderers);
#endif
	rendererUpdateCandidates.clear();
	rendererUpdateCandidates.reserve(totalRenderers);
	worldRenderers = new WorldRenderer *[totalRenderers]();
	sortedWorldRenderers = new WorldRenderer *[totalRenderers]();

#if PLATFORM_PC
	int_t k = 0;
#endif
	int_t l = 0;

	minBlockX = 0;
	minBlockY = 0;
	minBlockZ = 0;
	maxBlockX = renderChunksWide;
	maxBlockY = renderChunksTall;
	maxBlockZ = renderChunksDeep;

	for (int_t j1 = 0; j1 < renderChunksWide; j1++)
	{
		for (int_t k1 = 0; k1 < renderChunksTall; k1++)
		{
			for (int_t l1 = 0; l1 < renderChunksDeep; l1++)
			{
				int_t index = (l1 * renderChunksTall + k1) * renderChunksWide + j1;
#if PLATFORM_PC
				const int_t rendererListId = glRenderListBase + k;
#else
				const int_t rendererListId = 0;
#endif
				worldRenderers[index] = new WorldRenderer(worldObj, &tileEntities, j1 * 16, k1 * 16, l1 * 16, 16, rendererListId);

#if PLATFORM_PC
				if (occlusionEnabled)
					worldRenderers[index]->glOcclusionQuery = glOcclusionQueryBase[l];
				worldRenderers[index]->isWaitingOnOcclusionQuery = false;
#endif
				worldRenderers[index]->isVisible = true;
				worldRenderers[index]->isInFrustum = true;
				worldRenderers[index]->chunkIndex = l++;
				worldRenderers[index]->markDirty();
				sortedWorldRenderers[index] = worldRenderers[index];
				enqueueRendererUpdate(worldRenderers[index]);
#if PLATFORM_PC
				k += 3;
#endif
			}
		}
	}

	if (worldObj != nullptr)
	{
		EntityLiving *entityliving = mc->renderViewEntity;
		if (entityliving != nullptr)
		{
			markRenderersForNewPosition(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));

			// Java Beta 1.7.3 does Arrays.sort(sortedWorldRenderers, new EntitySorter(entityliving))
			// here. Advanced OpenGL relies on the near-to-far order before the first
			// occlusion-query pass; without this initial sort, chunks can be queried
			// before closer occluders are in the depth buffer and disappear incorrectly.
			std::sort(sortedWorldRenderers, sortedWorldRenderers + totalRenderers, EntitySorter(entityliving));
		}
	}

	renderEntitiesStartupCounter = 2;
}


void RenderGlobal::setAllRenderersVisible()
{
	if (worldRenderers == nullptr)
		return;
	const int_t count = renderChunksWide * renderChunksTall * renderChunksDeep;
	for (int_t i = 0; i < count; ++i)
	{
		WorldRenderer *renderer = worldRenderers[i];
		if (renderer != nullptr)
		{
			renderer->isVisible = true;
#if PLATFORM_PC
			renderer->isWaitingOnOcclusionQuery = false;
#endif
		}
	}
}

void RenderGlobal::renderEntities(Vec3D *vec3d, ICamera *icamera, float f)
{
	if (renderEntitiesStartupCounter > 0)
	{
		renderEntitiesStartupCounter--;
		return;
	}

	TileEntityRenderer::instance.cacheActiveRenderInfo(worldObj, renderEngine, mc->fontRenderer, mc->renderViewEntity, f);
	RenderManager::instance->cacheActiveRenderInfo(worldObj, renderEngine, mc->fontRenderer, mc->renderViewEntity, mc->gameSettings, f);

	countEntitiesTotal = 0;
	countEntitiesRendered = 0;
	countEntitiesHidden = 0;

	EntityLiving *entityliving = mc->renderViewEntity;

	RenderManager::renderPosX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
	RenderManager::renderPosY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
	RenderManager::renderPosZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;

	TileEntityRenderer::staticPlayerX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
	TileEntityRenderer::staticPlayerY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
	TileEntityRenderer::staticPlayerZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;

#if PLATFORM_PC
	// Vanilla 1.2.5 keeps the lightmap active for the complete entity pass.
	// Entity meshes use the current secondary texture coordinate rather than
	// carrying a lightmap UV per vertex, so the stage must be active before
	// RenderManager starts updating those coordinates.
	if (mc != nullptr && mc->entityRenderer != nullptr)
		mc->entityRenderer->enableLightmap(static_cast<double>(f));
#endif

	std::vector<Entity *> &list = worldObj->getLoadedEntityList();
	countEntitiesTotal = (int_t)list.size();
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
	platformProfileEntityFrame(countEntitiesTotal);
#endif

	for (int_t i = 0; i < (int_t)worldObj->weatherEffects.size(); i++)
	{
		Entity *entity = worldObj->weatherEffects[i];
		if (entity == nullptr || entity->isDead)
			continue;
		countEntitiesRendered++;
		if (entity->isInRangeToRenderVec3D(vec3d))
		{
#if PLATFORM_PROFILE_RENDER_PHASES
			const std::uint32_t cycWeatherDraw = platformProfileRenderPhaseBegin();
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			const std::uint32_t prof3WeatherDrawStart = platformProfileRenderPhaseBegin();
			const PlatformDrawSnapshot weatherDrawStart = platformProfileDrawSnapshot();
#endif
			RenderManager::instance->renderEntity(entity, f);
#if PLATFORM_PROFILE_RENDER_PHASES
			platformProfileRenderPhaseEnd(cycWeatherDraw, PlatformRenderPhase::EntityDraw);
			platformProfileEntityDraw(cycWeatherDraw, entity);
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityDrawDetailed(prof3WeatherDrawStart, weatherDrawStart, entity);
#endif
		}
	}

#if PLATFORM_PS2 && MC_LOG_LEVEL >= 2
	static unsigned int entityDiagnosticFrame = 0;
	const bool sampleEntities = worldObj->multiplayerWorld && (++entityDiagnosticFrame % 120u == 0);
	auto reportEntity = [&](Entity *entity, const char *reason) {
		if (sampleEntities)
			MC_LOG_DEBUG("net.entity.render", "id=%d reason=%s attached=%d pos=%.1f,%.1f,%.1f\n",
				entity->entityId, reason, entity->addedToChunk, entity->posX, entity->posY, entity->posZ);
	};
#else
	auto reportEntity = [](Entity *, const char *) {};
#endif
	for (int_t j = 0; j < (int_t)list.size(); j++)
	{
		Entity *entity1 = list[j];
		// Never dereference a null slot or a dead entity: a dead/removed entity
		// can briefly remain in this list, and rendering it virtual-calls into
		// (possibly freed) memory -> "jump to unaligned address" crash.
		if (entity1 == nullptr || entity1->isDead)
			continue;
#if PLATFORM_SKIP_WORLD_ENTITIES
		if (entity1 != mc->renderViewEntity)
			continue;
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
		platformProfileEntityCandidate();
		if (entity1->ignoreFrustumCheck)
			platformProfileEntitySpecialBypass();
#endif
#if PLATFORM_LIMIT_ENTITY_RENDER_DISTANCE
		if (entity1 != mc->renderViewEntity && !entity1->ignoreFrustumCheck && dynamic_cast<EntityLiving *>(entity1) != nullptr)
		{
			const float dx = static_cast<float>(entity1->posX - vec3d->xCoord);
			const float dz = static_cast<float>(entity1->posZ - vec3d->zCoord);
			const float radius = PLATFORM_ENTITY_RENDER_RADIUS_BLOCKS;
			if (dx * dx + dz * dz > radius * radius)
			{
	#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityCull(PlatformEntityCullReason::Distance);
#endif
			reportEntity(entity1, "distance");
				continue;
			}
		}
#endif
		if (!entity1->isInRangeToRenderVec3D(vec3d))
		{
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityCull(PlatformEntityCullReason::Range);
#endif
			reportEntity(entity1, "range");
			continue;
		}
		if (!entity1->ignoreFrustumCheck && !icamera->isBoundingBoxInFrustum(entity1->boundingBox))
		{
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityCull(PlatformEntityCullReason::Frustum);
#endif
			reportEntity(entity1, "frustum");
			continue;
		}
		if (entity1 == mc->renderViewEntity && !mc->gameSettings->thirdPersonView && !mc->renderViewEntity->isPlayerSleeping())
		{
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityCull(PlatformEntityCullReason::SelfHidden);
#endif
			reportEntity(entity1, "self");
			continue;
		}

		int_t l = MathHelper::floor_double(entity1->posY);
		if (l < 0)
			l = 0;
		if (l >= WorldHeight::HEIGHT)
			l = WorldHeight::MAX_Y;

#if PLATFORM_SKIP_WORLD_ENTITIES
		const bool forceRenderViewEntity = entity1 == mc->renderViewEntity && mc->gameSettings->thirdPersonView;
#else
		const bool forceRenderViewEntity = false;
#endif
		if (forceRenderViewEntity || worldObj->blockExists(MathHelper::floor_double(entity1->posX), l, MathHelper::floor_double(entity1->posZ)))
		{
			countEntitiesRendered++;
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityRendered();
#endif
#if PLATFORM_PROFILE_RENDER_PHASES
			const std::uint32_t cycEntDraw = platformProfileRenderPhaseBegin();
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			const std::uint32_t prof3EntityDrawStart = platformProfileRenderPhaseBegin();
			const PlatformDrawSnapshot entityDrawStart = platformProfileDrawSnapshot();
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL >= 2
			if (sampleEntities)
			{
				reportEntity(entity1, RenderManager::instance->getEntityRenderObject(entity1) != nullptr ? "submit" : "no-renderer");
				MC_LOG_DEBUG("net.entity.render", "id=%d gs-used=%ld capacity=%ld overflows=%ld\n",
					entity1->entityId, ps2_gs_queue_used_bytes(), ps2_gs_queue_capacity_bytes(), ps2_gs_queue_overflow_count());
			}
#endif
			RenderManager::instance->renderEntity(entity1, f);
#if PLATFORM_PROFILE_RENDER_PHASES
			platformProfileRenderPhaseEnd(cycEntDraw, PlatformRenderPhase::EntityDraw);
			platformProfileEntityDraw(cycEntDraw, entity1);
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityDrawDetailed(prof3EntityDrawStart, entityDrawStart, entity1);
#endif
		}
		else
		{
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileEntityCull(PlatformEntityCullReason::MissingChunk);
#endif
			reportEntity(entity1, "missing-chunk");
		}
	}

	RenderHelper::enableStandardItemLighting();

#if PLATFORM_PROFILE_RENDER_PHASES
	const std::uint32_t cycTileDraw = platformProfileRenderPhaseBegin();
#endif
	for (size_t k = 0; k < tileEntities.size(); )
	{
		TileEntity *tileEntity = tileEntities[k];
		if (tileEntity == nullptr || tileEntity->isInvalid() || worldObj == nullptr ||
			worldObj->getBlockTileEntity(tileEntity->xCoord, tileEntity->yCoord, tileEntity->zCoord) != tileEntity)
		{
			tileEntities.erase(tileEntities.begin() + k);
			continue;
		}

		TileEntityRenderer::instance.renderTileEntity(tileEntity, f);
		++k;
	}
#if PLATFORM_PROFILE_RENDER_PHASES
	platformProfileRenderPhaseEnd(cycTileDraw, PlatformRenderPhase::TileEntityDraw);
#endif

#if PLATFORM_PC
	if (mc != nullptr && mc->entityRenderer != nullptr)
		mc->entityRenderer->disableLightmap(static_cast<double>(f));
#endif
}

void RenderGlobal::markAllRenderersDirty()
{
	if (worldRenderers == nullptr)
		return;

	const int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
	for (int_t i = 0; i < total; ++i)
	{
		WorldRenderer *renderer = worldRenderers[i];
		if (renderer == nullptr)
			continue;

		// Keep the current renderer grid and invalidate only its compiled contents.
		// Console staging builds restart here; desktop display lists are replaced by
		// the normal renderer update path without discarding the entire grid first.
		renderer->markDirty();
		enqueueRendererUpdate(renderer);
	}
}

jstring RenderGlobal::getDebugInfoRenders()
{
	return "C: " + std::to_string(renderersBeingRendered) + "/" + std::to_string(renderersLoaded) + ". F: " + std::to_string(renderersBeingClipped) + ", O: " + std::to_string(renderersBeingOccluded) + ", E: " + std::to_string(renderersSkippingRenderPass);
}

jstring RenderGlobal::getDebugInfoEntities()
{
	return "E: " + std::to_string(countEntitiesRendered) + "/" + std::to_string(countEntitiesTotal) + ". B: " + std::to_string(countEntitiesHidden) + ", I: " + std::to_string(countEntitiesTotal - countEntitiesHidden - countEntitiesRendered);
}

#if PLATFORM_CENTER_VERTICAL_RENDERERS
#if PLATFORM_PS2 || PLATFORM_WII
int_t RenderGlobal::chooseConsoleVerticalStartSection(int_t playerBlockY) const
{
	const int_t maxStartSection = std::max(0, WorldHeight::SECTION_COUNT - renderChunksTall);
	const int_t playerSection = JavaArithmetic::intShr(playerBlockY, 4);
	const int_t belowBias = std::min(renderChunksTall - 1, renderChunksTall / 2 + 1);
	const int_t preferredStart = std::max(0, std::min(playerSection - belowBias, maxStartSection));

	if (!verticalWindowInitialized)
		return preferredStart;

	const int_t currentTopSection = verticalStartSection + renderChunksTall - 1;
	if (playerSection > currentTopSection + 1 || playerSection < verticalStartSection - 1)
		return preferredStart;

	if (playerSection > currentTopSection)
		return std::min(verticalStartSection + 1, maxStartSection);

	if (playerSection < verticalStartSection + 1)
		return std::max(verticalStartSection - 1, 0);

	return verticalStartSection;
}
#endif

void RenderGlobal::remapCenteredVerticalRendererSlots(int_t newStartSection)
{
	const int_t delta = newStartSection - verticalStartSection;
	if (delta == 0 || worldRenderers == nullptr || renderChunksTall <= 1)
		return;

	const int_t horizontalPlane = renderChunksWide * renderChunksDeep;
	const int_t movedLayers = std::min(std::abs(delta), renderChunksTall);
	const int_t recycled = movedLayers * horizontalPlane;
	const int_t reused = (renderChunksTall - movedLayers) * horizontalPlane;

	if (delta > -renderChunksTall && delta < renderChunksTall)
	{
		std::vector<WorldRenderer *> oldColumn(static_cast<std::size_t>(renderChunksTall));
		for (int_t zSlot = 0; zSlot < renderChunksDeep; ++zSlot)
		{
			for (int_t xSlot = 0; xSlot < renderChunksWide; ++xSlot)
			{
				for (int_t oldSlot = 0; oldSlot < renderChunksTall; ++oldSlot)
				{
					const int_t index = (zSlot * renderChunksTall + oldSlot) * renderChunksWide + xSlot;
					oldColumn[static_cast<std::size_t>(oldSlot)] = worldRenderers[index];
				}

				for (int_t newSlot = 0; newSlot < renderChunksTall; ++newSlot)
				{
					int_t sourceSlot = (newSlot + delta) % renderChunksTall;
					if (sourceSlot < 0)
						sourceSlot += renderChunksTall;
					const int_t index = (zSlot * renderChunksTall + newSlot) * renderChunksWide + xSlot;
					worldRenderers[index] = oldColumn[static_cast<std::size_t>(sourceSlot)];
				}
			}
		}
	}

#if (PLATFORM_PS2 || PLATFORM_WII) && MC_LOG_LEVEL > 2
	MC_LOG_DEBUG("render", "vertical shift reused=%d recycled=%d start=%d->%d\n",
		(int)reused, (int)recycled, (int)verticalStartSection, (int)newStartSection);
#endif
}
#endif

void RenderGlobal::markRenderersForNewPosition(int_t i, int_t j, int_t k)
{
	i -= 8;
	j -= 8;
	k -= 8;

	minBlockX = 0x7fffffff;
	minBlockY = 0x7fffffff;
	minBlockZ = 0x7fffffff;
	maxBlockX = 0x80000000;
	maxBlockY = 0x80000000;
	maxBlockZ = 0x80000000;

	int_t l = renderChunksWide * 16;
	int_t i1 = l / 2;

#if PLATFORM_CENTER_VERTICAL_RENDERERS
	// Keep the renderer objects that still represent the same world sections.
	// Console windows favour terrain below the player and move only when the
	// player reaches a guard edge, avoiding section-boundary oscillation.
	int_t startSection = 0;
#if PLATFORM_PS2 || PLATFORM_WII
	startSection = chooseConsoleVerticalStartSection(j + 8);
	if (verticalWindowInitialized)
		remapCenteredVerticalRendererSlots(startSection);
#else
	int_t centerSection = JavaArithmetic::intShr(j + 8, 4);
	startSection = centerSection - (renderChunksTall / 2);
	if (startSection < 0)
		startSection = 0;
	if (startSection > WorldHeight::SECTION_COUNT - renderChunksTall)
		startSection = WorldHeight::SECTION_COUNT - renderChunksTall;
	remapCenteredVerticalRendererSlots(startSection);
#endif
	verticalStartSection = startSection;
#if PLATFORM_PS2 || PLATFORM_WII
	verticalWindowInitialized = true;
#endif
#endif

	for (int_t j1 = 0; j1 < renderChunksWide; j1++)
	{
		int_t k1 = j1 * 16;
		int_t l1 = (k1 + i1) - i;

		if (l1 < 0)
			l1 -= l - 1;

		l1 /= l;
		k1 -= l1 * l;

		if (k1 < minBlockX)
			minBlockX = k1;
		if (k1 > maxBlockX)
			maxBlockX = k1;

		for (int_t i2 = 0; i2 < renderChunksDeep; i2++)
		{
			int_t j2 = i2 * 16;
			int_t k2 = (j2 + i1) - k;

			if (k2 < 0)
				k2 -= l - 1;

			k2 /= l;
			j2 -= k2 * l;

			if (j2 < minBlockZ)
				minBlockZ = j2;
			if (j2 > maxBlockZ)
				maxBlockZ = j2;

			for (int_t l2 = 0; l2 < renderChunksTall; l2++)
			{
#if PLATFORM_CENTER_VERTICAL_RENDERERS
				int_t i3 = (verticalStartSection + l2) * 16;
#else
				int_t i3 = l2 * 16;
#endif

				if (i3 < minBlockY)
					minBlockY = i3;
				if (i3 > maxBlockY)
					maxBlockY = i3;

				WorldRenderer *worldrenderer = worldRenderers[(i2 * renderChunksTall + l2) * renderChunksWide + j1];
				bool flag = worldrenderer->needsUpdate;
				worldrenderer->setPosition(k1, i3, j2);

				if (!flag && worldrenderer->needsUpdate)
					enqueueRendererUpdate(worldrenderer);
			}
		}
	}
}

void RenderGlobal::enqueueRendererUpdate(WorldRenderer *worldrenderer)
{
#ifdef PS2_PLATFORM
	if (worldrenderer == nullptr || worldrenderer->queuedForUpdate)
		return;
	worldrenderer->queuedForUpdate = true;
	worldRenderersToUpdate.push_back(worldrenderer);
#else
	if (worldrenderer != nullptr && worldRenderersQueuedForUpdate.insert(worldrenderer).second)
		worldRenderersToUpdate.push_back(worldrenderer);
#endif
}

#if PLATFORM_PS2 || PLATFORM_WII
void RenderGlobal::enqueueRendererUpdatePriority(WorldRenderer *worldrenderer)
{
	if (worldrenderer == nullptr)
		return;

	// Block edits are tiny, but a console queue can already contain many
	// slow chunk-build jobs. Put edited sections first so breaking/placing a
	// block updates visually without waiting behind terrain streaming.
#ifdef PS2_PLATFORM
	if (!worldrenderer->queuedForUpdate)
	{
		worldrenderer->queuedForUpdate = true;
		worldRenderersToUpdate.insert(worldRenderersToUpdate.begin(), worldrenderer);
		return;
	}
#else
	auto it = worldRenderersQueuedForUpdate.find(worldrenderer);
	if (it == worldRenderersQueuedForUpdate.end())
	{
		worldRenderersQueuedForUpdate.insert(worldrenderer);
		worldRenderersToUpdate.insert(worldRenderersToUpdate.begin(), worldrenderer);
		return;
	}
#endif

	auto vit = std::find(worldRenderersToUpdate.begin(), worldRenderersToUpdate.end(), worldrenderer);
	if (vit != worldRenderersToUpdate.end() && vit != worldRenderersToUpdate.begin())
	{
		worldRenderersToUpdate.erase(vit);
		worldRenderersToUpdate.insert(worldRenderersToUpdate.begin(), worldrenderer);
	}
}
#endif

void RenderGlobal::dequeueRendererUpdate(WorldRenderer *worldrenderer)
{
	if (worldrenderer == nullptr)
		return;
#ifdef PS2_PLATFORM
	worldrenderer->queuedForUpdate = false;
#else
	worldRenderersQueuedForUpdate.erase(worldrenderer);
#endif
}

void RenderGlobal::compactRendererUpdateQueue()
{
	std::size_t writeIndex = 0;
	for (WorldRenderer *renderer : worldRenderersToUpdate)
	{
		if (renderer == nullptr)
			continue;
		if (!renderer->needsUpdate)
		{
			dequeueRendererUpdate(renderer);
			continue;
		}
		worldRenderersToUpdate[writeIndex++] = renderer;
	}
	worldRenderersToUpdate.resize(writeIndex);
}

bool RenderGlobal::isRendererUpdateMoving(EntityLiving *entityliving)
{
	if (entityliving == nullptr)
		return false;

	constexpr double maxDifference = 0.001;
	const bool movingNow = entityliving->isJumping || entityliving->isSneaking() ||
		std::fabs((double)entityliving->swingProgress) > maxDifference ||
		std::fabs((double)entityliving->moveStrafing) > maxDifference ||
		std::fabs((double)entityliving->moveForward) > maxDifference ||
		std::fabs(entityliving->posX - entityliving->prevPosX) > maxDifference ||
		std::fabs(entityliving->posY - entityliving->prevPosY) > maxDifference ||
		std::fabs(entityliving->posZ - entityliving->prevPosZ) > maxDifference ||
		std::fabs((double)(entityliving->rotationYaw - entityliving->prevRotationYaw)) > maxDifference ||
		std::fabs((double)(entityliving->rotationPitch - entityliving->prevRotationPitch)) > maxDifference;

	const uint64_t nowMs = PlatformCompat::getMonotonicMicros() / 1000ULL;
	if (movingNow)
	{
		lastRendererMoveTimeMs = nowMs;
		return true;
	}

	// Preserve C6's two-second grace period so a one-frame input pause does not
	// immediately triple chunk work while the player is still moving around.
	if (nowMs < lastRendererMoveTimeMs)
	{
		lastRendererMoveTimeMs = nowMs;
		return true;
	}
	return nowMs - lastRendererMoveTimeMs < 2000ULL;
}

bool RenderGlobal::isRendererUpdateActing(EntityLiving *entityliving) const
{
	if (entityliving == nullptr || !entityliving->isPlayer())
		return false;
	EntityPlayer *player = static_cast<EntityPlayer *>(entityliving);
	return player->isSwinging || player->isUsingItem();
}

int_t RenderGlobal::sortAndRender(EntityLiving *entityliving, int_t i, double d)
{
	for (int_t j = 0; j < 10; j++)
	{
		worldRenderersCheckIndex = (worldRenderersCheckIndex + 1) % (renderChunksWide * renderChunksTall * renderChunksDeep);
		WorldRenderer *worldrenderer = worldRenderers[worldRenderersCheckIndex];

		if (worldrenderer->needsUpdate)
			enqueueRendererUpdate(worldrenderer);
	}

	if (mc->gameSettings->renderDistance != renderDistance && !Config::isLoadChunksFar())
		loadRenderers();

	if (i == 0)
	{
		renderersLoaded = 0;
		renderersBeingClipped = 0;
		renderersBeingOccluded = 0;
		renderersBeingRendered = 0;
		renderersSkippingRenderPass = 0;
	}

#if PLATFORM_FLOAT_VERTEX_MATH
	using render_sort_real_t = float;
#else
	using render_sort_real_t = double;
#endif
	const render_sort_real_t partialTicks = static_cast<render_sort_real_t>(d);
	const render_sort_real_t d1 = static_cast<render_sort_real_t>(entityliving->lastTickPosX) +
		(static_cast<render_sort_real_t>(entityliving->posX) - static_cast<render_sort_real_t>(entityliving->lastTickPosX)) * partialTicks;
	const render_sort_real_t d2 = static_cast<render_sort_real_t>(entityliving->lastTickPosY) +
		(static_cast<render_sort_real_t>(entityliving->posY) - static_cast<render_sort_real_t>(entityliving->lastTickPosY)) * partialTicks;
	const render_sort_real_t d3 = static_cast<render_sort_real_t>(entityliving->lastTickPosZ) +
		(static_cast<render_sort_real_t>(entityliving->posZ) - static_cast<render_sort_real_t>(entityliving->lastTickPosZ)) * partialTicks;

	const render_sort_real_t d4 = static_cast<render_sort_real_t>(entityliving->posX - prevSortX);
	const render_sort_real_t d5 = static_cast<render_sort_real_t>(entityliving->posY - prevSortY);
	const render_sort_real_t d6 = static_cast<render_sort_real_t>(entityliving->posZ - prevSortZ);

	if (d4 * d4 + d5 * d5 + d6 * d6 > static_cast<render_sort_real_t>(16.0))
	{
		prevSortX = entityliving->posX;
		prevSortY = entityliving->posY;
		prevSortZ = entityliving->posZ;
#if PLATFORM_CONSOLE_LOW
		const int_t preloadedBlocks = 0;
#else
		const int_t preloadedBlocks = Config::getPreloadedChunks() * 16;
#endif
		const render_sort_real_t reposX = static_cast<render_sort_real_t>(entityliving->posX - prevReposX);
		const render_sort_real_t reposY = static_cast<render_sort_real_t>(entityliving->posY - prevReposY);
		const render_sort_real_t reposZ = static_cast<render_sort_real_t>(entityliving->posZ - prevReposZ);
		const render_sort_real_t distSqRepos = reposX * reposX + reposY * reposY + reposZ * reposZ;
		if (distSqRepos > static_cast<render_sort_real_t>(preloadedBlocks * preloadedBlocks + 16))
		{
			prevReposX = entityliving->posX;
			prevReposY = entityliving->posY;
			prevReposZ = entityliving->posZ;
			markRenderersForNewPosition(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));
		}

		// Sort renderers front-to-back. Required by Advanced OpenGL occlusion
		// culling: the nearest chunks must be rendered first so the depth buffer
		// holds the closest occluders before farther chunks' boxes are queried.
		int_t totalRenderers = renderChunksWide * renderChunksTall * renderChunksDeep;
		std::sort(sortedWorldRenderers, sortedWorldRenderers + totalRenderers, EntitySorter(entityliving));
	}

	// Keep the display-list cache below its high-water mark before the per-frame
	// mesher starts asking for new terrain buffers. This prevents a full cache
	// from turning an ordinary walk across chunk boundaries into missing terrain.
#ifdef WII_PLATFORM
	if (i == 0)
		evictWiiMeshCache(entityliving);
#endif

	RenderHelper::disableStandardItemLighting();
	if (i == 0 && Config::isSmoothFps())
		renderFinishGpu();

	int_t k = 0;

#if PLATFORM_PC_LEGACY
	if (i == 0)
		updatePcLegacySectionVisibility(entityliving);
#endif
#if PLATFORM_PS2
	if (i == 0)
		updatePs2SectionVisibility(entityliving);
#endif

#if PLATFORM_PC
	const bool useOcclusion = occlusionEnabled
		&& mc != nullptr
		&& mc->gameSettings != nullptr
		&& mc->gameSettings->advancedOpengl
		&& !mc->gameSettings->anaglyph;

	if (useOcclusion && i == 0)
	{
		const int_t totalRenderers = renderChunksWide * renderChunksTall * renderChunksDeep;
		int_t firstIndex = 0;
		int_t endIndex = std::min(20, totalRenderers);
		checkOcclusionQueryResult(firstIndex, endIndex, entityliving->posX, entityliving->posY, entityliving->posZ);

		for (int_t index = firstIndex; index < endIndex; ++index)
			sortedWorldRenderers[index]->isVisible = true;

		k += renderSortedRenderers(firstIndex, endIndex, i, d);

		int_t occlusionStepIndex = 0;
		const int_t occlusionStep = 30;
		const int_t switchStep = std::max(1, renderChunksWide / 2);
		while (endIndex < totalRenderers)
		{
			const int_t startIndex = endIndex;
			if (occlusionStepIndex < switchStep)
				occlusionStepIndex++;
			else
				occlusionStepIndex--;

			endIndex = startIndex + occlusionStepIndex * occlusionStep;
			if (endIndex <= startIndex)
				endIndex = startIndex + 10;
			if (endIndex > totalRenderers)
				endIndex = totalRenderers;

			renderDisable(RenderCapability::Texture2D);
			renderDisable(RenderCapability::Lighting);
			renderDisable(RenderCapability::AlphaTest);
			renderDisable(RenderCapability::Fog);
			renderColorMask(false, false, false, false);
			renderDepthMask(false);

			checkOcclusionQueryResult(startIndex, endIndex, entityliving->posX, entityliving->posY, entityliving->posZ);

			renderPushMatrix();
			float accumulatedX = 0.0f;
			float accumulatedY = 0.0f;
			float accumulatedZ = 0.0f;

			for (int_t rendererIndex = startIndex; rendererIndex < endIndex; ++rendererIndex)
			{
				WorldRenderer *renderer = sortedWorldRenderers[rendererIndex];
				if (renderer->skipAllRenderPasses())
				{
					renderer->isInFrustum = false;
					continue;
				}

				if (!renderer->isInFrustum)
					continue;

				if (Config::isOcclusionFancy() && !renderer->isFullyInFrustum)
				{
					renderer->isVisible = true;
					continue;
				}

				if (renderer->isWaitingOnOcclusionQuery)
					continue;

				if (renderer->isVisibleFromPosition)
				{
					const float dx = std::fabs((float)(renderer->visibleFromX - entityliving->posX));
					const float dy = std::fabs((float)(renderer->visibleFromY - entityliving->posY));
					const float dz = std::fabs((float)(renderer->visibleFromZ - entityliving->posZ));
					const float distanceFromVisiblePosition = dx + dy + dz;
					if (distanceFromVisiblePosition < 10.0f + (float)rendererIndex / 1000.0f)
					{
						renderer->isVisible = true;
						continue;
					}
					renderer->isVisibleFromPosition = false;
				}

				const float boxX = (float)((double)renderer->posXMinus - d1);
				const float boxY = (float)((double)renderer->posYMinus - d2);
				const float boxZ = (float)((double)renderer->posZMinus - d3);
				const float translateX = boxX - accumulatedX;
				const float translateY = boxY - accumulatedY;
				const float translateZ = boxZ - accumulatedZ;

				if (translateX != 0.0f || translateY != 0.0f || translateZ != 0.0f)
				{
					renderTranslate(translateX, translateY, translateZ);
					accumulatedX += translateX;
					accumulatedY += translateY;
					accumulatedZ += translateZ;
				}

#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
				renderBeginOcclusionQuery(renderer->glOcclusionQuery);
				renderer->callOcclusionQueryList();
				renderEndOcclusionQuery();
				renderer->isWaitingOnOcclusionQuery = true;
#endif
			}

			renderPopMatrix();

			if (mc->gameSettings->anaglyph)
			{
				if (EntityRenderer::anaglyphField == 0)
					renderColorMask(false, true, true, true);
				else
					renderColorMask(true, false, false, true);
			}
			else
			{
				renderColorMask(true, true, true, true);
			}

			renderDepthMask(true);
			renderEnable(RenderCapability::Texture2D);
			renderEnable(RenderCapability::AlphaTest);
			renderEnable(RenderCapability::Fog);

			k += renderSortedRenderers(startIndex, endIndex, i, d);
		}
	}
	else
	{
		// When Advanced OpenGL is off, occlusion query results from previous
		// frames/settings must not hide chunks. occlusionEnabled only means the
		// driver supports ARB_occlusion_query; it does not mean the feature is
		// active in GameSettings.
		int_t totalRenderers = renderChunksWide * renderChunksTall * renderChunksDeep;
		if (!useOcclusion)
		{
			for (int_t r = 0; r < totalRenderers; ++r)
			{
				if (sortedWorldRenderers[r] != nullptr)
				{
					sortedWorldRenderers[r]->isVisible = true;
					sortedWorldRenderers[r]->isWaitingOnOcclusionQuery = false;
				}
			}
		}

		k += renderSortedRenderers(0, totalRenderers, i, d);
	}

#else
	const int_t totalRenderers = renderChunksWide * renderChunksTall * renderChunksDeep;
	k += renderSortedRenderers(0, totalRenderers, i, d);
#endif

	return k;
}

#if PLATFORM_PC
void RenderGlobal::checkOcclusionQueryResult(int_t i, int_t j, double playerX, double playerY, double playerZ)
{
	for (int_t k = i; k < j; k++)
	{
		WorldRenderer *renderer = sortedWorldRenderers[k];
		if (!renderer->isWaitingOnOcclusionQuery)
			continue;

		if (renderOcclusionQueryResultAvailable(renderer->glOcclusionQuery))
		{
			renderer->isWaitingOnOcclusionQuery = false;
			occlusionResult[0] = (int_t)renderOcclusionQueryResult(renderer->glOcclusionQuery);
			const bool wasVisible = renderer->isVisible;
			renderer->isVisible = occlusionResult[0] != 0;
			if (wasVisible && renderer->isVisible)
			{
				renderer->isVisibleFromPosition = true;
				renderer->visibleFromX = playerX;
				renderer->visibleFromY = playerY;
				renderer->visibleFromZ = playerZ;
			}
		}
	}
}
#endif

int_t RenderGlobal::renderSortedRenderers(int_t i, int_t j, int_t k, double d)
{
#if PLATFORM_SKIP_TRANSPARENT_WORLD_PASS
	// Low-console profile: skip alpha/water pass for now. The opaque terrain pass
	// is the useful one and pass 1 costs a lot of bandwidth/CPU time.
	if (k != 0)
		return 0;
#endif
	renderBatchRenderers.clear();

#if PLATFORM_PC
	const bool useOcclusion = occlusionEnabled
		&& mc != nullptr
		&& mc->gameSettings != nullptr
		&& mc->gameSettings->advancedOpengl
		&& !mc->gameSettings->anaglyph;
#else
	const bool useOcclusion = false;
#endif

	int_t l = 0;
	for (int_t i1 = i; i1 < j; i1++)
	{
		WorldRenderer *sortedRenderer = sortedWorldRenderers[i1];
#if PLATFORM_PC_LEGACY
		const bool cpuOccluded = !sortedRenderer->pcLegacyCpuVisible;
#elif PLATFORM_PS2
		const bool cpuOccluded = PLATFORM_CPU_SECTION_OCCLUSION && !sortedRenderer->ps2CpuVisible;
#else
		const bool cpuOccluded = false;
#endif
		if (k == 0)
		{
			renderersLoaded++;
			if (sortedRenderer->skipRenderPass(k))
			{
				renderersSkippingRenderPass++;
			}
			else if (!sortedRenderer->isInFrustum)
			{
				renderersBeingClipped++;
			}
			else if (cpuOccluded || (useOcclusion && !sortedRenderer->isVisible))
			{
				renderersBeingOccluded++;
			}
			else
			{
				renderersBeingRendered++;
			}
		}

		if (sortedRenderer->skipRenderPass(k) || !sortedRenderer->isInFrustum || cpuOccluded ||
			(useOcclusion && !sortedRenderer->isVisible))
			continue;

#if PLATFORM_PS2 || defined(WII_PLATFORM)
		// Native console terrain uses the renderer itself as the draw contract.
		// Visibility/pass checks above are sufficient; Wii resolves a GX handle
		// later when RenderList submits the batch.
		renderBatchRenderers.push_back(sortedRenderer);
		l++;
#else
		int_t j1 = sortedRenderer->getGLCallListForPass(k);
		if (j1 >= 0)
		{
			renderBatchRenderers.push_back(sortedRenderer);
			l++;
		}
#endif
	}

	EntityLiving *entityliving = mc->renderViewEntity;
	double d1 = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * d;
	double d2 = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * d;
	double d3 = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * d;

	// Publish the eye before any section is submitted. A backend that culls
	// terrain against it -- the Wii's face-direction cull -- must decide on the
	// same interpolated position the pass is drawn with, or geometry pops in and
	// out at the section boundary. No-op where nothing culls by eye.
	renderTerrainSetViewerPosition(d1, d2, d3);

#if PLATFORM_PS2
	// PS2: bypass desktop display-list batching and the GL matrix stack. Cached
	// section vertices are local to their WorldRenderer, so renderPassCached()
	// can build the full section-to-camera translation directly into the native
	// PS2 draw context. No per-section glPushMatrix/glTranslatef/glPopMatrix is
	// needed on the terrain hot path.
	//
	// The face bucket cull also needs this interpolated eye position. Publishing
	// it once keeps culling and the native transform on the exact same frame.
	WorldRenderer::setTerrainViewerPosition(d1, d2, d3);

	int_t renderedNow = 0;
	for (WorldRenderer *worldrenderer : renderBatchRenderers)
	{
		if (worldrenderer == nullptr)
			continue;
		if (renderedNow >= PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS)
			break;

		worldrenderer->renderPassImmediate(k);
		renderedNow++;
	}

#ifdef PS2_RENDER_STATS
	// Pass 1 (water/ice/glass) was never instrumented: every counter above is
	// gated on k == 0. Without it there is no way to tell "the section was never
	// submitted" (culled, no mesh yet, or dropped by the cap) from "it was
	// submitted and the GS discarded it" (depth test, blend, alpha) — which is
	// exactly the ambiguity behind water that comes and goes. The extra sweep
	// only exists in PS2_RENDER_STATS builds.
	static int s_ps2TerrainLogTick[2] = { 0, 0 };
	if (k >= 0 && k <= 1 && ++s_ps2TerrainLogTick[k] >= 120)
	{
		s_ps2TerrainLogTick[k] = 0;
		const int_t totalRenderers = renderChunksWide * renderChunksTall * renderChunksDeep;
		if (k == 0)
		{
			int_t pendingSkip = 0;
			int_t visiblePendingSkip = 0;
			for (int_t s = i; s < j; ++s)
			{
				WorldRenderer *renderer = sortedWorldRenderers[s];
				if (renderer == nullptr || !renderer->needsUpdate ||
					!renderer->skipRenderPass(0))
					continue;
				pendingSkip++;
				if (renderer->isInFrustum)
					visiblePendingSkip++;
			}
			MC_LOG_DEBUG("ps2", "terrain pass0 rendered=%d/%d total=%d loaded=%d skip=%d"
			       " dirtySkipped=%d visibleDirtySkipped=%d clip=%d occ=%d pending=%d\n",
			       (int)renderedNow, (int)PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS,
			       (int)totalRenderers,
			       (int)renderersLoaded, (int)renderersSkippingRenderPass,
			       (int)pendingSkip, (int)visiblePendingSkip,
			       (int)renderersBeingClipped, (int)renderersBeingOccluded,
			       (int)worldRenderersToUpdate.size());
		}
		else
		{
			// withGeom = sections that actually built transparent geometry;
			// culled  = of those, how many the frustum test rejected.
			int_t withGeom = 0, culled = 0;
			for (int_t s = i; s < j; s++)
			{
				if (sortedWorldRenderers[s]->skipRenderPass(k))
					continue;
				withGeom++;
				if (!sortedWorldRenderers[s]->isInFrustum)
					culled++;
			}
			MC_LOG_DEBUG("ps2", "terrain pass1 rendered=%d/%d total=%d withGeom=%d culled=%d listed=%d pending=%d\n",
			       (int)renderedNow, (int)PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS,
			       (int)totalRenderers, (int)withGeom, (int)culled, (int)l,
			       (int)worldRenderersToUpdate.size());
		}
	}
#endif

	return renderedNow;
#endif

#if !PLATFORM_PS2
	int_t k1 = 0;
	for (int_t l1 = 0; l1 < 4; l1++)
		allRenderLists[l1]->reset();

	for (size_t i2 = 0; i2 < renderBatchRenderers.size(); i2++)
	{
		WorldRenderer *worldrenderer = renderBatchRenderers[i2];
		int_t j2 = -1;

		for (int_t k2 = 0; k2 < k1; k2++)
		{
			if (allRenderLists[k2]->matchesPos(worldrenderer->posXMinus, worldrenderer->posYMinus, worldrenderer->posZMinus))
				j2 = k2;
		}

		if (j2 < 0)
		{
			j2 = k1++;
			allRenderLists[j2]->setup(worldrenderer->posXMinus, worldrenderer->posYMinus, worldrenderer->posZMinus, d1, d2, d3);
		}

		allRenderLists[j2]->addTerrainRenderer(worldrenderer, k);
	}

	renderAllRenderLists(k, d);
	return l;
#endif
}

void RenderGlobal::updateClouds()
{
	cloudOffsetX++;
}

void RenderGlobal::renderSky(float f)
{
	if (worldObj->worldProvider->worldType == 1)
	{
		renderDisable(RenderCapability::Fog);
		renderDisable(RenderCapability::AlphaTest);
		renderEnable(RenderCapability::Blend);
		renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
		RenderHelper::disableStandardItemLighting();
		renderDepthMask(false);
		renderBindTexture(renderEngine->getTexture("/misc/tunnel.png"));
		Tessellator *tessellator = &Tessellator::instance;
		float endSkyRed = 24.0f / 255.0f;
		float endSkyGreen = 24.0f / 255.0f;
		float endSkyBlue = 24.0f / 255.0f;
		applyPs2LegacyAtmosphereRgb(mc, endSkyRed, endSkyGreen, endSkyBlue);

		for (int_t face = 0; face < 6; ++face)
		{
			renderPushMatrix();
			if (face == 1)
				renderRotate(90.0f, 1.0f, 0.0f, 0.0f);
			if (face == 2)
				renderRotate(-90.0f, 1.0f, 0.0f, 0.0f);
			if (face == 3)
				renderRotate(180.0f, 1.0f, 0.0f, 0.0f);
			if (face == 4)
				renderRotate(90.0f, 0.0f, 0.0f, 1.0f);
			if (face == 5)
				renderRotate(-90.0f, 0.0f, 0.0f, 1.0f);

			tessellator->startDrawingQuads();
			tessellator->setColorRGBA_F(endSkyRed, endSkyGreen, endSkyBlue, 1.0f);
			tessellator->addVertexWithUV(-100.0f, -100.0f, -100.0f, 0.0f, 0.0f);
			tessellator->addVertexWithUV(-100.0f, -100.0f, 100.0f, 0.0f, 16.0f);
			tessellator->addVertexWithUV(100.0f, -100.0f, 100.0f, 16.0f, 16.0f);
			tessellator->addVertexWithUV(100.0f, -100.0f, -100.0f, 16.0f, 0.0f);
			tessellator->draw();
			renderPopMatrix();
		}

		renderDepthMask(true);
		renderEnable(RenderCapability::Texture2D);
		renderEnable(RenderCapability::AlphaTest);
		return;
	}

	if (!worldObj->worldProvider->func_48217_e())
		return;

	renderDisable(RenderCapability::Texture2D);
	Vec3D *vec3d = worldObj->getSkyColor(mc->renderViewEntity, f);
	float f1 = (float)vec3d->xCoord;
	float f2 = (float)vec3d->yCoord;
	float f3 = (float)vec3d->zCoord;

	applyPs2LegacyAtmosphereRgb(mc, f1, f2, f3);

	if (mc->gameSettings->anaglyph)
	{
		float f4 = (f1 * 30.0f + f2 * 59.0f + f3 * 11.0f) / 100.0f;
		float f5 = (f1 * 30.0f + f2 * 70.0f) / 100.0f;
		float f7 = (f1 * 30.0f + f3 * 70.0f) / 100.0f;
		f1 = f4;
		f2 = f5;
		f3 = f7;
	}

	renderColor3f(f1, f2, f3);

	Tessellator *tessellator = &Tessellator::instance;

	renderDepthMask(false);
	renderEnable(RenderCapability::Fog);
	renderColor3f(f1, f2, f3);
	if (Config::isSkyEnabled()) // OptiFine: Sky OFF (sol/luna/estrellas siguen visibles)
	{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
		renderStaticMeshDraw(skyMesh);
#else
		renderCallDisplayList(glSkyList);
#endif
	}
	renderDisable(RenderCapability::Fog);
	renderDisable(RenderCapability::AlphaTest);
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

	RenderHelper::disableStandardItemLighting();

	float *af = worldObj->worldProvider->calcSunriseSunsetColors(worldObj->getCelestialAngle(f), f);

	if (af != nullptr && Config::isSunMoonEnabled())
	{
		renderDisable(RenderCapability::Texture2D);
		renderShadeModel(RenderShadeModel::Smooth);
		renderPushMatrix();
		renderRotate(90.0f, 1.0f, 0.0f, 0.0f);

		renderRotate(MathHelper::sin(worldObj->getCelestialAngleRadians(f)) < 0.0f ? 180.0f : 0.0f, 0.0f, 0.0f, 1.0f);
		renderRotate(90.0f, 0.0f, 0.0f, 1.0f);

		float f10 = af[0];
		float f12 = af[1];
		float f14 = af[2];

		if (mc->gameSettings->anaglyph)
		{
			float f16 = (f10 * 30.0f + f12 * 59.0f + f14 * 11.0f) / 100.0f;
			float f18 = (f10 * 30.0f + f12 * 70.0f) / 100.0f;
			float f19 = (f10 * 30.0f + f14 * 70.0f) / 100.0f;
			f10 = f16;
			f12 = f18;
			f14 = f19;
		}

		float sunriseRed = f10;
		float sunriseGreen = f12;
		float sunriseBlue = f14;
		applyPs2LegacyAtmosphereRgb(mc, sunriseRed, sunriseGreen, sunriseBlue);

		float sunriseEdgeRed = af[0];
		float sunriseEdgeGreen = af[1];
		float sunriseEdgeBlue = af[2];
		applyPs2LegacyAtmosphereRgb(mc, sunriseEdgeRed, sunriseEdgeGreen, sunriseEdgeBlue);

		tessellator->startDrawing(6);
		tessellator->setColorRGBA_F(sunriseRed, sunriseGreen, sunriseBlue, af[3]);
		tessellator->addVertex(0.0f, 100.0f, 0.0f);

		int_t i = 16;
		tessellator->setColorRGBA_F(sunriseEdgeRed, sunriseEdgeGreen, sunriseEdgeBlue, 0.0f);

		for (int_t j = 0; j <= i; j++)
		{
			float f20 = ((float)j * 3.1415927f * 2.0f) / (float)i;
			float f21 = MathHelper::sin(f20);
			float f22 = MathHelper::cos(f20);
			tessellator->addVertex(f21 * 120.0f, f22 * 120.0f, -f22 * 40.0f * af[3]);
		}

		tessellator->draw();
		renderPopMatrix();
		renderShadeModel(RenderShadeModel::Flat);
	}

	renderEnable(RenderCapability::Texture2D);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::One);
	renderPushMatrix();

	float f6 = 1.0f - worldObj->getRainStrengthInterpolated(f);
	float f9 = 0.0f;
	float f11 = 0.0f;
	float f13 = 0.0f;

	renderColor4f(1.0f, 1.0f, 1.0f, f6);
	renderTranslate(f9, f11, f13);
	renderRotate(-90.0f, 0.0f, 1.0f, 0.0f);
	renderRotate(worldObj->getCelestialAngle(f) * 360.0f, 1.0f, 0.0f, 0.0f);

	if (Config::isSunMoonEnabled())
	{
		float f15 = 30.0f;
		renderBindTexture(renderEngine->getTexture("/terrain/sun.png"));
		tessellator->startDrawingQuads();
		tessellator->addVertexWithUV(-f15, 100.0f, -f15, 0.0f, 0.0f);
		tessellator->addVertexWithUV(f15, 100.0f, -f15, 1.0f, 0.0f);
		tessellator->addVertexWithUV(f15, 100.0f, f15, 1.0f, 1.0f);
		tessellator->addVertexWithUV(-f15, 100.0f, f15, 0.0f, 1.0f);
		tessellator->draw();

		f15 = 20.0f;
		renderBindTexture(renderEngine->getTexture("/terrain/moon_phases.png"));
		const int_t moonPhase = worldObj->getMoonPhase(f);
		const int_t moonPhaseColumn = moonPhase % 4;
		const int_t moonPhaseRow = moonPhase / 4 % 2;
		const float_t moonU0 = (float_t)moonPhaseColumn / 4.0f;
		const float_t moonV0 = (float_t)moonPhaseRow / 2.0f;
		const float_t moonU1 = (float_t)(moonPhaseColumn + 1) / 4.0f;
		const float_t moonV1 = (float_t)(moonPhaseRow + 1) / 2.0f;
		tessellator->startDrawingQuads();
		tessellator->addVertexWithUV(-f15, -100.0f, f15, moonU1, moonV1);
		tessellator->addVertexWithUV(f15, -100.0f, f15, moonU0, moonV1);
		tessellator->addVertexWithUV(f15, -100.0f, -f15, moonU0, moonV0);
		tessellator->addVertexWithUV(-f15, -100.0f, -f15, moonU1, moonV0);
		tessellator->draw();
	}

	renderDisable(RenderCapability::Texture2D);

	float f17 = worldObj->getStarBrightness(f) * f6;
	if (f17 > 0.0f && Config::isStarsEnabled())
	{
		float starRed = f17;
		float starGreen = f17;
		float starBlue = f17;
		applyPs2LegacyAtmosphereRgb(mc, starRed, starGreen, starBlue);
		renderColor4f(starRed, starGreen, starBlue, f17);
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
		renderStaticMeshDraw(starMesh);
#else
		renderCallDisplayList(starGLCallList);
#endif
	}

	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderDisable(RenderCapability::Blend);
	renderEnable(RenderCapability::AlphaTest);
	renderEnable(RenderCapability::Fog);
	renderPopMatrix();

	renderDisable(RenderCapability::Texture2D);
	renderColor3f(0.0f, 0.0f, 0.0f);
	Vec3D *playerPosition = mc->thePlayer->getPosition(f);
	double horizonOffset = playerPosition->yCoord - worldObj->getSeaLevel();

	if (Config::isSkyEnabled() && horizonOffset < 0.0)
	{
		renderPushMatrix();
		renderTranslate(0.0f, 12.0f, 0.0f);
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
		renderStaticMeshDraw(skyMesh2);
#else
		renderCallDisplayList(glSkyList2);
#endif
		renderPopMatrix();

		float voidHalfWidth = 1.0f;
		float voidBottom = -((float)(horizonOffset + 65.0));
		float voidTop = -voidHalfWidth;
		tessellator->startDrawingQuads();
		tessellator->setColorRGBA_I(0, 255);
		tessellator->addVertex(-voidHalfWidth, voidBottom, voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidBottom, voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidTop, voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidTop, voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidTop, -voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidTop, -voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidBottom, -voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidBottom, -voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidTop, -voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidTop, voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidBottom, voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidBottom, -voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidBottom, -voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidBottom, voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidTop, voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidTop, -voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidTop, -voidHalfWidth);
		tessellator->addVertex(-voidHalfWidth, voidTop, voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidTop, voidHalfWidth);
		tessellator->addVertex(voidHalfWidth, voidTop, -voidHalfWidth);
		tessellator->draw();
	}

	if (worldObj->worldProvider->hasSkyColorBlend())
		renderColor3f(f1 * 0.2f + 0.04f, f2 * 0.2f + 0.04f, f3 * 0.6f + 0.1f);
	else
		renderColor3f(f1, f2, f3);

	if (Config::isSkyEnabled()) // OptiFine: Sky OFF (plano del horizonte/void)
	{
		renderPushMatrix();
		renderTranslate(0.0f, -((float)(horizonOffset - 16.0)), 0.0f);
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
		renderStaticMeshDraw(skyMesh2);
#else
		renderCallDisplayList(glSkyList2);
#endif
		renderPopMatrix();
	}
	renderEnable(RenderCapability::Texture2D);
	renderDepthMask(true);
}

void RenderGlobal::renderClouds(float f)
{
	if (!mc->theWorld->worldProvider->func_48217_e())
		return;

	if (Config::isCloudsOff()) // OptiFine: Clouds OFF
		return;

	if (Config::isCloudsFancy()) // OptiFine: Clouds Fast/Fancy/Default
	{
		renderCloudsFancy(f);
		return;
	}

	renderDisable(RenderCapability::CullFace);

	float f1 = (float)(mc->renderViewEntity->lastTickPosY + (mc->renderViewEntity->posY - mc->renderViewEntity->lastTickPosY) * (double)f);

	byte_t byte0 = 32;
	int_t i = 256 / byte0;

	Tessellator *tessellator = &Tessellator::instance;

	renderBindTexture(renderEngine->getTexture("/environment/clouds.png"));
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

	Vec3D *vec3d = worldObj->getCloudFogColor(f);
	float f2 = (float)vec3d->xCoord;
	float f3 = (float)vec3d->yCoord;
	float f4 = (float)vec3d->zCoord;

	if (mc->gameSettings->anaglyph)
	{
		float f5 = (f2 * 30.0f + f3 * 59.0f + f4 * 11.0f) / 100.0f;
		float f7 = (f2 * 30.0f + f3 * 70.0f) / 100.0f;
		float f8 = (f2 * 30.0f + f4 * 70.0f) / 100.0f;
		f2 = f5;
		f3 = f7;
		f4 = f8;
	}
	applyPs2LegacyAtmosphereRgb(mc, f2, f3, f4);

	float f6 = 0.0004882813f;

	double cloudX = static_cast<double>((float)cloudOffsetX + f);
	double d = mc->renderViewEntity->prevPosX + (mc->renderViewEntity->posX - mc->renderViewEntity->prevPosX) * (double)f + cloudX * static_cast<double>(0.03f);
	double d1 = mc->renderViewEntity->prevPosZ + (mc->renderViewEntity->posZ - mc->renderViewEntity->prevPosZ) * (double)f;

	int_t j = MathHelper::floor_double(d / 2048.0);
	int_t k = MathHelper::floor_double(d1 / 2048.0);

	d -= j * 2048;
	d1 -= k * 2048;

	float f9 = (worldObj->worldProvider->getCloudHeight() - f1) + 0.33f;
	const tess_coord_t cloudLocalX = static_cast<tess_coord_t>(d);
	const tess_coord_t cloudLocalZ = static_cast<tess_coord_t>(d1);
	float f10 = static_cast<float>(cloudLocalX * static_cast<tess_coord_t>(f6));
	float f11 = static_cast<float>(cloudLocalZ * static_cast<tess_coord_t>(f6));

	tessellator->startDrawingQuads();
	tessellator->setColorRGBA_F(f2, f3, f4, 0.8f);

	for (int_t l = -byte0 * i; l < byte0 * i; l += byte0)
	{
		for (int_t i1 = -byte0 * i; i1 < byte0 * i; i1 += byte0)
		{
			tessellator->addVertexWithUV(l + 0, f9, i1 + byte0, (float)(l + 0) * f6 + f10, (float)(i1 + byte0) * f6 + f11);
			tessellator->addVertexWithUV(l + byte0, f9, i1 + byte0, (float)(l + byte0) * f6 + f10, (float)(i1 + byte0) * f6 + f11);
			tessellator->addVertexWithUV(l + byte0, f9, i1 + 0, (float)(l + byte0) * f6 + f10, (float)(i1 + 0) * f6 + f11);
			tessellator->addVertexWithUV(l + 0, f9, i1 + 0, (float)(l + 0) * f6 + f10, (float)(i1 + 0) * f6 + f11);
		}
	}

	tessellator->draw();

	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderDisable(RenderCapability::Blend);
	renderEnable(RenderCapability::CullFace);
}

bool RenderGlobal::hasCloudFog(double d, double d1, double d2, float f)
{
	return false;
}

void RenderGlobal::renderCloudsFancy(float f)
{
	renderDisable(RenderCapability::CullFace);

	float f1 = (float)(mc->renderViewEntity->lastTickPosY + (mc->renderViewEntity->posY - mc->renderViewEntity->lastTickPosY) * (double)f);

	Tessellator *tessellator = &Tessellator::instance;

	float f2 = 12.0f;
	float f3 = 4.0f;

	double fancyCloudX = static_cast<double>((float)cloudOffsetX + f);
	double d = (mc->renderViewEntity->prevPosX + (mc->renderViewEntity->posX - mc->renderViewEntity->prevPosX) * (double)f + fancyCloudX * static_cast<double>(0.03f)) / (double)f2;
	double d1 = (mc->renderViewEntity->prevPosZ + (mc->renderViewEntity->posZ - mc->renderViewEntity->prevPosZ) * (double)f) / (double)f2 + 0.33000001311302185;

	float f4 = (worldObj->worldProvider->getCloudHeight() - f1) + 0.33f;

	int_t i = MathHelper::floor_double(d / 2048.0);
	int_t j = MathHelper::floor_double(d1 / 2048.0);

	d -= i * 2048;
	d1 -= j * 2048;

	const int_t fancyCloudFloorX = MathHelper::floor_double(d);
	const int_t fancyCloudFloorZ = MathHelper::floor_double(d1);
	const tess_coord_t fancyCloudLocalX = static_cast<tess_coord_t>(d);
	const tess_coord_t fancyCloudLocalZ = static_cast<tess_coord_t>(d1);

	renderBindTexture(renderEngine->getTexture("/environment/clouds.png"));
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

	Vec3D *vec3d = worldObj->getCloudFogColor(f);
	float f5 = (float)vec3d->xCoord;
	float f6 = (float)vec3d->yCoord;
	float f7 = (float)vec3d->zCoord;

	if (mc->gameSettings->anaglyph)
	{
		float f8 = (f5 * 30.0f + f6 * 59.0f + f7 * 11.0f) / 100.0f;
		float f10 = (f5 * 30.0f + f6 * 70.0f) / 100.0f;
		float f12 = (f5 * 30.0f + f7 * 70.0f) / 100.0f;
		f5 = f8;
		f6 = f10;
		f7 = f12;
	}

	float cloudBottomRed = f5 * 0.7f;
	float cloudBottomGreen = f6 * 0.7f;
	float cloudBottomBlue = f7 * 0.7f;
	applyPs2LegacyAtmosphereRgb(mc, cloudBottomRed, cloudBottomGreen, cloudBottomBlue);

	float cloudTopRed = f5;
	float cloudTopGreen = f6;
	float cloudTopBlue = f7;
	applyPs2LegacyAtmosphereRgb(mc, cloudTopRed, cloudTopGreen, cloudTopBlue);

	float cloudSideXRed = f5 * 0.9f;
	float cloudSideXGreen = f6 * 0.9f;
	float cloudSideXBlue = f7 * 0.9f;
	applyPs2LegacyAtmosphereRgb(mc, cloudSideXRed, cloudSideXGreen, cloudSideXBlue);

	float cloudSideZRed = f5 * 0.8f;
	float cloudSideZGreen = f6 * 0.8f;
	float cloudSideZBlue = f7 * 0.8f;
	applyPs2LegacyAtmosphereRgb(mc, cloudSideZRed, cloudSideZGreen, cloudSideZBlue);

	float f9 = static_cast<float>(fancyCloudLocalX * static_cast<tess_coord_t>(0.0));
	float f11 = static_cast<float>(fancyCloudLocalZ * static_cast<tess_coord_t>(0.0));
	float f13 = 0.00390625f;

	f9 = static_cast<float>(fancyCloudFloorX) * f13;
	f11 = static_cast<float>(fancyCloudFloorZ) * f13;

	float f14 = static_cast<float>(fancyCloudLocalX - static_cast<tess_coord_t>(fancyCloudFloorX));
	float f15 = static_cast<float>(fancyCloudLocalZ - static_cast<tess_coord_t>(fancyCloudFloorZ));

	int_t k = 8;
	byte_t byte0 = 3;
	float f16 = 0.0009765625f;

	renderScale(f2, 1.0f, f2);

	for (int_t l = 0; l < 2; l++)
	{
		if (l == 0)
		{
			renderColorMask(false, false, false, false);
		}
		else if (mc->gameSettings->anaglyph)
		{
			if (EntityRenderer::anaglyphField == 0)
				renderColorMask(false, true, true, true);
			else
				renderColorMask(true, false, false, true);
		}
		else
		{
			renderColorMask(true, true, true, true);
		}

		for (int_t i1 = -byte0 + 1; i1 <= byte0; i1++)
		{
			for (int_t j1 = -byte0 + 1; j1 <= byte0; j1++)
			{
				tessellator->startDrawingQuads();

				float f17 = i1 * k;
				float f18 = j1 * k;
				float f19 = f17 - f14;
				float f20 = f18 - f15;

				if (f4 > -f3 - 1.0f)
				{
					tessellator->setColorRGBA_F(cloudBottomRed, cloudBottomGreen, cloudBottomBlue, 0.8f);
					tessellator->setNormal(0.0f, -1.0f, 0.0f);
					tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, f20 + (float)k, (f17 + 0.0f) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, f20 + (float)k, (f17 + (float)k) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, f20 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, f20 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
				}

				if (f4 <= f3 + 1.0f)
				{
					tessellator->setColorRGBA_F(cloudTopRed, cloudTopGreen, cloudTopBlue, 0.8f);
					tessellator->setNormal(0.0f, 1.0f, 0.0f);
					tessellator->addVertexWithUV(f19 + 0.0f, (f4 + f3) - f16, f20 + (float)k, (f17 + 0.0f) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, (f4 + f3) - f16, f20 + (float)k, (f17 + (float)k) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, (f4 + f3) - f16, f20 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					tessellator->addVertexWithUV(f19 + 0.0f, (f4 + f3) - f16, f20 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
				}

				tessellator->setColorRGBA_F(cloudSideXRed, cloudSideXGreen, cloudSideXBlue, 0.8f);

				if (i1 > -1)
				{
					tessellator->setNormal(-1.0f, 0.0f, 0.0f);
					for (int_t k1 = 0; k1 < k; k1++)
					{
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + 0.0f, f20 + (float)k, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + f3, f20 + (float)k, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + f3, f20 + 0.0f, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + 0.0f, f20 + 0.0f, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					}
				}

				if (i1 <= 1)
				{
					tessellator->setNormal(1.0f, 0.0f, 0.0f);
					for (int_t l1 = 0; l1 < k; l1++)
					{
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + 0.0f, f20 + (float)k, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + f3, f20 + (float)k, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + f3, f20 + 0.0f, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + 0.0f, f20 + 0.0f, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					}
				}

				tessellator->setColorRGBA_F(cloudSideZRed, cloudSideZGreen, cloudSideZBlue, 0.8f);

				if (j1 > -1)
				{
					tessellator->setNormal(0.0f, 0.0f, -1.0f);
					for (int_t i2 = 0; i2 < k; i2++)
					{
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + f3, f20 + (float)i2 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + f3, f20 + (float)i2 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, f20 + (float)i2 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, f20 + (float)i2 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
					}
				}

				if (j1 <= 1)
				{
					tessellator->setNormal(0.0f, 0.0f, 1.0f);
					for (int_t j2 = 0; j2 < k; j2++)
					{
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + f3, (f20 + (float)j2 + 1.0f) - f16, (f17 + 0.0f) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + f3, (f20 + (float)j2 + 1.0f) - f16, (f17 + (float)k) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, (f20 + (float)j2 + 1.0f) - f16, (f17 + (float)k) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, (f20 + (float)j2 + 1.0f) - f16, (f17 + 0.0f) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
					}
				}

				tessellator->draw();
			}
		}
	}

	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderDisable(RenderCapability::Blend);
	renderEnable(RenderCapability::CullFace);
}



#if PLATFORM_MESH_BUDGET && !PLATFORM_PC_LEGACY
namespace
{
	// Per-frame meshing budget.
	//
	// PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME used to be a raw count of
	// updateRenderer() calls, which two separate measurements showed was the
	// wrong meter:
	//
	//   * A renderer whose source chunks are not generated yet returns from
	//     ps2BuildRendererStep having done nothing but a 3x3 chunkExists sweep,
	//     yet still burned one of the four slots. With the outer cache ring
	//     streaming in, the FRAME log sat at "renderers updated=1 pending=48"
	//     for thousands of frames while "chunk build" stayed at 2-4ms, far under
	//     what the frame could afford. Charging only real work fixes that.
	//
	//   * A block COUNT is a poor proxy for time: the same 512-block step
	//     measured "chunk build avg=2.7ms max=55.4ms". The tail is what the
	//     player feels, so there is also a wall-clock ceiling.
	//
	// The clock is advisory. If it is dead -- which is a real possibility on a
	// console, and why this reads PlatformCompat rather than System::nanoTime --
	// elapsed always reads 0, exhausted() never fires on time, and the work
	// counter alone governs. That is a degraded budget, not a missing one.
	struct MeshBudget
	{
		int_t worked = 0;
		int_t maxUpdates = (int_t)PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME;
		long long spentUs = 0;

		bool exhausted() const
		{
			if (worked >= maxUpdates)
				return true;
			return PLATFORM_CHUNK_BUILD_BUDGET_MS > 0 &&
			       spentUs >= (long long)PLATFORM_CHUNK_BUILD_BUDGET_MS * 1000LL;
		}

		// Returns true when the renderer actually meshed, i.e. when the call
		// should be charged.
		bool run(WorldRenderer *wr)
		{
			const uint64_t t0 = PlatformCompat::getMonotonicMicros();
			wr->updateRenderer();
			const uint64_t t1 = PlatformCompat::getMonotonicMicros();

			if (!wr->lastTerrainBuildStepDidWork())
				return false;

			worked++;
			if (t1 > t0)
			{
				spentUs += (long long)(t1 - t0);
#if defined(WII_PLATFORM) && MC_LOG_LEVEL >= 2
				platformProfileMesh((long long)(t1 - t0) * 1000LL);
#endif
			}
			return true;
		}
	};
}
#endif

#if PLATFORM_PS2 && defined(PS2_RENDER_STATS)
namespace
{
long ps2UpdateNoSlotBreaks = 0;
}
#endif

bool RenderGlobal::updateRenderers(EntityLiving *entityliving, bool flag)
{
#if PLATFORM_PS2
	const bool ps2MeshPressure = trimPs2MeshCache(entityliving);
#endif
	if (worldRenderersToUpdate.empty())
		return true;

	int_t requestedUpdateLimit = std::max(1, Config::getUpdatesPerFrame());
	if (Config::isDynamicUpdates() && !isRendererUpdateMoving(entityliving)) requestedUpdateLimit = requestedUpdateLimit * 3;

#if PLATFORM_PC_LEGACY
	pcLegacyRunMeshScheduler(
		worldRenderersToUpdate,
		entityliving,
		flag,
		requestedUpdateLimit);
	compactRendererUpdateQueue();
	return worldRenderersToUpdate.empty();
#elif PLATFORM_MESH_BUDGET
	// Console terrain builds are incremental and subject to a measured wall-clock
	// budget. OptiFine may request more work while the player is stationary, but
	// it never gets to raise the platform's hard per-frame ceiling.
	const int_t effectiveUpdateLimit = std::max(1, std::min(requestedUpdateLimit, (int_t)PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME));
	MeshBudget meshBudget;
	meshBudget.maxUpdates = effectiveUpdateLimit;

	rendererUpdateCandidates.clear();
	for (WorldRenderer *candidate : worldRenderersToUpdate)
	{
		if (candidate == nullptr || !candidate->needsUpdate)
			continue;
#if PLATFORM_PS2
		if ((flag || ps2MeshPressure) && !candidate->isInFrustum && !candidate->isTerrainBuildInProgress())
			continue;
#else
		if (flag && !candidate->isInFrustum)
			continue;
#endif
#if PLATFORM_DEFER_MESH_DURING_POPULATE
		if (!candidate->isTerrainBuildInProgress() && candidate->hasPublishedTerrain() &&
			candidate->worldObj != nullptr &&
			candidate->distanceToEntitySquared(entityliving) > PLATFORM_POPULATE_MESH_DEFER_DISTANCE_SQ)
		{
			const int_t chunkX = JavaArithmetic::intShr(candidate->posX, 4);
			const int_t chunkZ = JavaArithmetic::intShr(candidate->posZ, 4);
			if (candidate->worldObj->isChunkPopulationPendingForRendering(chunkX, chunkZ))
				continue;
		}
#endif
		rendererUpdateCandidates.push_back(candidate);
	}

	auto buildInProgress = [](WorldRenderer *renderer)
	{
		return renderer->isTerrainBuildInProgress();
	};
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
	int_t profileActiveBuilders = 0;
	for (WorldRenderer *candidate : rendererUpdateCandidates)
	{
		if (candidate != nullptr && candidate->isTerrainBuildInProgress())
			++profileActiveBuilders;
	}
#endif

	const int_t attemptLimit = std::max(1, (int_t)PLATFORM_RENDERER_UPDATE_CANDIDATES_PER_FRAME);
	const float movementX = (float)entityliving->motionX;
	const float movementZ = (float)entityliving->motionZ;
	const bool hasMovementPriority = movementX * movementX + movementZ * movementZ > 0.0004f;
	auto distanceRing = [&](WorldRenderer *renderer)
	{
		const float dx = std::fabs((float)entityliving->posX - (float)renderer->posXPlus);
		const float dz = std::fabs((float)entityliving->posZ - (float)renderer->posZPlus);
		return (int_t)(std::max(dx, dz) * (1.0f / 16.0f));
	};
	auto movementAhead = [&](WorldRenderer *renderer)
	{
		if (!hasMovementPriority)
			return false;
		const float dx = (float)renderer->posXPlus - (float)entityliving->posX;
		const float dz = (float)renderer->posZPlus - (float)entityliving->posZ;
		return dx * movementX + dz * movementZ > 0.0f;
	};
	auto rendererPriority = [&](WorldRenderer *a, WorldRenderer *b)
	{
		// Block changes next to the player go first, even ahead of builds that
		// are already mid-flight: those are streaming work the player is not
		// looking at yet.
		if (a->urgentRebuild != b->urgentRebuild)
			return a->urgentRebuild;
		const bool aActive = buildInProgress(a);
		const bool bActive = buildInProgress(b);
		if (aActive != bActive)
			return aActive;
		if (a->isInFrustum != b->isInFrustum)
			return a->isInFrustum;
		const int_t aRing = distanceRing(a);
		const int_t bRing = distanceRing(b);
		if (aRing != bRing)
			return aRing < bRing;
		const bool aAhead = movementAhead(a);
		const bool bAhead = movementAhead(b);
		if (aAhead != bAhead)
			return aAhead;
		const float aDistance = a->distanceToEntitySquared(entityliving);
		const float bDistance = b->distanceToEntitySquared(entityliving);
		if (aDistance != bDistance)
			return aDistance < bDistance;
		return a->chunkIndex < b->chunkIndex;
	};

	// Source readiness is intentionally not pre-scanned here. The platform build
	// step already owns the authoritative source-availability check and generation
	// gate. Keeping the scheduler limited to the closest attemptLimit candidates
	// avoids doing the same 3x3 chunkExists sweep twice for renderers that mesh.
	const std::size_t sortedCandidateCount = std::min<std::size_t>(
		rendererUpdateCandidates.size(), static_cast<std::size_t>(attemptLimit));
	if (sortedCandidateCount > 0)
	{
		std::partial_sort(rendererUpdateCandidates.begin(),
			rendererUpdateCandidates.begin() + sortedCandidateCount,
			rendererUpdateCandidates.end(), rendererPriority);
	}

	int_t attempted = 0;
	int_t completed = 0;

	// The lease reserve below only applies while an edit is actually waiting;
	// otherwise streaming keeps every slot. Urgent renderers sort first.
	const bool urgentPending = sortedCandidateCount > 0 &&
		rendererUpdateCandidates[0] != nullptr &&
		rendererUpdateCandidates[0]->urgentRebuild &&
		rendererUpdateCandidates[0]->needsUpdate;

	// Urgent lane: a section the player just edited is run to completion on
	// its own wall-clock budget, before the shared per-frame budget is spent on
	// streaming. Early Pocket Edition rebuilt the edited section synchronously;
	// this is the bounded form of that. Sorted first, so they sit at the front.
	if (PLATFORM_URGENT_MESH_BUDGET_MS > 0)
	{
		long long urgentSpentUs = 0;
		for (std::size_t i = 0; i < sortedCandidateCount; ++i)
		{
			WorldRenderer *candidate = rendererUpdateCandidates[i];
			if (candidate == nullptr || !candidate->urgentRebuild)
				break;
			if (!candidate->needsUpdate)
			{
				candidate->urgentRebuild = false;
				continue;
			}
			if (!buildInProgress(candidate) && !renderTerrainStagingHasFreeSlot())
			{
#if PLATFORM_PS2
				// The reserve lease only holds while an edit is already waiting;
				// when the edit arrives with every lease taken, take one back.
				if (!evictStreamingBuildForUrgent(entityliving))
					break;
#else
				break;
#endif
			}

			attempted++;
			// Step cap as well as the clock: on a board where the monotonic
			// clock reads 0 (see PS2_CHUNK_BUILD_BUDGET_MS) the clock alone
			// would never stop this loop.
			int_t urgentSteps = 0;
#if PLATFORM_PS2 && MC_LOG_LEVEL >= 2
			const bool urgentBuildActiveAtEntry = candidate->isTerrainBuildInProgress();
			const unsigned int urgentRestartsAtEntry = candidate->ps2BuildRestarts;
			int_t urgentOwnerSteps = 0;
#endif
#if PLATFORM_PS2
			// A completed section waits at publish while another renderer owns
			// the sort/pack pipeline, and that owner only advances on its own
			// scheduler step. Left alone, the edit sat behind a streaming
			// section's pack for as many frames as that pack took. Step the
			// owner through on the urgent budget first; it is at most one
			// section's publish, and the lease it releases is the one the edit
			// needs anyway.
			int_t ownerPacketPolls = 0;
			for (WorldRenderer *owner = WorldRenderer::ps2PublishOwnerRenderer();
			     owner != nullptr && owner != candidate && urgentSteps < (int_t)PLATFORM_URGENT_MESH_STEP_CAP &&
			     urgentSpentUs < (long long)PLATFORM_URGENT_MESH_BUDGET_MS * 1000LL;
			     owner = WorldRenderer::ps2PublishOwnerRenderer())
			{
				++urgentSteps;
#if MC_LOG_LEVEL >= 2
				++urgentOwnerSteps;
#endif
				const uint64_t t0 = PlatformCompat::getMonotonicMicros();
				owner->updateRenderer();
				const uint64_t t1 = PlatformCompat::getMonotonicMicros();
				if (t1 > t0)
					urgentSpentUs += (long long)(t1 - t0);
				if (!owner->needsUpdate)
					completed++;
				if (!owner->lastTerrainBuildStepDidWork())
				{
					// Its pack is in flight on VU0; see the poll note below.
					if (WorldRenderer::ps2PublishOwnerRenderer() == owner && ownerPacketPolls < 256)
					{
						++ownerPacketPolls;
						--urgentSteps;
						continue;
					}
					break;
				}
			}
#endif
#if PLATFORM_PS2
			// The VU0 pack at the end of a build runs in batches, and a step
			// that finds the batch still in flight reports no work. On the
			// streaming path that is a yield to the next frame; here it would
			// make the edit take one frame per batch. Poll again instead; the
			// batch is microseconds, and the clock still bounds the lane.
			int_t urgentPacketPolls = 0;
#endif
			while (candidate->needsUpdate && urgentSteps < (int_t)PLATFORM_URGENT_MESH_STEP_CAP &&
			       urgentSpentUs < (long long)PLATFORM_URGENT_MESH_BUDGET_MS * 1000LL)
			{
				++urgentSteps;
				const uint64_t t0 = PlatformCompat::getMonotonicMicros();
				candidate->updateRenderer();
				const uint64_t t1 = PlatformCompat::getMonotonicMicros();
				if (t1 > t0)
					urgentSpentUs += (long long)(t1 - t0);
				if (!candidate->lastTerrainBuildStepDidWork())
				{
#if PLATFORM_PS2
					if (WorldRenderer::ps2PublishOwnerRenderer() == candidate && urgentPacketPolls < 256)
					{
						++urgentPacketPolls;
						--urgentSteps;
						continue;
					}
#endif
					break;
				}
			}
#if PLATFORM_PS2 && MC_LOG_LEVEL >= 2
			{
				const uint64_t nowUs = PlatformCompat::getMonotonicMicros();
				MC_LOG_DEBUG("ps2", "urgent mesh: %s after %ld us (steps=%d owner=%d polls=%d lane=%lld us"
					" activeIn=%d activeOut=%d restarts=%u pending=%d)\n",
					candidate->needsUpdate ? "yielded" : "published",
					(long)(nowUs > candidate->urgentMarkUs ? nowUs - candidate->urgentMarkUs : 0),
					(int)urgentSteps, (int)urgentOwnerSteps, (int)urgentPacketPolls, urgentSpentUs,
					urgentBuildActiveAtEntry ? 1 : 0, candidate->isTerrainBuildInProgress() ? 1 : 0,
					candidate->ps2BuildRestarts - urgentRestartsAtEntry,
					(int)worldRenderersToUpdate.size());
			}
#endif
			if (!candidate->needsUpdate)
			{
				candidate->urgentRebuild = false;
				completed++;
			}
		}
	}

	bool madeProgress = true;
	while (!meshBudget.exhausted() && attempted < attemptLimit && madeProgress)
	{
		madeProgress = false;
		for (std::size_t i = 0; i < sortedCandidateCount; ++i)
		{
			if (attempted >= attemptLimit || meshBudget.exhausted())
				break;

			WorldRenderer *candidate = rendererUpdateCandidates[i];
			if (candidate == nullptr || !candidate->needsUpdate)
				continue;

			// When all leases are occupied, only the active builders can make
			// progress. The next round starts from the front again, so a renderer
			// that just completed immediately frees a slot for the nearest waiting
			// candidate without spreading one step across the whole dirty queue.
			if (!buildInProgress(candidate))
			{
				if (!renderTerrainStagingHasFreeSlot())
				{
#if PLATFORM_PS2 && defined(PS2_RENDER_STATS)
					++ps2UpdateNoSlotBreaks;
#endif
					break;
				}
				// Keep one lease for the urgent lane, so an edit never waits for
				// a streaming build to finish before it can even start.
				if (urgentPending && !candidate->urgentRebuild && PLATFORM_MESH_STAGING_RESERVE_FOR_URGENT > 0 &&
				    renderTerrainStagingSlotsInUse() >= PLATFORM_MESH_STAGING_SLOTS - PLATFORM_MESH_STAGING_RESERVE_FOR_URGENT)
					continue;
			}

			attempted++;
			const bool worked = meshBudget.run(candidate);
			madeProgress |= worked;
			if (!candidate->needsUpdate)
			{
				candidate->urgentRebuild = false;
				completed++;
			}
		}
	}

#if PLATFORM_PS2
	// updateRenderers may leave the single VU0 finalize owner between batches.
	// Drain micro mode before returning to the render pipeline, where matrix,
	// culling and fallback paths issue VU0 macro/COP2 instructions. The packed
	// result remains in VU0 data RAM and is collected by the owner on its next
	// scheduler step without keeping micro mode active across rendering.
	(void)ps2_vu0_mesh_finalize_drain();
#endif

	compactRendererUpdateQueue();
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
	platformProfileMeshScheduler(attempted, meshBudget.worked, completed, profileActiveBuilders,
		static_cast<int>(worldRenderersToUpdate.size()));
#endif

#if PLATFORM_PS2 && defined(PS2_RENDER_STATS)
	static int s_ps2UpdateLogTick = 0;
	static long s_ps2UpdateAttempts = 0;
	static long s_ps2UpdateSteps = 0;
	static long s_ps2UpdateCompleted = 0;
	s_ps2UpdateAttempts += attempted;
	s_ps2UpdateSteps += meshBudget.worked;
	s_ps2UpdateCompleted += completed;
	if (++s_ps2UpdateLogTick >= 120)
	{
		s_ps2UpdateLogTick = 0;
		// cand/slots are this frame's; noSlot counts the frames in the period
		// where the scheduler stopped for want of a staging lease.
		MC_LOG_DEBUG("ps2", "renderer work: attempts=%ld steps=%ld complete=%ld pending=%d cand=%d slots=%d/%d noSlot=%ld\n",
			s_ps2UpdateAttempts, s_ps2UpdateSteps, s_ps2UpdateCompleted,
			(int)worldRenderersToUpdate.size(), (int)rendererUpdateCandidates.size(),
			renderTerrainStagingSlotsInUse(), (int)PLATFORM_MESH_STAGING_SLOTS, ps2UpdateNoSlotBreaks);
		s_ps2UpdateAttempts = 0;
		s_ps2UpdateSteps = 0;
		s_ps2UpdateCompleted = 0;
		ps2UpdateNoSlotBreaks = 0;
	}
#endif

	return worldRenderersToUpdate.empty();
#else
	// OptiFine C6 chooses work by distance instead of sorting/rebuilding the
	// complete dirty list. Sections outside the frustum are penalized fourfold,
	// and active block/item interaction makes nearby dirty sections immediate.
	const bool acting = isRendererUpdateActing(entityliving);
	int_t updated = 0;
	rendererUpdateCandidates.clear();

	auto weightedDistance = [&](WorldRenderer *renderer)
	{
		float distance = renderer->distanceToEntitySquared(entityliving);
		if (!renderer->isInFrustum)
			distance *= 4.0f;
		return distance;
	};

	for (WorldRenderer *renderer : worldRenderersToUpdate)
	{
		if (renderer == nullptr)
			continue;
		if (!renderer->needsUpdate)
		{
			dequeueRendererUpdate(renderer);
			continue;
		}
		if (flag && !renderer->isInFrustum)
			continue;

		const float distance = renderer->distanceToEntitySquared(entityliving);
		if (acting && distance <= 256.0f)
		{
			renderer->updateRenderer();
			if (!renderer->needsUpdate)
				dequeueRendererUpdate(renderer);
			updated++;
			continue;
		}
		rendererUpdateCandidates.push_back(renderer);
	}

	WorldRenderer *best = nullptr;
	float bestDistance = 0.0f;
	for (WorldRenderer *renderer : rendererUpdateCandidates)
	{
		const float distance = weightedDistance(renderer);
		if (best == nullptr || distance < bestDistance)
		{
			best = renderer;
			bestDistance = distance;
		}
	}

	if (best != nullptr && updated < requestedUpdateLimit)
	{
		best->updateRenderer();
		if (!best->needsUpdate)
			dequeueRendererUpdate(best);
		updated++;

		// C6 groups renderers whose weighted distance is close to the best one so
		// chunk rings fill coherently instead of producing isolated rebuilt holes.
		const float maxDistanceDifference = bestDistance / 5.0f;
		for (WorldRenderer *renderer : rendererUpdateCandidates)
		{
			if (updated >= requestedUpdateLimit)
				break;
			if (renderer == best || !renderer->needsUpdate)
				continue;
			const float distance = weightedDistance(renderer);
			if (std::fabs(distance - bestDistance) >= maxDistanceDifference)
				continue;
			renderer->updateRenderer();
			if (!renderer->needsUpdate)
				dequeueRendererUpdate(renderer);
			updated++;
		}
	}

	compactRendererUpdateQueue();

	// C6 intentionally reports the per-frame batch as complete even when dirty
	// renderers remain queued. EntityRenderer uses this return value to decide
	// whether to call updateRenderers() again before the frame deadline; returning
	// queue-empty here would therefore multiply the configured Chunk Updates limit
	// several times in one frame. The remaining work stays queued for the next one.
	return true;
#endif
}

void RenderGlobal::drawBlockBreaking(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f)
{
	Tessellator *tessellator = &Tessellator::instance;

	renderEnable(RenderCapability::Blend);
	renderEnable(RenderCapability::AlphaTest);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::One);
	renderColor4f(1.0f, 1.0f, 1.0f, (MathHelper::sin((float)PlatformCompat::getTicks() / 100.0f) * 0.2f + 0.4f) * 0.5f);

	if (i == 0)
	{
		if (damagePartialTime > 0.0f)
		{
			renderBlendFunc(RenderBlendFactor::DstColor, RenderBlendFactor::SrcColor);
			int_t j = renderEngine->getTexture("/terrain.png");
			renderBindTexture(j);
			renderColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			renderPushMatrix();

			int_t k = worldObj->getBlockId(movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ);
			Block *block = k <= 0 ? nullptr : Block::blocksList[k];

			renderDisable(RenderCapability::AlphaTest);
			renderPolygonOffset(-3.0f, -3.0f);
			renderEnable(RenderCapability::PolygonOffsetFill);

			double d = entityplayer->lastTickPosX + (entityplayer->posX - entityplayer->lastTickPosX) * (double)f;
			double d1 = entityplayer->lastTickPosY + (entityplayer->posY - entityplayer->lastTickPosY) * (double)f;
			double d2 = entityplayer->lastTickPosZ + (entityplayer->posZ - entityplayer->lastTickPosZ) * (double)f;

			if (block == nullptr)
				block = Block::stone;

			renderEnable(RenderCapability::AlphaTest);
			tessellator->startDrawingQuads();
			tessellator->setTranslationD(-d, -d1, -d2);
			tessellator->disableColor();
			globalRenderBlocks->renderBlockUsingTexture(block, movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ, 240 + (int_t)(damagePartialTime * 10.0f));
			tessellator->draw();
			tessellator->setTranslationD(0.0, 0.0, 0.0);

			renderDisable(RenderCapability::AlphaTest);
			renderPolygonOffset(0.0f, 0.0f);
			renderDisable(RenderCapability::PolygonOffsetFill);
			renderEnable(RenderCapability::AlphaTest);
			renderDepthMask(true);
			renderPopMatrix();
		}
	}
	else if (itemstack != nullptr)
	{
		renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
		float f1 = MathHelper::sin((float)PlatformCompat::getTicks() / 100.0f) * 0.2f + 0.8f;
		renderColor4f(f1, f1, f1, MathHelper::sin((float)PlatformCompat::getTicks() / 200.0f) * 0.2f + 0.5f);

		int_t l = renderEngine->getTexture("/terrain.png");
		renderBindTexture(l);

		int_t i1 = movingobjectposition->blockX;
		int_t j1 = movingobjectposition->blockY;
		int_t k1 = movingobjectposition->blockZ;

		if (movingobjectposition->sideHit == 0)
			j1--;
		if (movingobjectposition->sideHit == 1)
			j1++;
		if (movingobjectposition->sideHit == 2)
			k1--;
		if (movingobjectposition->sideHit == 3)
			k1++;
		if (movingobjectposition->sideHit == 4)
			i1--;
		if (movingobjectposition->sideHit == 5)
			i1++;
	}

	renderDisable(RenderCapability::Blend);
	renderDisable(RenderCapability::AlphaTest);
}

void RenderGlobal::drawSelectionBox(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f)
{
	if (i == 0 && movingobjectposition->typeOfHit == EnumMovingObjectType::TILE)
	{
		renderEnable(RenderCapability::Blend);
		renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
		renderColor4f(0.0f, 0.0f, 0.0f, 0.4f);
		renderLineWidth(2.0f);
		renderDisable(RenderCapability::Texture2D);
		renderDepthMask(false);

		float f1 = 0.002f;

		int_t j = worldObj->getBlockId(movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ);
		if (j > 0)
		{
			Block::blocksList[j]->setBlockBoundsBasedOnState(worldObj, movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ);

			double d = entityplayer->lastTickPosX + (entityplayer->posX - entityplayer->lastTickPosX) * (double)f;
			double d1 = entityplayer->lastTickPosY + (entityplayer->posY - entityplayer->lastTickPosY) * (double)f;
			double d2 = entityplayer->lastTickPosZ + (entityplayer->posZ - entityplayer->lastTickPosZ) * (double)f;

			drawOutlinedBoundingBox(Block::blocksList[j]->getSelectedBoundingBoxFromPool(worldObj, movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ)->expand(f1, f1, f1)->getOffsetBoundingBox(-d, -d1, -d2));
		}

		renderDepthMask(true);
		renderEnable(RenderCapability::Texture2D);
		renderDisable(RenderCapability::Blend);
	}
}

void RenderGlobal::drawOutlinedBoundingBox(AxisAlignedBB *axisalignedbb)
{
	Tessellator *tessellator = &Tessellator::instance;

	tessellator->startDrawing(renderPrimitiveValue(RenderPrimitive::LineStrip));
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->draw();

	tessellator->startDrawing(renderPrimitiveValue(RenderPrimitive::LineStrip));
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->draw();

	tessellator->startDrawing(renderPrimitiveValue(RenderPrimitive::Lines));
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->draw();
}

void RenderGlobal::markBlockAndNeighborsNeedsUpdate(int_t i, int_t j, int_t k)
{
	markRenderersInRange(i - 1, j - 1, k - 1, i + 1, j + 1, k + 1);
}

void RenderGlobal::markBlockRangeNeedsUpdate(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
	markRenderersInRange(i - 1, j - 1, k - 1, l + 1, i1 + 1, j1 + 1);
}

void RenderGlobal::onChunkPublished(int_t chunkX, int_t chunkZ)
{
#if PLATFORM_PS2
	if (worldRenderers == nullptr)
		return;

	const int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
	for (int_t i = 0; i < total; ++i)
	{
		WorldRenderer *renderer = worldRenderers[i];
		if (renderer == nullptr || !renderer->needsRebuildForPublishedChunk(chunkX, chunkZ))
			continue;

		enqueueRendererUpdatePriority(renderer);
		if (!renderer->needsUpdate)
			renderer->markDirty();
	}
#else
	(void)chunkX;
	(void)chunkZ;
#endif
}

void RenderGlobal::markRenderersInRange(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1) // func_949_a
{
	int_t k1 = MathHelper::bucketInt(i, 16);
	int_t l1 = MathHelper::bucketInt(j, 16);
	int_t i2 = MathHelper::bucketInt(k, 16);
	int_t j2 = MathHelper::bucketInt(l, 16);
	int_t k2 = MathHelper::bucketInt(i1, 16);
	int_t l2 = MathHelper::bucketInt(j1, 16);

	for (int_t i3 = k1; i3 <= j2; i3++)
	{
		int_t j3 = i3 % renderChunksWide;
		if (j3 < 0)
			j3 += renderChunksWide;

		for (int_t k3 = l1; k3 <= k2; k3++)
		{
#if PLATFORM_CENTER_VERTICAL_RENDERERS
			// The vertical window does not wrap by modulo: slot 0 holds world
			// section verticalStartSection. Map section->slot the same way as
			// markRenderersForNewPosition(), and skip sections outside the window
			// (those have no resident renderer to dirty).
			int_t l3 = k3 - verticalStartSection;
			if (l3 < 0 || l3 >= renderChunksTall)
				continue;
#else
			int_t l3 = k3 % renderChunksTall;
			if (l3 < 0)
				l3 += renderChunksTall;
#endif

			for (int_t i4 = i2; i4 <= l2; i4++)
			{
				int_t j4 = i4 % renderChunksDeep;
				if (j4 < 0)
					j4 += renderChunksDeep;

				int_t k4 = (j4 * renderChunksTall + l3) * renderChunksWide + j3;
				WorldRenderer *worldrenderer = worldRenderers[k4];

#if PLATFORM_PS2 || PLATFORM_WII
				// Active builds must observe every mutation so deferred population
				// can mark one final rebuild without throwing away the current staging
				// mesh. markDirty() itself decides whether to coalesce or restart; a
				// light-only mark always coalesces.
				enqueueRendererUpdatePriority(worldrenderer);
				if (worldObj != nullptr && worldObj->isMarkingFromLighting())
				{
					worldrenderer->markDirtyFromLighting();
				}
				else
				{
					worldrenderer->markDirty();
					// A block the player just placed or broke, next to the viewer.
					// See the urgent lane in updateRenderers(). The edit scope is
					// what separates it from a spring or a gravel vein settling at
					// the same distance while terrain streams in.
					if (PLATFORM_URGENT_MESH_DISTANCE_SQ > 0.0f && mc != nullptr &&
					    mc->renderViewEntity != nullptr &&
					    worldObj != nullptr && worldObj->isMarkingFromPlayerEdit() &&
					    worldrenderer->distanceToEntitySquared(mc->renderViewEntity) <= PLATFORM_URGENT_MESH_DISTANCE_SQ)
					{
#if PLATFORM_PS2 && MC_LOG_LEVEL >= 2
						worldrenderer->urgentMarkUs = PlatformCompat::getMonotonicMicros();
#endif
						worldrenderer->urgentRebuild = true;
					}
				}
#elif PLATFORM_PC_LEGACY
				// Legacy keeps only one queue entry, but active incremental builds still
				// need to hear every mutation so population can be coalesced safely.
				enqueueRendererUpdate(worldrenderer);
				worldrenderer->markDirty();
#else
				if (!worldrenderer->needsUpdate)
				{
					enqueueRendererUpdate(worldrenderer);
					worldrenderer->markDirty();
				}
#endif
			}
		}
	}
}

void RenderGlobal::playSound(const jstring &s, double d, double d1, double d2, float f, float f1)
{
	float f2 = 16.0f;
	if (f > 1.0f)
		f2 *= f;

	if (mc->renderViewEntity->getDistanceSq(d, d1, d2) < (double)(f2 * f2))
		mc->sndManager->playSound(s, (float)d, (float)d1, (float)d2, f, f1);
}

void RenderGlobal::spawnParticle(const jstring &name, double x, double y, double z,
                                 double velocityX, double velocityY, double velocityZ)
{
	(void)spawnParticleEffect(name, x, y, z, velocityX, velocityY, velocityZ);
}

EntityFX *RenderGlobal::spawnParticleEffect(const jstring &name, double x, double y, double z,
                                            double velocityX, double velocityY, double velocityZ)
{
	if (mc == nullptr || mc->renderViewEntity == nullptr || mc->effectRenderer == nullptr)
		return nullptr;

	int_t particleSetting = mc->gameSettings != nullptr ? mc->gameSettings->particleSetting : 0;
	if (particleSetting == 1 && worldObj != nullptr && worldObj->rand.nextInt(3) == 0)
		particleSetting = 2;

	EntityFX *effect = nullptr;
	if (name == "hugeexplosion")
	{
		if (Config::isAnimatedExplosion())
			effect = new EntityHugeExplodeFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}
	else if (name == "largeexplode")
	{
		if (Config::isAnimatedExplosion())
			effect = new EntityLargeExplodeFX(renderEngine, worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}

	if (effect != nullptr)
	{
		mc->effectRenderer->addEffect(effect);
		return effect;
	}

	const double dx = mc->renderViewEntity->posX - x;
	const double dy = mc->renderViewEntity->posY - y;
	const double dz = mc->renderViewEntity->posZ - z;
	constexpr double maxDistance = 16.0;
	if (dx * dx + dy * dy + dz * dz > maxDistance * maxDistance)
		return nullptr;
	if (particleSetting > 1)
		return nullptr;

	if (name == "bubble")
		effect = new EntityBubbleFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "suspended")
	{
		if (Config::isWaterParticles())
			effect = new EntitySuspendFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}
	else if (name == "depthsuspend")
	{
		if (Config::isVoidParticles())
			effect = new EntityAuraFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}
	else if (name == "townaura")
		effect = new EntityAuraFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "crit")
		effect = new EntityCritFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "magicCrit")
	{
		EntityCritFX *crit = new EntityCritFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
		crit->setParticleColor(crit->getParticleRed() * 0.3f,
		                       crit->getParticleGreen() * 0.8f,
		                       crit->getParticleBlue());
		crit->setParticleTextureIndex(crit->getParticleTextureIndex() + 1);
		effect = crit;
	}
	else if (name == "smoke")
	{
		if (Config::isAnimatedSmoke())
			effect = new EntitySmokeFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}
	else if (name == "mobSpell")
	{
		EntitySpellParticleFX *spell = new EntitySpellParticleFX(worldObj, x, y, z, 0.0, 0.0, 0.0);
		spell->setParticleColor(static_cast<float>(velocityX), static_cast<float>(velocityY),
		                        static_cast<float>(velocityZ));
		effect = spell;
	}
	else if (name == "spell")
		effect = new EntitySpellParticleFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "instantSpell")
	{
		EntitySpellParticleFX *spell = new EntitySpellParticleFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
		spell->setBaseSpellTextureIndex(144);
		effect = spell;
	}
	else if (name == "note")
		effect = new EntityNoteFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "portal")
	{
		if (Config::isPortalParticles())
			effect = new EntityPortalFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}
	else if (name == "enchantmenttable")
		effect = new EntityEnchantmentTableParticleFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "explode")
	{
		if (Config::isAnimatedExplosion())
			effect = new EntityExplodeFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}
	else if (name == "flame")
	{
		if (Config::isAnimatedFlame())
			effect = new EntityFlameFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	}
	else if (name == "lava")
		effect = new EntityLavaFX(worldObj, x, y, z);
	else if (name == "footstep")
		effect = new EntityFootStepFX(renderEngine, worldObj, x, y, z);
	else if (name == "splash")
		effect = new EntitySplashFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "largesmoke")
	{
		if (Config::isAnimatedSmoke())
			effect = new EntitySmokeFX(worldObj, x, y, z, velocityX, velocityY, velocityZ, 2.5f);
	}
	else if (name == "cloud")
		effect = new EntityCloudFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "reddust")
	{
		if (Config::isAnimatedRedstone())
			effect = new EntityReddustFX(worldObj, x, y, z, static_cast<float>(velocityX),
			                          static_cast<float>(velocityY), static_cast<float>(velocityZ));
	}
	else if (name == "snowballpoof")
		effect = new EntityBreakingFX(worldObj, x, y, z, Item::snowball);
	else if (name == "dripWater")
	{
		if (Config::isDrippingWaterLava())
			effect = new EntityDropParticleFX(worldObj, x, y, z, Material::water);
	}
	else if (name == "dripLava")
	{
		if (Config::isDrippingWaterLava())
			effect = new EntityDropParticleFX(worldObj, x, y, z, Material::lava);
	}
	else if (name == "snowshovel")
		effect = new EntitySnowShovelFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name == "slime")
		effect = new EntityBreakingFX(worldObj, x, y, z, Item::slimeBall);
	else if (name == "heart")
		effect = new EntityHeartFX(worldObj, x, y, z, velocityX, velocityY, velocityZ);
	else if (name.rfind("iconcrack_", 0) == 0)
	{
		int_t itemId = -1;
		if (String::tryParseInt(jstring(name.substr(10)), itemId) && itemId >= 0 && itemId < Item::ITEM_LIST_SIZE)
			effect = new EntityBreakingFX(worldObj, x, y, z, velocityX, velocityY, velocityZ, Item::itemsList[itemId]);
	}
	else if (name.rfind("tilecrack_", 0) == 0)
	{
		int_t blockId = -1;
		if (String::tryParseInt(jstring(name.substr(10)), blockId) && blockId >= 0 && blockId < Block::BLOCK_REGISTRY_SIZE &&
		    Block::blocksList[blockId] != nullptr)
		{
			effect = new EntityDiggingFX(worldObj, x, y, z, velocityX, velocityY, velocityZ,
			                             Block::blocksList[blockId], 0, 0);
		}
	}

	if (effect != nullptr)
	{
		if (name == "portal")
			CustomColorizer::updatePortalFX(effect);
		else if (name == "splash" || name == "bubble")
			CustomColorizer::updateWaterFX(effect, worldObj);
		else if (name == "reddust")
			CustomColorizer::updateReddustFX(effect, worldObj, x, y, z);
		else if (name == "townaura")
			CustomColorizer::updateMyceliumFX(effect);
		mc->effectRenderer->addEffect(effect);
	}
	return effect;
}

void RenderGlobal::obtainEntitySkin(Entity *entity)
{
	entity->updateCloak();
	if (!entity->skinUrl.empty())
		renderEngine->obtainImageData(entity->skinUrl, new ImageBufferDownload());

	if (!entity->cloakUrl.empty())
		renderEngine->obtainImageData(entity->cloakUrl, new ImageBufferDownload());
}

void RenderGlobal::releaseEntitySkin(Entity *entity)
{
	if (!entity->skinUrl.empty())
		renderEngine->releaseImageData(entity->skinUrl);

	if (!entity->cloakUrl.empty())
		renderEngine->releaseImageData(entity->cloakUrl);
}

void RenderGlobal::updateAllRenderers()
{
	int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
	for (int_t i = 0; i < total; i++)
	{
		if (worldRenderers[i]->isChunkLit && !worldRenderers[i]->needsUpdate)
		{
			enqueueRendererUpdate(worldRenderers[i]);
			worldRenderers[i]->markDirty();
		}
	}
}

void RenderGlobal::playRecord(const jstring &s, int_t i, int_t j, int_t k)
{
	if (!s.empty())
		mc->ingameGUI->setRecordPlayingMessage("C418 - " + s);

	mc->sndManager->playStreaming(s, i, j, k, 1.0f, 1.0f);
}

void RenderGlobal::doNothingWithTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity)
{
	if (tileentity == nullptr)
		return;

	// Scrub non-owning render references before World/Chunk deletes the object.
	tileEntities.erase(std::remove(tileEntities.begin(), tileEntities.end(), tileentity), tileEntities.end());

	if (worldRenderers != nullptr)
	{
		int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
		for (int_t idx = 0; idx < total; ++idx)
		{
			WorldRenderer *renderer = worldRenderers[idx];
			if (renderer == nullptr)
				continue;
#if PLATFORM_PC_LEGACY
			pcLegacyStaticTileEntityUnpublish(tileentity, renderer);
			renderer->pcLegacyStaticTileEntityRenderers.erase(
				std::remove(renderer->pcLegacyStaticTileEntityRenderers.begin(), renderer->pcLegacyStaticTileEntityRenderers.end(), tileentity),
				renderer->pcLegacyStaticTileEntityRenderers.end());
#endif
			renderer->tileEntityRenderers.erase(
				std::remove(renderer->tileEntityRenderers.begin(), renderer->tileEntityRenderers.end(), tileentity),
				renderer->tileEntityRenderers.end());
		}
	}
}

void RenderGlobal::playAuxSFX(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1)
{
	Random &random = worldObj->rand;

	switch (i)
	{
	default:
		break;

	case 1000:
		worldObj->playSoundEffect((double)j, (double)k, (double)l, "random.click", 1.0f, 1.0f);
		break;
	case 1001:
		worldObj->playSoundEffect((double)j, (double)k, (double)l, "random.click", 1.0f, 1.2f);
		break;
	case 1002:
		worldObj->playSoundEffect((double)j, (double)k, (double)l, "random.bow", 1.0f, 1.2f);
		break;
	case 1003:
		if (Math::random() < 0.5)
			worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "random.door_open", 1.0f, worldObj->rand.nextFloat() * 0.1f + 0.9f);
		else
			worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "random.door_close", 1.0f, worldObj->rand.nextFloat() * 0.1f + 0.9f);
		break;
	case 1004:
		worldObj->playSoundEffect((double)((float)j + 0.5f), (double)((float)k + 0.5f), (double)((float)l + 0.5f), "random.fizz", 0.5f, 2.6f + random.nextFloatDifference() * 0.8f);
		break;
	case 1005:
		if (i1 >= 0 && i1 < Item::ITEM_LIST_SIZE && Item::itemsList[i1] != nullptr && dynamic_cast<ItemRecord *>(Item::itemsList[i1]) != nullptr)
			worldObj->playRecord(((ItemRecord *)Item::itemsList[i1])->recordName, j, k, l);
		else
			worldObj->playRecord("", j, k, l);
		break;
	case 1007:
		worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "mob.ghast.charge", 10.0f, random.nextFloatDifference() * 0.2f + 1.0f);
		break;
	case 1008:
		worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "mob.ghast.fireball", 10.0f, random.nextFloatDifference() * 0.2f + 1.0f);
		break;
	case 1010:
		worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "mob.zombie.wood", 2.0f, random.nextFloatDifference() * 0.2f + 1.0f);
		break;
	case 1011:
		worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "mob.zombie.metal", 2.0f, random.nextFloatDifference() * 0.2f + 1.0f);
		break;
	case 1012:
		worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "mob.zombie.woodbreak", 2.0f, random.nextFloatDifference() * 0.2f + 1.0f);
		break;

	case 2000:
	{
		int_t xDir = i1 % 3 - 1;
		int_t zDir = (i1 / 3) % 3 - 1;
		double baseX = (double)j + (double)xDir * 0.6 + 0.5;
		double baseY = (double)k + 0.5;
		double baseZ = (double)l + (double)zDir * 0.6 + 0.5;
		for (int_t n = 0; n < 10; ++n)
		{
			double speed = random.nextDouble() * 0.2 + 0.01;
			double px = baseX + (double)xDir * 0.01 + (random.nextDouble() - 0.5) * (double)zDir * 0.5;
			double py = baseY + (random.nextDouble() - 0.5) * 0.5;
			double pz = baseZ + (double)zDir * 0.01 + (random.nextDouble() - 0.5) * (double)xDir * 0.5;
			double vx = (double)xDir * speed + random.nextGaussian() * 0.01;
			double vy = -0.03 + random.nextGaussian() * 0.01;
			double vz = (double)zDir * speed + random.nextGaussian() * 0.01;
			spawnParticle("smoke", px, py, pz, vx, vy, vz);
		}
		return;
	}

	case 2001:
	{
		int_t blockId = i1 & 4095;
		if (blockId > 0 && blockId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[blockId] != nullptr)
		{
			Block *block = Block::blocksList[blockId];
			mc->sndManager->playSound(block->stepSound->getBreakSound(), (float)j + 0.5f, (float)k + 0.5f, (float)l + 0.5f,
			                          (block->stepSound->getVolume() + 1.0f) / 2.0f, block->stepSound->getPitch() * 0.8f);
		}
		if (mc->effectRenderer != nullptr)
			mc->effectRenderer->addBlockDestroyEffects(j, k, l, blockId, (i1 >> 12) & 255);
		break;
	}

	case 2002:
	{
		double px = (double)j;
		double py = (double)k;
		double pz = (double)l;
		jstring icon = "iconcrack_" + String::toString(Item::potion->shiftedIndex);
		for (int_t n = 0; n < 8; ++n)
		{
			double vx = random.nextGaussian() * 0.15;
			double vy = random.nextDouble() * 0.2;
			double vz = random.nextGaussian() * 0.15;
			spawnParticle(icon, px, py, pz, vx, vy, vz);
		}

		ItemPotion *potion = dynamic_cast<ItemPotion *>(Item::potion);
		int_t color = Item::potion->getColorFromDamage(i1, 0);
		float red = (float)((color >> 16) & 255) / 255.0f;
		float green = (float)((color >> 8) & 255) / 255.0f;
		float blue = (float)(color & 255) / 255.0f;
		jstring particle = (potion != nullptr && potion->isEffectInstant(i1)) ? "instantSpell" : "spell";
#if PLATFORM_FLOAT_VERTEX_MATH
		constexpr float pi = 3.1415927f;
		for (int_t n = 0; n < 100; ++n)
		{
			const float radius = random.nextDoubleFloat() * 4.0f;
			const float angle = random.nextDoubleFloat() * pi * 2.0f;
			const float vx = std::cos(angle) * radius;
			const double vy = 0.01 + random.nextDouble() * 0.5;
			const float vz = std::sin(angle) * radius;
			EntityFX *effect = spawnParticleEffect(particle, px + static_cast<double>(vx) * 0.1, py + 0.3, pz + static_cast<double>(vz) * 0.1,
			                                      static_cast<double>(vx), vy, static_cast<double>(vz));
#else
		constexpr double pi = 3.14159265358979323846;
		for (int_t n = 0; n < 100; ++n)
		{
			double radius = random.nextDouble() * 4.0;
			double angle = random.nextDouble() * pi * 2.0;
			double vx = JavaMath::cos(angle) * radius;
			double vy = 0.01 + random.nextDouble() * 0.5;
			double vz = JavaMath::sin(angle) * radius;
			EntityFX *effect = spawnParticleEffect(particle, px + vx * 0.1, py + 0.3, pz + vz * 0.1, vx, vy, vz);
#endif
			if (effect != nullptr)
			{
				float brightness = 0.75f + random.nextFloat() * 0.25f;
				effect->setParticleColor(red * brightness, green * brightness, blue * brightness);
				effect->multiplyVelocity((float)radius);
			}
		}
		worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, "random.glass", 1.0f,
		                          worldObj->rand.nextFloat() * 0.1f + 0.9f);
		break;
	}

	case 2003:
	{
		double px = (double)j + 0.5;
		double py = (double)k;
		double pz = (double)l + 0.5;
		jstring icon = "iconcrack_" + String::toString(Item::eyeOfEnder->shiftedIndex);
		for (int_t n = 0; n < 8; ++n)
		{
			double vx = random.nextGaussian() * 0.15;
			double vy = random.nextDouble() * 0.2;
			double vz = random.nextGaussian() * 0.15;
			spawnParticle(icon, px, py, pz, vx, vy, vz);
		}
#if PLATFORM_FLOAT_VERTEX_MATH
		constexpr float pi = 3.1415927f;
		for (float angle = 0.0f; angle < pi * 2.0f; angle += pi * 0.05f)
		{
			const float c = std::cos(angle);
			const float s = std::sin(angle);
			spawnParticle("portal", px + static_cast<double>(c) * 5.0, py - 0.4, pz + static_cast<double>(s) * 5.0,
			              static_cast<double>(c) * -5.0, 0.0, static_cast<double>(s) * -5.0);
			spawnParticle("portal", px + static_cast<double>(c) * 5.0, py - 0.4, pz + static_cast<double>(s) * 5.0,
			              static_cast<double>(c) * -7.0, 0.0, static_cast<double>(s) * -7.0);
		}
#else
		constexpr double pi = 3.14159265358979323846;
		for (double angle = 0.0; angle < pi * 2.0; angle += pi * 0.05)
		{
			double c = JavaMath::cos(angle);
			double s = JavaMath::sin(angle);
			spawnParticle("portal", px + c * 5.0, py - 0.4, pz + s * 5.0, c * -5.0, 0.0, s * -5.0);
			spawnParticle("portal", px + c * 5.0, py - 0.4, pz + s * 5.0, c * -7.0, 0.0, s * -7.0);
		}
#endif
		return;
	}

	case 2004:
		for (int_t n = 0; n < 20; ++n)
		{
			double px = (double)j + 0.5 + ((double)worldObj->rand.nextFloat() - 0.5) * 2.0;
			double py = (double)k + 0.5 + ((double)worldObj->rand.nextFloat() - 0.5) * 2.0;
			double pz = (double)l + 0.5 + ((double)worldObj->rand.nextFloat() - 0.5) * 2.0;
			worldObj->spawnParticle("smoke", px, py, pz, 0.0, 0.0, 0.0);
			worldObj->spawnParticle("flame", px, py, pz, 0.0, 0.0, 0.0);
		}
		break;
	}
}

void RenderGlobal::clipRenderersByFrustrum(ICamera *icamera, float f)
{
	int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
	for (int_t i = 0; i < total; i++)
	{
#if PLATFORM_PS2
		// Re-test every section every frame, dropping vanilla's `& 0xf`.
		//
		// Vanilla only re-tests an ALREADY-ACCEPTED section once every 16 frames,
		// which is the right trade on a desktop grid of ~2000 sections: the test
		// is the cost there, and a stale accept merely draws a chunk that is cheap
		// to draw. Both halves are false on console. The PS2 grid is 5x3x5 = 75,
		// so testing all of them is 75 isBoxInFrustum calls (six float plane
		// evaluations each, no doubles since ClippingHelper was rewritten), while
		// a single stale accept costs ~400 EE vertex transforms that all die in
		// the per-triangle clip outcodes.
		//
		// Measured 2026-07-27 in the stats build, camera at chunk-grid centre:
		// "terrain pass0 rendered=30/64 total=72 skip=38 clip=4" -- only 4 of the
		// 34 non-empty sections rejected, when roughly a third of them sit behind
		// the camera -- against 7313 triangles/frame killed by the clip outcodes.
		// Turning the camera is exactly when those stale accepts pile up, which is
		// also when the frame time was worst.
		//
		// The skipAllRenderPasses() gate is dropped too, and that part is a
		// CORRECTNESS fix, not a performance one. This pass runs before
		// updateRenderers, so a section that finishes meshing later in the same
		// frame was drawn by sortAndRender carrying frustum flags from whenever it
		// last held geometry -- a different camera position, sometimes a different
		// section entirely. A stale isFullyInFrustum then told the draw path to
		// skip clipping for geometry that had since moved behind the camera, and
		// the perspective divide smeared it across the screen as huge stretched
		// polygons (1-5 sections per frame, which is why it looked random).
		//
		// The classification depends only on the section's box and the camera,
		// never on whether a mesh exists, so testing an empty renderer is both
		// valid and cheap: the full grid is 75 boxes against six float planes.
		worldRenderers[i]->updateInFrustrum(icamera);
#elif PLATFORM_WII
		// Drop vanilla's `& 0xf` re-test throttle, but keep the
		// skipAllRenderPasses() gate. The PS2 rationale above applies only in
		// part here, and the two halves are worth separating.
		//
		// The throttle only ever applies to sections that are ALREADY accepted:
		// the `!isInFrustum` half of vanilla's condition re-tests every rejected
		// section every frame anyway. So dropping it adds one box test per
		// section currently in view -- a few hundred -- and removes a window of
		// up to 16 frames in which a section that has left the frustum keeps
		// being submitted. Nothing downstream caps how many sections a Wii pass
		// draws (PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS is read on the PS2 path
		// only), so each stale accept is a whole GX display list replayed for
		// nothing, and they pile up exactly while the camera is turning.
		//
		// The gate stays because it is not a staleness source. skipAllRenderPasses()
		// returns false until a section has been meshed at least once, so a section
		// that has never held geometry is still tested every frame; one that is
		// initialised and empty is never drawn whatever its flag says, and testing
		// it would only pay for the sections that are cheapest to reject.
		//
		// isFullyInFrustum is not involved: WorldRenderer::updateInFrustrum never
		// sets it on Wii, so the correctness half of the PS2 note has no Wii
		// equivalent to fix.
		if (!worldRenderers[i]->skipAllRenderPasses())
			worldRenderers[i]->updateInFrustrum(icamera);
#else
		if (!worldRenderers[i]->skipAllRenderPasses() && (!worldRenderers[i]->isInFrustum || (i + frustrumCheckOffset & 0xf) == 0))
			worldRenderers[i]->updateInFrustrum(icamera);
#endif
	}

	frustrumCheckOffset++;
}

bool RenderGlobal::isCloudFog(double x, double y, double z, float partialTicks)
{
	return false;
}

void RenderGlobal::clearWorldRenderers() // func_28137_f
{
	if (worldRenderers != nullptr)
	{
		int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
		for (int_t i = 0; i < total; i++)
		{
			delete worldRenderers[i];
		}
		delete[] worldRenderers;
		worldRenderers = nullptr;
	}
	delete[] sortedWorldRenderers;
	sortedWorldRenderers = nullptr;
	std::vector<WorldRenderer *>().swap(worldRenderersToUpdate);
#if !defined(PS2_PLATFORM)
	std::unordered_set<WorldRenderer *>().swap(worldRenderersQueuedForUpdate);
#endif
	std::vector<TileEntity *>().swap(tileEntities);
	std::vector<WorldRenderer *>().swap(renderBatchRenderers);
	renderChunksWide = 0;
	renderChunksTall = 0;
	renderChunksDeep = 0;
	worldRenderersCheckIndex = 0;
}

#if !PLATFORM_PS2
void RenderGlobal::renderAllRenderLists(int_t pass, double partialTick)
{
	(void)pass;
	if (mc != nullptr && mc->entityRenderer != nullptr)
		mc->entityRenderer->enableLightmap(partialTick);
	for (int_t i = 0; i < 4; i++)
	{
		if (allRenderLists[i] != nullptr)
			allRenderLists[i]->render();
	}
	if (mc != nullptr && mc->entityRenderer != nullptr)
		mc->entityRenderer->disableLightmap(partialTick);
}
#endif
