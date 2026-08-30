#pragma once

class EntityLiving;
class EntityPlayer;
class Vec3D;
class World;

// net.minecraft.src.ActiveRenderInfo
class ActiveRenderInfo
{
public:
    static void updateRenderInfo(EntityPlayer* player, bool reverseView);
    static Vec3D* projectViewFromEntity(EntityLiving* entity, double partialTick);
    static int getBlockIdAtEntityViewpoint(World* world, EntityLiving* entity, float partialTick);

    static float objectX;
    static float objectY;
    static float objectZ;
    static float rotationX;
    static float rotationXZ;
    static float rotationZ;
    static float rotationYZ;
    static float rotationXY;
};
