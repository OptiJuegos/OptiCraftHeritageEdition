#include "EntityDragon.h"

#include <cmath>
#include <limits>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockEndPortal.h"
#include "DamageSource.h"
#include "DataWatcher.h"
#include "EntityDragonPart.h"
#include "EntityEnderCrystal.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityXPOrb.h"
#include "MathHelper.h"
#include "java/Math.h"
#include "Vec3D.h"
#include "World.h"

namespace
{
constexpr float PI_F = 3.14159265358979323846f;
}

EntityDragon::EntityDragon(World *world)
    : EntityDragonBase(world, 200),
      targetX(0.0), targetY(100.0), targetZ(0.0), field_40162_d{}, field_40164_e(-1),
      dragonPartHead(new EntityDragonPart(this, "head", 6.0f, 6.0f)),
      dragonPartBody(new EntityDragonPart(this, "body", 8.0f, 8.0f)),
      dragonPartTail1(new EntityDragonPart(this, "tail", 4.0f, 4.0f)),
      dragonPartTail2(new EntityDragonPart(this, "tail", 4.0f, 4.0f)),
      dragonPartTail3(new EntityDragonPart(this, "tail", 4.0f, 4.0f)),
      dragonPartWing1(new EntityDragonPart(this, "wing", 4.0f, 4.0f)),
      dragonPartWing2(new EntityDragonPart(this, "wing", 4.0f, 4.0f)),
      field_40173_aw(0.0f), field_40172_ax(0.0f), field_40163_ay(false), field_40161_az(false),
      field_40178_aA(0), healingEnderCrystal(nullptr), targetEntityId(-1), healingCrystalEntityId(-1)
{
    ensureEntityInit();
    setHealth(dragonMaxHealth);
    texture = "/mob/enderdragon/ender.png";
    setSize(16.0f, 8.0f);
    noClip = true;
    immuneToFire = true;
    ignoreFrustumCheck = true;
}

EntityDragon::~EntityDragon()
{
    delete dragonPartHead;
    delete dragonPartBody;
    delete dragonPartTail1;
    delete dragonPartTail2;
    delete dragonPartTail3;
    delete dragonPartWing1;
    delete dragonPartWing2;
}

void EntityDragon::entityInit()
{
    EntityLiving::entityInit();
    if (!dataWatcher->hasObject(16))
        dataWatcher->addObject(16, static_cast<int_t>(0));
}

std::array<double, 3> EntityDragon::func_40160_a(int_t offset, float partialTick) const
{
    if (health <= 0)
        partialTick = 0.0f;

    partialTick = 1.0f - partialTick;
    const int_t current = (field_40164_e - offset) & 63;
    const int_t previous = (field_40164_e - offset - 1) & 63;

    std::array<double, 3> result{};
    double yaw = field_40162_d[current][0];
    double deltaYaw = field_40162_d[previous][0] - yaw;
    while (deltaYaw < -180.0)
        deltaYaw += 360.0;
    while (deltaYaw >= 180.0)
        deltaYaw -= 360.0;

    result[0] = yaw + deltaYaw * static_cast<double>(partialTick);
    const double y = field_40162_d[current][1];
    const double deltaY = field_40162_d[previous][1] - y;
    result[1] = y + deltaY * static_cast<double>(partialTick);
    result[2] = field_40162_d[current][2] +
                (field_40162_d[previous][2] - field_40162_d[current][2]) * static_cast<double>(partialTick);
    return result;
}

Entity *EntityDragon::resolveTarget()
{
    if (targetEntityId < 0 || worldObj == nullptr)
        return nullptr;
    Entity *entity = worldObj->getEntityByID(targetEntityId);
    if (entity == nullptr || entity->isDead)
    {
        targetEntityId = -1;
        return nullptr;
    }
    return entity;
}

