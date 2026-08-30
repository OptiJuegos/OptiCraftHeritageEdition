#include "EntityFish.h"
#include "java/Arithmetic.h"
#include "DamageSource.h"
#include "java/Math.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "EntityItem.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "StatList.h"
#include "Vec3D.h"
#include "World.h"
#include "platform/PlatformTuning.h"

EntityFish::EntityFish(World *world)
    : Entity(world)
{
    ensureEntityInit();
    xTile = -1;
    yTile = -1;
    zTile = -1;
    inTile = 0;
    inGround = false;
    shake = 0;
    anglerEntityId = -1;
    bobberEntityId = -1;
    ticksInGround = 0;
    ticksInAir = 0;
    ticksCatchable = 0;
    fishPosRotationIncrements = 0;
    velocityX = velocityY = velocityZ = 0.0;
    setSize(0.25f, 0.25f);
    ignoreFrustumCheck = true;
}

EntityFish::EntityFish(World *world, double d, double d1, double d2)
    : EntityFish(world)
{
    setPosition(d, d1, d2);
    ignoreFrustumCheck = true;
}

EntityFish::EntityFish(World *world, EntityPlayer *entityplayer)
    : Entity(world)
{
    ensureEntityInit();
    xTile = -1;
    yTile = -1;
    zTile = -1;
    inTile = 0;
    inGround = false;
    shake = 0;
    anglerEntityId = entityplayer != nullptr ? entityplayer->entityId : -1;
    bobberEntityId = -1;
    if (entityplayer != nullptr)
        entityplayer->fishEntity = this;
    ticksInGround = 0;
    ticksInAir = 0;
    ticksCatchable = 0;
    fishPosRotationIncrements = 0;
    velocityX = velocityY = velocityZ = 0.0;
    setSize(0.25f, 0.25f);
    ignoreFrustumCheck = true;
    setLocationAndAngles(entityplayer->posX, entityplayer->posY + 1.62 - (double)entityplayer->yOffset,
                         entityplayer->posZ, entityplayer->rotationYaw, entityplayer->rotationPitch);
    posX -= MathHelper::cos((rotationYaw / 180.0f) * 3.1415927f) * 0.16f;
    posY -= 0.10000000149011612;
    posZ -= MathHelper::sin((rotationYaw / 180.0f) * 3.1415927f) * 0.16f;
    setPosition(posX, posY, posZ);
    yOffset = 0.0f;
    float f = 0.4f;
    motionX = -MathHelper::sin((rotationYaw / 180.0f) * 3.1415927f) * MathHelper::cos((rotationPitch / 180.0f) * 3.1415927f) * f;
    motionZ = MathHelper::cos((rotationYaw / 180.0f) * 3.1415927f) * MathHelper::cos((rotationPitch / 180.0f) * 3.1415927f) * f;
    motionY = -MathHelper::sin((rotationPitch / 180.0f) * 3.1415927f) * f;
    setFishHeading(motionX, motionY, motionZ, 1.5f, 1.0f);
}

void EntityFish::entityInit() {}

EntityPlayer *EntityFish::getAngler() const
{
    if (worldObj == nullptr || anglerEntityId < 0)
        return nullptr;
    return dynamic_cast<EntityPlayer *>(worldObj->getEntityByID(anglerEntityId));
}

Entity *EntityFish::getBobber() const
{
    if (worldObj == nullptr || bobberEntityId < 0)
        return nullptr;
    return worldObj->getEntityByID(bobberEntityId);
}

void EntityFish::setBobber(Entity *entity)
{
    bobberEntityId = entity != nullptr ? entity->entityId : -1;
}

void EntityFish::setEntityDead()
{
    EntityPlayer *angler = getAngler();
    if (angler != nullptr && angler->fishEntity == this)
        angler->fishEntity = nullptr;
    anglerEntityId = -1;
    bobberEntityId = -1;
    Entity::setEntityDead();
}

