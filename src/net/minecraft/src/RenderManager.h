#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include <map>
#include <typeindex>
#include "java/Type.h"

class Render;
class World;
class RenderEngine;
class FontRenderer;
class EntityLiving;
class GameSettings;
class Entity;
class ItemRenderer;

class RenderManager {
public:
    static RenderManager* instance;

    RenderManager();
    ~RenderManager();

    Render* getEntityClassRenderObject(std::type_index classType);
    Render* getEntityRenderObject(Entity* entity);

    void cacheActiveRenderInfo(World* world, RenderEngine* renderengine, FontRenderer* fontrenderer, EntityLiving* entityliving, GameSettings* gamesettings, float f);
    void renderEntity(Entity* entity, float f);
    void renderEntityWithPosYaw(Entity* entity, double d, double d1, double d2, float f, float f1);
    void setWorld(World* world);
    double getDistanceToCamera(double d, double d1, double d2);
    FontRenderer* getFontRenderer();

    std::map<std::type_index, Render*> entityRenderMap;
    FontRenderer* fontRenderer;
    static double renderPosX;
    static double renderPosY;
    static double renderPosZ;
    RenderEngine* renderEngine;
    ItemRenderer* itemRenderer;
    World* worldObj;
    EntityLiving* livingPlayer;
    float playerViewY;
    float playerViewX;
    GameSettings* options;
    double field_1222_l;
    double field_1221_m;
    double field_1220_n;

private:
    void registerRenderers();
};

#endif