EntityEnderCrystal *EntityDragon::resolveHealingCrystal()
{
    if (healingCrystalEntityId < 0 || worldObj == nullptr)
    {
        healingEnderCrystal = nullptr;
        return nullptr;
    }

    Entity *entity = worldObj->getEntityByID(healingCrystalEntityId);
    EntityEnderCrystal *crystal = dynamic_cast<EntityEnderCrystal *>(entity);
    if (crystal == nullptr)
        healingEnderCrystal = nullptr;
    else
        healingEnderCrystal = crystal;
    return healingEnderCrystal;
}

EntityEnderCrystal *EntityDragon::getHealingEnderCrystal()
{
    return resolveHealingCrystal();
}

void EntityDragon::onLivingUpdate()
{
    field_40173_aw = field_40172_ax;
    if (!worldObj->multiplayerWorld)
        dataWatcher->updateObject(16, health);

    if (health <= 0)
    {
        const float x = (rand.nextFloat() - 0.5f) * 8.0f;
        const float y = (rand.nextFloat() - 0.5f) * 4.0f;
        const float z = (rand.nextFloat() - 0.5f) * 8.0f;
        worldObj->spawnParticle("largeexplode", posX + static_cast<double>(x),
                                posY + 2.0 + static_cast<double>(y),
                                posZ + static_cast<double>(z), 0.0, 0.0, 0.0);
        return;
    }

    updateDragonEnderCrystal();
    float flapSpeed = 0.2f / (MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ) * 10.0f + 1.0f);
    flapSpeed *= static_cast<float>(JavaMath::pow(2.0, motionY));
    field_40172_ax += field_40161_az ? flapSpeed * 0.5f : flapSpeed;

    while (rotationYaw >= 180.0f)
        rotationYaw -= 360.0f;
    while (rotationYaw < -180.0f)
        rotationYaw += 360.0f;

    if (field_40164_e < 0)
    {
        for (auto &entry : field_40162_d)
        {
            entry[0] = static_cast<double>(rotationYaw);
            entry[1] = posY;
        }
    }

    if (++field_40164_e == static_cast<int_t>(field_40162_d.size()))
        field_40164_e = 0;
    field_40162_d[field_40164_e][0] = static_cast<double>(rotationYaw);
    field_40162_d[field_40164_e][1] = posY;

    if (worldObj->multiplayerWorld)
    {
        if (newPosRotationIncrements > 0)
        {
            const double nextX = posX + (newPosX - posX) / static_cast<double>(newPosRotationIncrements);
            const double nextY = posY + (newPosY - posY) / static_cast<double>(newPosRotationIncrements);
            const double nextZ = posZ + (newPosZ - posZ) / static_cast<double>(newPosRotationIncrements);
            double yawDelta = newRotationYaw - static_cast<double>(rotationYaw);
            while (yawDelta < -180.0)
                yawDelta += 360.0;
            while (yawDelta >= 180.0)
                yawDelta -= 360.0;
            rotationYaw = static_cast<float>(static_cast<double>(rotationYaw) + yawDelta / static_cast<double>(newPosRotationIncrements));
            rotationPitch = static_cast<float>(static_cast<double>(rotationPitch) +
                (newRotationPitch - static_cast<double>(rotationPitch)) / static_cast<double>(newPosRotationIncrements));
            --newPosRotationIncrements;
            setPosition(nextX, nextY, nextZ);
            setRotation(rotationYaw, rotationPitch);
        }
    }
    else
    {
        double dx = targetX - posX;
        double dy = targetY - posY;
        double dz = targetZ - posZ;
        const double targetDistanceSq = dx * dx + dy * dy + dz * dz;
        Entity *target = resolveTarget();
        if (target != nullptr)
        {
            targetX = target->posX;
            targetZ = target->posZ;
            const double targetDx = targetX - posX;
            const double targetDz = targetZ - posZ;
            const double horizontalDistance = JavaMath::sqrt(targetDx * targetDx + targetDz * targetDz);
            double targetYOffset = static_cast<double>(0.4f) + horizontalDistance / 80.0 - 1.0;
            if (targetYOffset > 10.0)
                targetYOffset = 10.0;
            targetY = target->boundingBox->minY + targetYOffset;
        }
        else
        {
            targetX += rand.nextGaussian() * 2.0;
            targetZ += rand.nextGaussian() * 2.0;
        }

        if (field_40163_ay || targetDistanceSq < 100.0 || targetDistanceSq > 22500.0 ||
            isCollidedHorizontally || isCollidedVertically)
        {
            func_41006_aA();
        }

        const double horizontalDistance = static_cast<double>(MathHelper::sqrt_double(dx * dx + dz * dz));
        if (horizontalDistance != 0.0)
            dy /= horizontalDistance;
        const double maxVertical = static_cast<double>(0.6f);
        if (dy < -maxVertical)
            dy = -maxVertical;
        if (dy > maxVertical)
            dy = maxVertical;
        motionY += dy * static_cast<double>(0.1f);

        while (rotationYaw < -180.0f)
            rotationYaw += 360.0f;
        while (rotationYaw >= 180.0f)
            rotationYaw -= 360.0f;

        const double desiredYaw = 180.0 - JavaMath::atan2(dx, dz) * 180.0 / static_cast<double>(PI_F);
        double yawDelta = desiredYaw - static_cast<double>(rotationYaw);
        while (yawDelta < -180.0)
            yawDelta += 360.0;
        while (yawDelta >= 180.0)
            yawDelta -= 360.0;
        if (yawDelta > 50.0)
            yawDelta = 50.0;
        if (yawDelta < -50.0)
            yawDelta = -50.0;

        Vec3D *targetDirection = Vec3D::createVector(targetX - posX, targetY - posY, targetZ - posZ)->normalize();
        Vec3D *motionDirection = Vec3D::createVector(
            static_cast<double>(MathHelper::sin(rotationYaw * PI_F / 180.0f)), motionY,
            static_cast<double>(-MathHelper::cos(rotationYaw * PI_F / 180.0f)))->normalize();
        float facing = static_cast<float>(motionDirection->dotProduct(targetDirection) + 0.5) / 1.5f;
        if (facing < 0.0f)
            facing = 0.0f;

        randomYawVelocity *= 0.8f;
        const float horizontalSpeed = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ) + 1.0f;
        double speed = JavaMath::sqrt(motionX * motionX + motionZ * motionZ) + 1.0;
        if (speed > 40.0)
            speed = 40.0;
        randomYawVelocity = static_cast<float>(static_cast<double>(randomYawVelocity) +
            yawDelta * (static_cast<double>(0.7f) / speed / static_cast<double>(horizontalSpeed)));
        rotationYaw += randomYawVelocity * 0.1f;
        const float turnScale = static_cast<float>(2.0 / (speed + 1.0));
        const float acceleration = 0.06f;
        moveFlying(0.0f, -1.0f, acceleration * (facing * turnScale + (1.0f - turnScale)));
        if (field_40161_az)
            moveEntity(motionX * static_cast<double>(0.8f), motionY * static_cast<double>(0.8f), motionZ * static_cast<double>(0.8f));
        else
            moveEntity(motionX, motionY, motionZ);

        Vec3D *normalizedMotion = Vec3D::createVector(motionX, motionY, motionZ)->normalize();
        float damping = static_cast<float>(normalizedMotion->dotProduct(motionDirection) + 1.0) / 2.0f;
        damping = 0.8f + 0.15f * damping;
        motionX *= static_cast<double>(damping);
        motionZ *= static_cast<double>(damping);
        motionY *= static_cast<double>(0.91f);
    }

    renderYawOffset = rotationYaw;
    dragonPartHead->width = dragonPartHead->height = 3.0f;
    dragonPartTail1->width = dragonPartTail1->height = 2.0f;
    dragonPartTail2->width = dragonPartTail2->height = 2.0f;
    dragonPartTail3->width = dragonPartTail3->height = 2.0f;
    dragonPartBody->height = 3.0f;
    dragonPartBody->width = 5.0f;
    dragonPartWing1->height = 2.0f;
    dragonPartWing1->width = 4.0f;
    dragonPartWing2->height = 3.0f;
    dragonPartWing2->width = 4.0f;

    const float pitchOffset = static_cast<float>(func_40160_a(5, 1.0f)[1] - func_40160_a(10, 1.0f)[1]) *
                              10.0f / 180.0f * PI_F;
    const float cosPitch = MathHelper::cos(pitchOffset);
    const float negSinPitch = -MathHelper::sin(pitchOffset);
    const float yawRadians = rotationYaw * PI_F / 180.0f;
    const float sinYaw = MathHelper::sin(yawRadians);
    const float cosYaw = MathHelper::cos(yawRadians);

    dragonPartBody->onUpdate();
    dragonPartBody->setLocationAndAngles(posX + static_cast<double>(sinYaw * 0.5f), posY,
                                         posZ - static_cast<double>(cosYaw * 0.5f), 0.0f, 0.0f);
    dragonPartWing1->onUpdate();
    dragonPartWing1->setLocationAndAngles(posX + static_cast<double>(cosYaw * 4.5f), posY + 2.0,
                                          posZ + static_cast<double>(sinYaw * 4.5f), 0.0f, 0.0f);
    dragonPartWing2->onUpdate();
    dragonPartWing2->setLocationAndAngles(posX - static_cast<double>(cosYaw * 4.5f), posY + 2.0,
                                          posZ - static_cast<double>(sinYaw * 4.5f), 0.0f, 0.0f);

    if (!worldObj->multiplayerWorld)
        func_41007_az();

    if (!worldObj->multiplayerWorld && maxHurtTime == 0)
    {
        collideWithEntities(worldObj->getEntitiesWithinAABBExcludingEntity(
            this, dragonPartWing1->boundingBox->expand(4.0, 2.0, 4.0)->offset(0.0, -2.0, 0.0)));
        collideWithEntities(worldObj->getEntitiesWithinAABBExcludingEntity(
            this, dragonPartWing2->boundingBox->expand(4.0, 2.0, 4.0)->offset(0.0, -2.0, 0.0)));
        attackEntitiesInList(worldObj->getEntitiesWithinAABBExcludingEntity(
            this, dragonPartHead->boundingBox->expand(1.0, 1.0, 1.0)));
    }

    const std::array<double, 3> history5 = func_40160_a(5, 1.0f);
    const std::array<double, 3> history0 = func_40160_a(0, 1.0f);
    const float headSin = MathHelper::sin(rotationYaw * PI_F / 180.0f - randomYawVelocity * 0.01f);
    const float headCos = MathHelper::cos(rotationYaw * PI_F / 180.0f - randomYawVelocity * 0.01f);
    dragonPartHead->onUpdate();
    dragonPartHead->setLocationAndAngles(
        posX + static_cast<double>(headSin * 5.5f * cosPitch),
        posY + (history0[1] - history5[1]) + static_cast<double>(negSinPitch * 5.5f),
        posZ - static_cast<double>(headCos * 5.5f * cosPitch), 0.0f, 0.0f);

    EntityDragonPart *tailParts[3] = {dragonPartTail1, dragonPartTail2, dragonPartTail3};
    for (int_t i = 0; i < 3; ++i)
    {
        EntityDragonPart *part = tailParts[i];
        const std::array<double, 3> history = func_40160_a(12 + i * 2, 1.0f);
        const float tailYaw = rotationYaw * PI_F / 180.0f + simplifyAngle(history[0] - history5[0]) * PI_F / 180.0f;
        const float tailSin = MathHelper::sin(tailYaw);
        const float tailCos = MathHelper::cos(tailYaw);
        const float baseOffset = 1.5f;
        const float segmentOffset = static_cast<float>(i + 1) * 2.0f;
        part->onUpdate();
        part->setLocationAndAngles(
            posX - static_cast<double>((sinYaw * baseOffset + tailSin * segmentOffset) * cosPitch),
            posY + (history[1] - history5[1]) - static_cast<double>((segmentOffset + baseOffset) * negSinPitch) + 1.5,
            posZ + static_cast<double>((cosYaw * baseOffset + tailCos * segmentOffset) * cosPitch), 0.0f, 0.0f);
    }

    if (!worldObj->multiplayerWorld)
        field_40161_az = destroyBlocksInAABB(dragonPartHead->boundingBox) |
                         destroyBlocksInAABB(dragonPartBody->boundingBox);
}