void EntityFish::setFishHeading(double d, double d1, double d2, float f, float f1)
{
    float f2 = MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2);
    d /= (double)f2;
    d1 /= (double)f2;
    d2 /= (double)f2;
    d += rand.nextGaussian() * 0.0074999998323619366 * (double)f1;
    d1 += rand.nextGaussian() * 0.0074999998323619366 * (double)f1;
    d2 += rand.nextGaussian() * 0.0074999998323619366 * (double)f1;
    d *= (double)f;
    d1 *= (double)f;
    d2 *= (double)f;
    motionX = d;
    motionY = d1;
    motionZ = d2;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
    const float dx = static_cast<float>(d);
    const float dy = static_cast<float>(d1);
    const float dz = static_cast<float>(d2);
    const float f3 = MathHelper::sqrt_float(dx * dx + dz * dz);
    prevRotationYaw = rotationYaw = std::atan2(dx, dz) * 180.0f / 3.1415927f;
    prevRotationPitch = rotationPitch = std::atan2(dy, f3) * 180.0f / 3.1415927f;
#else
    float f3 = MathHelper::sqrt_double(d * d + d2 * d2);
    prevRotationYaw = rotationYaw = (float)((JavaMath::atan2(d, d2) * 180.0) / 3.1415927410125732);
    prevRotationPitch = rotationPitch = (float)((JavaMath::atan2(d1, (double)f3) * 180.0) / 3.1415927410125732);
#endif
    ticksInGround = 0;
}

void EntityFish::setVelocity(double d, double d1, double d2)
{
    velocityX = motionX = d;
    velocityY = motionY = d1;
    velocityZ = motionZ = d2;
}

