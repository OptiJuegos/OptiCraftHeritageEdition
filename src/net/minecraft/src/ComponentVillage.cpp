#include "ComponentVillage.h"

#include <algorithm>

#include "ComponentVillageStartPiece.h"
#include "EntityVillager.h"
#include "StructureBoundingBox.h"
#include "StructureVillagePieces.h"
#include "World.h"
#include "WorldProvider.h"
#include "java/Random.h"

ComponentVillage::ComponentVillage(int_t componentType)
    : StructureComponent(componentType), villagersSpawned(0)
{
}

StructureComponent *ComponentVillage::getNextComponentNN(
    ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
    Random &random, int_t yOffset, int_t sideOffset)
{
    if (start == nullptr || boundingBox == nullptr)
        return nullptr;

    switch (coordBaseMode)
    {
    case 0:
    case 2:
        return StructureVillagePieces::getNextStructureComponent(
            start, components, random, boundingBox->minX - 1,
            boundingBox->minY + yOffset, boundingBox->minZ + sideOffset,
            1, getComponentType());
    case 1:
    case 3:
        return StructureVillagePieces::getNextStructureComponent(
            start, components, random, boundingBox->minX + sideOffset,
            boundingBox->minY + yOffset, boundingBox->minZ - 1,
            2, getComponentType());
    default:
        return nullptr;
    }
}

StructureComponent *ComponentVillage::getNextComponentPP(
    ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
    Random &random, int_t yOffset, int_t sideOffset)
{
    if (start == nullptr || boundingBox == nullptr)
        return nullptr;

    switch (coordBaseMode)
    {
    case 0:
    case 2:
        return StructureVillagePieces::getNextStructureComponent(
            start, components, random, boundingBox->maxX + 1,
            boundingBox->minY + yOffset, boundingBox->minZ + sideOffset,
            3, getComponentType());
    case 1:
    case 3:
        return StructureVillagePieces::getNextStructureComponent(
            start, components, random, boundingBox->minX + sideOffset,
            boundingBox->minY + yOffset, boundingBox->maxZ + 1,
            0, getComponentType());
    default:
        return nullptr;
    }
}

int_t ComponentVillage::getAverageGroundLevel(
    World *world, const StructureBoundingBox &chunkBounds) const
{
    if (world == nullptr || world->worldProvider == nullptr || boundingBox == nullptr)
        return -1;

    int_t totalHeight = 0;
    int_t count = 0;
    for (int_t z = boundingBox->minZ; z <= boundingBox->maxZ; ++z)
    {
        for (int_t x = boundingBox->minX; x <= boundingBox->maxX; ++x)
        {
            if (!chunkBounds.isVecInside(x, 64, z))
                continue;

            totalHeight += std::max(world->getTopSolidOrLiquidBlock(x, z),
                                    world->worldProvider->getAverageGroundLevel());
            ++count;
        }
    }

    return count == 0 ? -1 : totalHeight / count;
}

bool ComponentVillage::canVillageGoDeeper(const StructureBoundingBox *bounds)
{
    return bounds != nullptr && bounds->minY > 10;
}

void ComponentVillage::spawnVillagers(
    World *world, const StructureBoundingBox &chunkBounds,
    int_t x, int_t y, int_t z, int_t count)
{
    if (world == nullptr || villagersSpawned >= count)
        return;

    for (int_t index = villagersSpawned; index < count; ++index)
    {
        const int_t worldX = getXWithOffset(x + index, z);
        const int_t worldY = getYWithOffset(y);
        const int_t worldZ = getZWithOffset(x + index, z);
        if (!chunkBounds.isVecInside(worldX, worldY, worldZ))
            break;

        ++villagersSpawned;
        EntityVillager *villager = new EntityVillager(world, getVillagerType(index));
        villager->setLocationAndAngles((double)worldX + 0.5, (double)worldY,
                                       (double)worldZ + 0.5, 0.0f, 0.0f);
        if (!world->entityJoinedWorld(villager))
            delete villager;
    }
}

int_t ComponentVillage::getVillagerType(int_t) const
{
    return 0;
}