void EntityDragon::updateDragonEnderCrystal()
{
    if (healingCrystalEntityId >= 0)
    {
        EntityEnderCrystal *crystal = resolveHealingCrystal();
        if (crystal == nullptr || crystal->isDead)
        {
            if (!worldObj->multiplayerWorld)
                attackEntityFromPart(dragonPartHead, DamageSource::explosion, 10);
            healingCrystalEntityId = -1;
            healingEnderCrystal = nullptr;
        }
        else if (ticksExisted % 10 == 0 && health < dragonMaxHealth)
        {
            setHealth(health + 1);
        }
    }

    if (rand.nextInt(10) == 0)
    {
        const double radius = 32.0;
        AxisAlignedBB *searchBox = boundingBox->expand(radius, radius, radius);
        const std::vector<Entity *> &entities = worldObj->getEntitiesWithinAABB(typeid(EntityEnderCrystal), searchBox);
        EntityEnderCrystal *closest = nullptr;
        double closestDistance = std::numeric_limits<double>::max();
        for (Entity *entity : entities)
        {
            EntityEnderCrystal *crystal = dynamic_cast<EntityEnderCrystal *>(entity);
            if (crystal == nullptr)
                continue;
            const double distance = crystal->getDistanceSqToEntity(this);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closest = crystal;
            }
        }
        healingEnderCrystal = closest;
        healingCrystalEntityId = closest != nullptr ? closest->entityId : -1;
    }
}