void EntityFish::onUpdate()
{
    Entity::onUpdate();

    if (fishPosRotationIncrements > 0)
    {
        double d = posX + (fishX - posX) / (double)fishPosRotationIncrements;
        double d1 = posY + (fishY - posY) / (double)fishPosRotationIncrements;
        double d2 = posZ + (fishZ - posZ) / (double)fishPosRotationIncrements;
        double d4;
        for (d4 = fishYaw - (double)rotationYaw; d4 < -180.0; d4 += 360.0) {}
        for (; d4 >= 180.0; d4 -= 360.0) {}
        rotationYaw += (float)(d4 / (double)fishPosRotationIncrements);
        rotationPitch += (float)((fishPitch - (double)rotationPitch) / (double)fishPosRotationIncrements);
        --fishPosRotationIncrements;
        setPosition(d, d1, d2);
        setRotation(rotationYaw, rotationPitch);
        return;
    }

    if (!worldObj->multiplayerWorld)
    {
        EntityPlayer *angler = getAngler();
        if (angler == nullptr)
        {
            setEntityDead();
            return;
        }
        ItemStack *itemstack = angler->getCurrentEquippedItem();
        if (angler->isDead || !angler->isEntityAlive() || itemstack == nullptr ||
            itemstack->getItem() != Item::fishingRod || getDistanceSqToEntity(angler) > 1024.0)
        {
            setEntityDead();
            return;
        }
        Entity *bobber = getBobber();
        if (bobber != nullptr)
        {
            if (bobber->isDead)
                setBobber(nullptr);
            else
            {
                posX = bobber->posX;
                posY = bobber->boundingBox->minY + (double)bobber->height * 0.8;
                posZ = bobber->posZ;
                return;
            }
        }
    }

    if (shake > 0)
        --shake;

    if (inGround)
    {
        int_t i = worldObj->getBlockId(xTile, yTile, zTile);
        if (i != inTile)
        {
            inGround = false;
            motionX *= rand.nextFloat() * 0.2f;
            motionY *= rand.nextFloat() * 0.2f;
            motionZ *= rand.nextFloat() * 0.2f;
            ticksInGround = 0;
            ticksInAir = 0;
        }
        else
        {
            ++ticksInGround;
            if (ticksInGround == 1200)
                setEntityDead();
            return;
        }
    }
    else
    {
        ++ticksInAir;
    }

    Vec3D *vec3d = Vec3D::createVector(posX, posY, posZ);
    Vec3D *vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
    MovingObjectPosition *movingobjectposition = worldObj->rayTraceBlocks(vec3d, vec3d1);
    vec3d = Vec3D::createVector(posX, posY, posZ);
    vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
    if (movingobjectposition != nullptr)
        vec3d1 = Vec3D::createVector(movingobjectposition->hitVec->xCoord, movingobjectposition->hitVec->yCoord, movingobjectposition->hitVec->zCoord);

    Entity *entity = nullptr;
    EntityPlayer *angler = getAngler();
    const auto &list = worldObj->getEntitiesWithinAABBExcludingEntity(this,
        boundingBox->addCoord(motionX, motionY, motionZ)->expand(1.0, 1.0, 1.0));
    double nearest = 0.0;
    for (Entity *entity1 : list)
    {
        if (!entity1->canBeCollidedWith() || (entity1 == angler && ticksInAir < 5))
            continue;
        float f2 = 0.3f;
        AxisAlignedBB *axisalignedbb = entity1->boundingBox->expand(f2, f2, f2);
        MovingObjectPosition *mop = axisalignedbb->calculateIntercept(vec3d, vec3d1);
        if (mop == nullptr)
            continue;
        double hitDist = vec3d->distanceTo(mop->hitVec);
        if (hitDist < nearest || nearest == 0.0)
        {
            entity = entity1;
            nearest = hitDist;
        }
        delete mop;
    }

    if (entity != nullptr)
    {
        delete movingobjectposition;
        movingobjectposition = new MovingObjectPosition(entity);
    }

    if (movingobjectposition != nullptr)
    {
        if (movingobjectposition->entityHit != nullptr)
        {
            if (movingobjectposition->entityHit->attackEntityFrom(DamageSource::causeThrownDamage(this, angler), 0))
                setBobber(movingobjectposition->entityHit);
        }
        else
        {
            inGround = true;
        }
    }
    delete movingobjectposition;

    if (inGround)
        return;

    moveEntity(motionX, motionY, motionZ);
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
    const float motionXf = static_cast<float>(motionX);
    const float motionYf = static_cast<float>(motionY);
    const float motionZf = static_cast<float>(motionZ);
    const float f = MathHelper::sqrt_float(motionXf * motionXf + motionZf * motionZf);
    rotationYaw = std::atan2(motionXf, motionZf) * 180.0f / 3.1415927f;
    for (rotationPitch = std::atan2(motionYf, f) * 180.0f / 3.1415927f;
#else
    float f = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
    rotationYaw = (float)((JavaMath::atan2(motionX, motionZ) * 180.0) / 3.1415927410125732);
    for (rotationPitch = (float)((JavaMath::atan2(motionY, (double)f) * 180.0) / 3.1415927410125732);
#endif
         rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
    for (; rotationPitch - prevRotationPitch >= 180.0f; prevRotationPitch += 360.0f) {}
    for (; rotationYaw - prevRotationYaw < -180.0f; prevRotationYaw -= 360.0f) {}
    for (; rotationYaw - prevRotationYaw >= 180.0f; prevRotationYaw += 360.0f) {}
    rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
    rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;

    float drag = 0.92f;
    if (onGround || isCollidedHorizontally)
        drag = 0.5f;

    const int_t slices = 5;
    double waterFraction = 0.0;
    for (int_t l = 0; l < slices; ++l)
    {
        double d8 = boundingBox->minY + ((boundingBox->maxY - boundingBox->minY) * (double)l) / (double)slices;
        double d9 = boundingBox->minY + ((boundingBox->maxY - boundingBox->minY) * (double)(l + 1)) / (double)slices;
        AxisAlignedBB *slice = AxisAlignedBB::getBoundingBoxFromPool(boundingBox->minX, d8, boundingBox->minZ,
                                                                     boundingBox->maxX, d9, boundingBox->maxZ);
        if (worldObj->isAABBInMaterial(slice, Material::water))
            waterFraction += 1.0 / (double)slices;
    }

    if (waterFraction > 0.0)
    {
        if (ticksCatchable > 0)
        {
            --ticksCatchable;
        }
        else
        {
            int_t delay = 500;
            if (worldObj->canLightningStrikeAt(MathHelper::floor_double(posX), MathHelper::floor_double(posY) + 1,
                                               MathHelper::floor_double(posZ)))
                delay = 300;
            if (rand.nextInt(delay) == 0)
            {
                ticksCatchable = rand.nextInt(30) + 10;
                motionY -= 0.20000000298023224;
                worldObj->playSoundAtEntity(this, "random.splash", 0.25f,
                                            1.0f + rand.nextFloatDifference() * 0.4f);
                float splashY = (float)MathHelper::floor_double(boundingBox->minY);
                for (int_t i = 0; (float)i < 1.0f + width * 20.0f; ++i)
                {
                    float dx = (rand.nextFloat() * 2.0f - 1.0f) * width;
                    float dz = (rand.nextFloat() * 2.0f - 1.0f) * width;
                    worldObj->spawnParticle("bubble", posX + dx, splashY + 1.0f, posZ + dz,
                                            motionX, motionY - rand.nextFloat() * 0.2f, motionZ);
                }
                for (int_t i = 0; (float)i < 1.0f + width * 20.0f; ++i)
                {
                    float dx = (rand.nextFloat() * 2.0f - 1.0f) * width;
                    float dz = (rand.nextFloat() * 2.0f - 1.0f) * width;
                    worldObj->spawnParticle("splash", posX + dx, splashY + 1.0f, posZ + dz,
                                            motionX, motionY, motionZ);
                }
            }
        }
    }

    if (ticksCatchable > 0)
        motionY -= (double)(rand.nextFloatProduct3()) * 0.2;

    double buoyancy = waterFraction * 2.0 - 1.0;
    motionY += 0.039999999105930328 * buoyancy;
    if (waterFraction > 0.0)
    {
        drag = (float)((double)drag * 0.9);
        motionY *= 0.8;
    }
    motionX *= drag;
    motionY *= drag;
    motionZ *= drag;
    setPosition(posX, posY, posZ);
}

void EntityFish::handleHealthUpdate(byte_t byte0)
{
    if (byte0 == 31)
        catchFish();
}

int_t EntityFish::catchFish()
{
    EntityPlayer *angler = getAngler();
    if (angler == nullptr || worldObj == nullptr)
    {
        setEntityDead();
        return 0;
    }

    int_t rodDamage = 0;
    Entity *bobber = getBobber();
    if (bobber != nullptr)
    {
        double dx = angler->posX - posX;
        double dy = angler->posY - posY;
        double dz = angler->posZ - posZ;
        double distance = MathHelper::sqrt_double(dx * dx + dy * dy + dz * dz);
        bobber->motionX += dx * 0.1;
        bobber->motionY += dy * 0.1 + (double)MathHelper::sqrt_double(distance) * 0.08;
        bobber->motionZ += dz * 0.1;
        rodDamage = 3;
    }
    else if (ticksCatchable > 0)
    {
        EntityItem *caughtFish = new EntityItem(worldObj, posX, posY, posZ, new ItemStack(Item::fishRaw));
        double dx = angler->posX - posX;
        double dy = angler->posY - posY;
        double dz = angler->posZ - posZ;
        double distance = MathHelper::sqrt_double(dx * dx + dy * dy + dz * dz);
        caughtFish->motionX = dx * 0.1;
        caughtFish->motionY = dy * 0.1 + (double)MathHelper::sqrt_double(distance) * 0.08;
        caughtFish->motionZ = dz * 0.1;
        if (!worldObj->entityJoinedWorld(caughtFish))
            delete caughtFish;
        angler->addStat(StatList::fishCaughtStat, 1);
        rodDamage = 1;
    }
    if (inGround)
        rodDamage = 2;
    setEntityDead();
    return rodDamage;
}

void EntityFish::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
    fishX = d;
    fishY = d1;
    fishZ = d2;
    fishYaw = f;
    fishPitch = f1;
    fishPosRotationIncrements = i;
    motionX = velocityX;
    motionY = velocityY;
    motionZ = velocityZ;
}

void EntityFish::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
    nbttagcompound->setShort("xTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(xTile)));
    nbttagcompound->setShort("yTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(yTile)));
    nbttagcompound->setShort("zTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(zTile)));
    nbttagcompound->setByte("inTile", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(inTile)));
    nbttagcompound->setByte("shake", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(shake)));
    nbttagcompound->setByte("inGround", (byte_t)(inGround ? 1 : 0));
}

void EntityFish::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
    xTile = nbttagcompound->getShort("xTile");
    yTile = nbttagcompound->getShort("yTile");
    zTile = nbttagcompound->getShort("zTile");
    inTile = nbttagcompound->getByte("inTile") & 0xff;
    shake = nbttagcompound->getByte("shake") & 0xff;
    inGround = nbttagcompound->getByte("inGround") == 1;
}

bool EntityFish::canBeCollidedWith()
{
    return false;
}

float EntityFish::getShadowSize()
{
    return 0.0f;
}

bool EntityFish::isInRangeToRenderDist(double d)
{
    double range = boundingBox->getAverageEdgeLength() * 4.0;
    range *= 64.0;
    return d < range * range;
}
