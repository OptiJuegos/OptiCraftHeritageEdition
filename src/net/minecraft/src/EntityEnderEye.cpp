#include "EntityEnderEye.h"

#include <cmath>

#include "EntityItem.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

EntityEnderEye::EntityEnderEye(World *world)
    : Entity(world)
{
    setSize(0.25f, 0.25f);
}

EntityEnderEye::EntityEnderEye(World *world, double x, double y, double z)
    : Entity(world)
{
    setSize(0.25f, 0.25f);
    setPosition(x, y, z);
    yOffset = 0.0f;
}

void EntityEnderEye::entityInit()
{
}

bool EntityEnderEye::isInRangeToRenderDist(double distance)
{
    double range = boundingBox->getAverageEdgeLength() * 4.0;
    range *= 64.0;
    return distance < range * range;
}

void EntityEnderEye::moveTowards(double x, int_t y, double z)
{
    const double dx = x - posX;
    const double dz = z - posZ;
    const float horizontal = MathHelper::sqrt_double(dx * dx + dz * dz);
    if (horizontal > 12.0f)
    {
        targetX = posX + dx / static_cast<double>(horizontal) * 12.0;
        targetZ = posZ + dz / static_cast<double>(horizontal) * 12.0;
        targetY = posY + 8.0;
    }
    else
    {
        targetX = x;
        targetY = static_cast<double>(y);
        targetZ = z;
    }

    despawnTimer = 0;
    shatterOrDrop = rand.nextInt(5) > 0;
}

void EntityEnderEye::setVelocity(double x, double y, double z)
{
    motionX = x;
    motionY = y;
    motionZ = z;
    if (prevRotationPitch == 0.0f && prevRotationYaw == 0.0f)
    {
        const float horizontal = MathHelper::sqrt_double(x * x + z * z);
        prevRotationYaw = rotationYaw = static_cast<float>(JavaMath::atan2(x, z) * 180.0 / static_cast<double>(3.1415927f));
        prevRotationPitch = rotationPitch = static_cast<float>(JavaMath::atan2(y, static_cast<double>(horizontal)) * 180.0 / static_cast<double>(3.1415927f));
    }
}

void EntityEnderEye::onUpdate()
{
    lastTickPosX = posX;
    lastTickPosY = posY;
    lastTickPosZ = posZ;
    Entity::onUpdate();

    posX += motionX;
    posY += motionY;
    posZ += motionZ;
    const float horizontalMotion = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
    rotationYaw = static_cast<float>(JavaMath::atan2(motionX, motionZ) * 180.0 / static_cast<double>(3.1415927f));
    rotationPitch = static_cast<float>(JavaMath::atan2(motionY, static_cast<double>(horizontalMotion)) * 180.0 / static_cast<double>(3.1415927f));

    while (rotationPitch - prevRotationPitch < -180.0f)
        prevRotationPitch -= 360.0f;
    while (rotationPitch - prevRotationPitch >= 180.0f)
        prevRotationPitch += 360.0f;
    while (rotationYaw - prevRotationYaw < -180.0f)
        prevRotationYaw -= 360.0f;
    while (rotationYaw - prevRotationYaw >= 180.0f)
        prevRotationYaw += 360.0f;

    rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
    rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;

    if (!worldObj->multiplayerWorld)
    {
        const double dx = targetX - posX;
        const double dz = targetZ - posZ;
        const float distance = static_cast<float>(JavaMath::sqrt(dx * dx + dz * dz));
        const float angle = static_cast<float>(JavaMath::atan2(dz, dx));
        double speed = static_cast<double>(horizontalMotion) + (static_cast<double>(distance) - static_cast<double>(horizontalMotion)) * 0.0025;
        if (distance < 1.0f)
        {
            speed *= 0.8;
            motionY *= 0.8;
        }

        motionX = JavaMath::cos(static_cast<double>(angle)) * speed;
        motionZ = JavaMath::sin(static_cast<double>(angle)) * speed;
        if (posY < targetY)
            motionY += (1.0 - motionY) * static_cast<double>(0.015f);
        else
            motionY += (-1.0 - motionY) * static_cast<double>(0.015f);
    }

    const float fraction = 0.25f;
    if (isInWater())
    {
        for (int_t i = 0; i < 4; ++i)
        {
            worldObj->spawnParticle("bubble", posX - motionX * static_cast<double>(fraction),
                                    posY - motionY * static_cast<double>(fraction),
                                    posZ - motionZ * static_cast<double>(fraction), motionX, motionY, motionZ);
        }
    }
    else
    {
        const double particleX = posX - motionX * static_cast<double>(fraction) + rand.nextDouble() * 0.6 - 0.3;
        const double particleZ = posZ - motionZ * static_cast<double>(fraction) + rand.nextDouble() * 0.6 - 0.3;
        worldObj->spawnParticle("portal", particleX,
                                posY - motionY * static_cast<double>(fraction) - 0.5, particleZ,
                                motionX, motionY, motionZ);
    }

    if (!worldObj->multiplayerWorld)
    {
        setPosition(posX, posY, posZ);
        ++despawnTimer;
        if (despawnTimer > 80)
        {
            setEntityDead();
            const int_t x = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posX));
            const int_t y = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posY));
            const int_t z = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posZ));
            if (shatterOrDrop)
            {
                EntityItem *item = new EntityItem(worldObj, posX, posY, posZ, new ItemStack(Item::eyeOfEnder));
                if (!worldObj->spawnEntityInWorld(item))
                    delete item;
            }
            else
            {
                worldObj->playAuxSFX(2003, x, y, z, 0);
            }
        }
    }
}

void EntityEnderEye::writeEntityToNBT(NBTTagCompound *)
{
}

void EntityEnderEye::readEntityFromNBT(NBTTagCompound *)
{
}

void EntityEnderEye::onCollideWithPlayer(EntityPlayer *)
{
}

float EntityEnderEye::getShadowSize()
{
    return 0.0f;
}

float EntityEnderEye::getEntityBrightness(float)
{
    return 1.0f;
}

int_t EntityEnderEye::getBrightnessForRender(float)
{
    return 15728880;
}

bool EntityEnderEye::canAttackWithItem()
{
    return false;
}
