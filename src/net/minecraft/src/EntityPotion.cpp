#include "EntityPotion.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

#include <cmath>
#include <vector>

#include "AxisAlignedBB.h"
#include "EntityLiving.h"
#include "Item.h"
#include "ItemPotion.h"
#include "MovingObjectPosition.h"
#include "Potion.h"
#include "PotionEffect.h"
#include "World.h"

EntityPotion::EntityPotion(World *world) : EntityThrowable(world)
{
}

EntityPotion::EntityPotion(World *world, EntityLiving *thrower, int damage)
    : EntityThrowable(world, thrower), potionDamage(damage)
{
}

EntityPotion::EntityPotion(World *world, double x, double y, double z, int damage)
    : EntityThrowable(world, x, y, z), potionDamage(damage)
{
}

float EntityPotion::getGravityVelocity() const
{
    return 0.05f;
}

float EntityPotion::getVelocity() const
{
    return 0.5f;
}

float EntityPotion::getInaccuracyPitchOffset() const
{
    return -20.0f;
}

int EntityPotion::getPotionDamage() const
{
    return potionDamage;
}

void EntityPotion::onImpact(MovingObjectPosition *hit)
{
    if (worldObj->multiplayerWorld)
        return;

    ItemPotion *potionItem = dynamic_cast<ItemPotion *>(Item::potion);
    if (potionItem != nullptr)
    {
        const auto &effects = potionItem->getEffects(potionDamage);
        if (!effects.empty())
        {
            AxisAlignedBB *area = boundingBox->expand(4.0, 2.0, 4.0);
            const auto &entities = worldObj->getEntitiesWithinAABB(typeid(EntityLiving), area);
            std::vector<Entity *> affectedEntities(entities.begin(), entities.end());
            for (Entity *entity : affectedEntities)
            {
                if (entity == nullptr || !entity->isLiving())
                    continue;
                const double distanceSq = getDistanceSqToEntity(entity);
                if (distanceSq >= 16.0)
                    continue;

                double strength = 1.0 - JavaMath::sqrt(distanceSq) / 4.0;
                if (hit != nullptr && entity == hit->entityHit)
                    strength = 1.0;

                EntityLiving *living = static_cast<EntityLiving *>(entity);
                for (const PotionEffect &effect : effects)
                {
                    Potion *potion = Potion::getPotion(effect.getPotionID());
                    if (potion == nullptr)
                        continue;
                    if (potion->isInstant())
                    {
                        potion->affectEntity(getThrower(), living, effect.getAmplifier(), strength);
                    }
                    else
                    {
                        const int duration = JavaArithmetic::doubleToInt(strength * static_cast<double>(effect.getDuration()) + 0.5);
                        if (duration > 20)
                            living->addPotionEffect(new PotionEffect(effect.getPotionID(), duration, effect.getAmplifier()));
                    }
                }
            }
        }
    }

    worldObj->playAuxSFX(2002,
                         JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posX)),
                         JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posY)),
                         JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posZ)),
                         potionDamage);
    setEntityDead();
}