void EntityDragon::func_41007_az()
{
}

void EntityDragon::collideWithEntities(const std::vector<Entity *> &entities)
{
    const double centerX = (dragonPartBody->boundingBox->minX + dragonPartBody->boundingBox->maxX) / 2.0;
    const double centerZ = (dragonPartBody->boundingBox->minZ + dragonPartBody->boundingBox->maxZ) / 2.0;
    for (Entity *entity : entities)
    {
        if (entity == nullptr || !entity->isLiving())
            continue;
        const double dx = entity->posX - centerX;
        const double dz = entity->posZ - centerZ;
        const double distanceSq = dx * dx + dz * dz;
        if (distanceSq > 0.0)
            entity->addVelocity(dx / distanceSq * 4.0, static_cast<double>(0.2f), dz / distanceSq * 4.0);
    }
}

void EntityDragon::attackEntitiesInList(const std::vector<Entity *> &entities)
{
    const std::vector<Entity *> attackTargets(entities.begin(), entities.end());
    for (Entity *entity : attackTargets)
    {
        if (entity != nullptr && entity->isLiving())
            entity->attackEntityFrom(DamageSource::causeMobDamage(this), 10);
    }
}

void EntityDragon::func_41006_aA()
{
    field_40163_ay = false;
    if (rand.nextInt(2) == 0 && !worldObj->playerEntities.empty())
    {
        EntityPlayer *player = worldObj->playerEntities[rand.nextInt(static_cast<int_t>(worldObj->playerEntities.size()))];
        targetEntityId = player != nullptr ? player->entityId : -1;
        return;
    }

    bool farEnough;
    do
    {
        targetX = static_cast<double>(rand.nextFloat() * 120.0f - 60.0f);
        targetY = static_cast<double>(70.0f + rand.nextFloat() * 50.0f);
        targetZ = static_cast<double>(rand.nextFloat() * 120.0f - 60.0f);
        const double dx = posX - targetX;
        const double dy = posY - targetY;
        const double dz = posZ - targetZ;
        farEnough = dx * dx + dy * dy + dz * dz > 100.0;
    }
    while (!farEnough);
    targetEntityId = -1;
}

