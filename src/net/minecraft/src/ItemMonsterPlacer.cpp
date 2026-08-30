#include "ItemMonsterPlacer.h"

#include "Block.h"
#include "Entity.h"
#include "EntityEggInfo.h"
#include "EntityList.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "PlayerCapabilities.h"
#include "StatCollector.h"
#include "World.h"

ItemMonsterPlacer::ItemMonsterPlacer(int id)
    : Item(id)
{
    setHasSubtypes(true);
}

std::string ItemMonsterPlacer::getItemDisplayName(ItemStack *stack)
{
    std::string name = StatCollector::translateToLocal(getItemName() + ".name");
    const std::string entityName = EntityList::getStringFromID(stack->getItemDamage());
    if (!entityName.empty())
        name += " " + StatCollector::translateToLocal("entity." + entityName + ".name");
    return name;
}

int ItemMonsterPlacer::getColorFromDamage(int damage, int renderPass)
{
    const EntityEggInfo *info = EntityList::getEggInfo(damage);
    return info != nullptr ? (renderPass == 0 ? info->primaryColor : info->secondaryColor) : 0xffffff;
}

bool ItemMonsterPlacer::func_46058_c()
{
    return true;
}

int ItemMonsterPlacer::func_46057_a(int damage, int renderPass)
{
    return renderPass > 0 ? Item::func_46057_a(damage, renderPass) + 16 : Item::func_46057_a(damage, renderPass);
}

bool ItemMonsterPlacer::onItemUse(ItemStack *stack, EntityPlayer *player, World *world, int x, int y, int z, int side)
{
    if (world->multiplayerWorld)
        return true;

    const int_t clickedBlock = world->getBlockId(x, y, z);
    static const int_t offsetX[6] = {0, 0, 0, 0, -1, 1};
    static const int_t offsetY[6] = {-1, 1, 0, 0, 0, 0};
    static const int_t offsetZ[6] = {0, 0, -1, 1, 0, 0};
    if (side < 0 || side >= 6)
        return false;
    x += offsetX[side];
    y += offsetY[side];
    z += offsetZ[side];

    double yOffset = 0.0;
    if ((side == 1 && clickedBlock == Block::fence->blockID) || clickedBlock == Block::netherFence->blockID)
        yOffset = 0.5;

    if (spawnCreature(world, stack->getItemDamage(), static_cast<double>(x) + 0.5,
                      static_cast<double>(y) + yOffset, static_cast<double>(z) + 0.5) &&
        !player->capabilities.isCreativeMode)
    {
        --stack->stackSize;
    }
    return true;
}

bool ItemMonsterPlacer::spawnCreature(World *world, int_t entityId, double x, double y, double z)
{
    if (EntityList::getEggInfo(entityId) == nullptr)
        return false;

    Entity *entity = EntityList::createEntity(entityId, world);
    if (entity == nullptr || !entity->isLiving())
    {
        delete entity;
        return false;
    }

    EntityLiving *living = static_cast<EntityLiving *>(entity);
    living->setLocationAndAngles(x, y, z, world->rand.nextFloat() * 360.0f, 0.0f);
    bool spawned = world->spawnEntityInWorld(living);
    living->playLivingSound();
    if (!spawned)
    {
        delete living;
        return false;
    }
    return true;
}
