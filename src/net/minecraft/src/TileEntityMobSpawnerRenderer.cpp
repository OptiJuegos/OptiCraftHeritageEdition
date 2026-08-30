#include "TileEntityMobSpawnerRenderer.h"

#include "TileEntityMobSpawner.h"
#include "EntityList.h"
#include "World.h"
#include "RenderManager.h"
#include "Entity.h"

#include "platform/PlatformTuning.h"
#include "platform/RenderAPI.h"

TileEntityMobSpawnerRenderer::TileEntityMobSpawnerRenderer()
{
}

TileEntityMobSpawnerRenderer::~TileEntityMobSpawnerRenderer()
{
    for (auto &entry : entityHashMap)
        delete entry.second;
    entityHashMap.clear();
}

void TileEntityMobSpawnerRenderer::renderTileEntityMobSpawner(TileEntityMobSpawner *tileentitymobspawner, double d, double d1, double d2, float f)
{
    renderPushMatrix();
    renderTranslate((float)d + 0.5f, (float)d1, (float)d2 + 0.5f);
    Entity *entity = nullptr;
    auto it = entityHashMap.find(tileentitymobspawner->getMobID());
    if (it == entityHashMap.end())
    {
        entity = EntityList::createEntityInWorld(tileentitymobspawner->getMobID(), nullptr);
        entityHashMap[tileentitymobspawner->getMobID()] = entity;
    }
    else
    {
        entity = it->second;
    }
    if (entity != nullptr)
    {
        entity->setWorld(tileentitymobspawner->worldObj);
        float f1 = 0.4375f;
        renderTranslate(0.0f, 0.4f, 0.0f);
#if PLATFORM_FLOAT_VERTEX_MATH
        const float interpolatedYaw = static_cast<float>(tileentitymobspawner->yaw2) +
            (static_cast<float>(tileentitymobspawner->yaw) - static_cast<float>(tileentitymobspawner->yaw2)) * f;
        renderRotate(interpolatedYaw * 10.0f, 0.0f, 1.0f, 0.0f);
#else
        renderRotate((float)(tileentitymobspawner->yaw2 + (tileentitymobspawner->yaw - tileentitymobspawner->yaw2) * (double)f) * 10.0f, 0.0f, 1.0f, 0.0f);
#endif
        renderRotate(-30.0f, 1.0f, 0.0f, 0.0f);
        renderTranslate(0.0f, -0.4f, 0.0f);
        renderScale(f1, f1, f1);
        entity->setLocationAndAngles(d, d1, d2, 0.0f, 0.0f);
        RenderManager::instance->renderEntityWithPosYaw(entity, 0.0, 0.0, 0.0, 0.0f, f);
    }
    renderPopMatrix();
}

void TileEntityMobSpawnerRenderer::renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f)
{
    renderTileEntityMobSpawner((TileEntityMobSpawner *)tileentity, d, d1, d2, f);
}