float EntityDragon::simplifyAngle(double angle) const
{
    while (angle >= 180.0)
        angle -= 360.0;
    while (angle < -180.0)
        angle += 360.0;
    return static_cast<float>(angle);
}

bool EntityDragon::destroyBlocksInAABB(AxisAlignedBB *box)
{
    const int_t minX = MathHelper::floor_double(box->minX);
    const int_t minY = MathHelper::floor_double(box->minY);
    const int_t minZ = MathHelper::floor_double(box->minZ);
    const int_t maxX = MathHelper::floor_double(box->maxX);
    const int_t maxY = MathHelper::floor_double(box->maxY);
    const int_t maxZ = MathHelper::floor_double(box->maxZ);
    bool hitProtectedBlock = false;
    bool destroyedBlock = false;

    for (int_t x = minX; x <= maxX; ++x)
    {
        for (int_t y = minY; y <= maxY; ++y)
        {
            for (int_t z = minZ; z <= maxZ; ++z)
            {
                const int_t blockId = worldObj->getBlockId(x, y, z);
                if (blockId == 0)
                    continue;
                if (blockId != Block::obsidian->blockID && blockId != Block::whiteStone->blockID &&
                    blockId != Block::bedrock->blockID)
                {
                    destroyedBlock = true;
                    worldObj->setBlockWithNotify(x, y, z, 0);
                }
                else
                {
                    hitProtectedBlock = true;
                }
            }
        }
    }

    if (destroyedBlock)
    {
        const double x = box->minX + (box->maxX - box->minX) * static_cast<double>(rand.nextFloat());
        const double y = box->minY + (box->maxY - box->minY) * static_cast<double>(rand.nextFloat());
        const double z = box->minZ + (box->maxZ - box->minZ) * static_cast<double>(rand.nextFloat());
        worldObj->spawnParticle("largeexplode", x, y, z, 0.0, 0.0, 0.0);
    }

    return hitProtectedBlock;
}

bool EntityDragon::attackEntityFromPart(EntityDragonPart *part, const DamageSource &source, int_t damage)
{
    if (part != dragonPartHead)
        damage = damage / 4 + 1;

    const float yaw = rotationYaw * PI_F / 180.0f;
    const float sinYaw = MathHelper::sin(yaw);
    const float cosYaw = MathHelper::cos(yaw);
    targetX = posX + static_cast<double>(sinYaw * 5.0f) + static_cast<double>((rand.nextFloat() - 0.5f) * 2.0f);
    targetY = posY + static_cast<double>(rand.nextFloat() * 3.0f) + 1.0;
    targetZ = posZ - static_cast<double>(cosYaw * 5.0f) + static_cast<double>((rand.nextFloat() - 0.5f) * 2.0f);
    targetEntityId = -1;

    if (dynamic_cast<EntityPlayer *>(source.getEntity()) != nullptr || source.getDamageType() == DamageSource::explosion.getDamageType())
        superAttackFrom(source, damage);
    return true;
}

void EntityDragon::onDeathUpdate()
{
    ++field_40178_aA;
    if (field_40178_aA >= 180 && field_40178_aA <= 200)
    {
        const float x = (rand.nextFloat() - 0.5f) * 8.0f;
        const float y = (rand.nextFloat() - 0.5f) * 4.0f;
        const float z = (rand.nextFloat() - 0.5f) * 8.0f;
        worldObj->spawnParticle("hugeexplosion", posX + static_cast<double>(x),
                                posY + 2.0 + static_cast<double>(y),
                                posZ + static_cast<double>(z), 0.0, 0.0, 0.0);
    }

    if (!worldObj->multiplayerWorld && field_40178_aA > 150 && field_40178_aA % 5 == 0)
    {
        int_t xp = 1000;
        while (xp > 0)
        {
            const int_t split = EntityXPOrb::getXPSplit(xp);
            xp -= split;
            EntityXPOrb *orb = new EntityXPOrb(worldObj, posX, posY, posZ, split);
            if (!worldObj->spawnEntityInWorld(orb))
                delete orb;
        }
    }

    moveEntity(0.0, static_cast<double>(0.1f), 0.0);
    rotationYaw += 20.0f;
    renderYawOffset = rotationYaw;

    if (field_40178_aA == 200)
    {
        if (!worldObj->multiplayerWorld)
        {
            int_t xp = 10000;
            while (xp > 0)
            {
                const int_t split = EntityXPOrb::getXPSplit(xp);
                xp -= split;
                EntityXPOrb *orb = new EntityXPOrb(worldObj, posX, posY, posZ, split);
                if (!worldObj->spawnEntityInWorld(orb))
                    delete orb;
            }
            createEnderPortal(MathHelper::floor_double(posX), MathHelper::floor_double(posZ));
        }
        onEntityDeath();
        setEntityDead();
    }
}

void EntityDragon::createEnderPortal(int_t centerX, int_t centerZ)
{
    const int_t portalY = 64;
    BlockEndPortal::bossDefeated = true;
    const int_t radius = 4;

    for (int_t y = portalY - 1; y <= portalY + 32; ++y)
    {
        for (int_t x = centerX - radius; x <= centerX + radius; ++x)
        {
            for (int_t z = centerZ - radius; z <= centerZ + radius; ++z)
            {
                const double dx = static_cast<double>(x - centerX);
                const double dz = static_cast<double>(z - centerZ);
                const double distance = static_cast<double>(MathHelper::sqrt_double(dx * dx + dz * dz));
                if (distance > static_cast<double>(radius) - 0.5)
                    continue;

                if (y < portalY)
                {
                    if (distance <= static_cast<double>(radius - 1) - 0.5)
                        worldObj->setBlockWithNotify(x, y, z, Block::bedrock->blockID);
                }
                else if (y > portalY)
                {
                    worldObj->setBlockWithNotify(x, y, z, 0);
                }
                else if (distance > static_cast<double>(radius - 1) - 0.5)
                {
                    worldObj->setBlockWithNotify(x, y, z, Block::bedrock->blockID);
                }
                else
                {
                    worldObj->setBlockWithNotify(x, y, z, Block::endPortal->blockID);
                }
            }
        }
    }

    worldObj->setBlockWithNotify(centerX, portalY, centerZ, Block::bedrock->blockID);
    worldObj->setBlockWithNotify(centerX, portalY + 1, centerZ, Block::bedrock->blockID);
    worldObj->setBlockWithNotify(centerX, portalY + 2, centerZ, Block::bedrock->blockID);
    worldObj->setBlockWithNotify(centerX - 1, portalY + 2, centerZ, Block::torchWood->blockID);
    worldObj->setBlockWithNotify(centerX + 1, portalY + 2, centerZ, Block::torchWood->blockID);
    worldObj->setBlockWithNotify(centerX, portalY + 2, centerZ - 1, Block::torchWood->blockID);
    worldObj->setBlockWithNotify(centerX, portalY + 2, centerZ + 1, Block::torchWood->blockID);
    worldObj->setBlockWithNotify(centerX, portalY + 3, centerZ, Block::bedrock->blockID);
    worldObj->setBlockWithNotify(centerX, portalY + 4, centerZ, Block::dragonEgg->blockID);
    BlockEndPortal::bossDefeated = false;
}

void EntityDragon::despawnEntity()
{
}

std::vector<Entity *> EntityDragon::getParts() const
{
    return {dragonPartHead, dragonPartBody, dragonPartTail1, dragonPartTail2,
            dragonPartTail3, dragonPartWing1, dragonPartWing2};
}

bool EntityDragon::canBeCollidedWith()
{
    return false;
}

int_t EntityDragon::func_41010_ax()
{
    return dataWatcher->getWatchableObjectInt(16);
}
