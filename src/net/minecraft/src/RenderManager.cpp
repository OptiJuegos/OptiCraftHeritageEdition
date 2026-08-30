#include "RenderManager.h"
#include "Render.h"
#include "World.h"
#include "Block.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "ItemRenderer.h"
#include "GameSettings.h"
#include "EntityLiving.h"
#include "EntityMob.h"
#include "EntityAnimal.h"
#include "EntityCreature.h"
#include "EntityFlying.h"
#include "EntityWaterMob.h"
#include "EntityWeatherEffect.h"
#include "Entity.h"
#include "MathHelper.h"

#include "EntitySpider.h"
#include "RenderSpider.h"
#include "EntityPig.h"
#include "RenderPig.h"
#include "ModelPig.h"
#include "EntitySheep.h"
#include "RenderSheep.h"
#include "ModelSheep2.h"
#include "ModelSheep1.h"
#include "EntityCow.h"
#include "RenderCow.h"
#include "ModelCow.h"
#include "EntityWolf.h"
#include "RenderWolf.h"
#include "ModelWolf.h"
#include "EntityOcelot.h"
#include "RenderOcelot.h"
#include "ModelOcelot.h"
#include "EntityChicken.h"
#include "RenderChicken.h"
#include "ModelChicken.h"
#include "EntityCreeper.h"
#include "RenderCreeper.h"
#include "EntitySkeleton.h"
#include "RenderBiped.h"
#include "ModelSkeleton.h"
#include "EntityZombie.h"
#include "EntityPigZombie.h"
#include "ModelZombie.h"
#include "EntitySlime.h"
#include "RenderSlime.h"
#include "ModelSlime.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "EntityClientPlayerMP.h"
#include "EntityOtherPlayerMP.h"
#include "RenderPlayer.h"
#include "EntityGiantZombie.h"
#include "RenderGiantZombie.h"
#include "EntityGhast.h"
#include "RenderGhast.h"
#include "EntitySquid.h"
#include "RenderSquid.h"
#include "ModelSquid.h"
#include "ModelBiped.h"
#include "EntityPainting.h"
#include "RenderPainting.h"
#include "EntityArrow.h"
#include "RenderArrow.h"
#include "EntitySnowball.h"
#include "RenderSnowball.h"
#include "EntityEgg.h"
#include "Item.h"
#include "EntityFireball.h"
#include "RenderFireball.h"
#include "EntityItem.h"
#include "RenderItem.h"
#include "EntityTNTPrimed.h"
#include "RenderTNTPrimed.h"
#include "EntityFallingSand.h"
#include "RenderFallingSand.h"
#include "EntityMinecart.h"
#include "RenderMinecart.h"
#include "EntityBoat.h"
#include "RenderBoat.h"
#include "EntityFish.h"
#include "RenderFish.h"
#include "EntityLightningBolt.h"
#include "RenderLightningBolt.h"
#include "RenderEntity.h"
#include "EntityBlaze.h"
#include "RenderBlaze.h"
#include "EntityMagmaCube.h"
#include "RenderMagmaCube.h"
#include "EntitySilverfish.h"
#include "RenderSilverfish.h"
#include "EntitySnowman.h"
#include "RenderSnowMan.h"
#include "EntityVillager.h"
#include "RenderVillager.h"
#include "EntityMooshroom.h"
#include "RenderMooshroom.h"
#include "EntityEnderCrystal.h"
#include "RenderEnderCrystal.h"
#include "EntityGolem.h"
#include "EntityAgeable.h"
#include "EntityEnderman.h"
#include "EntityIronGolem.h"
#include "EntityXPOrb.h"
#include "RenderEnderman.h"
#include "RenderIronGolem.h"
#include "RenderXPOrb.h"
#include "EntityDragonBase.h"
#include "EntityDragon.h"
#include "RenderDragon.h"
#include "EntityCaveSpider.h"
#include "EntityEnderPearl.h"
#include "EntityEnderEye.h"
#include "EntityPotion.h"
#include "EntityExpBottle.h"
#include "EntitySmallFireball.h"
#include "OpenGlHelper.h"
#include <unordered_set>

RenderManager* RenderManager::instance = nullptr;
double RenderManager::renderPosX = 0.0;
double RenderManager::renderPosY = 0.0;
double RenderManager::renderPosZ = 0.0;

RenderManager::RenderManager() {
    instance      = this;
    renderEngine  = nullptr;
    fontRenderer  = nullptr;
    worldObj      = nullptr;
    livingPlayer  = nullptr;
    itemRenderer  = nullptr;
    options       = nullptr;
    playerViewY   = 0.0f;
    playerViewX   = 0.0f;
    field_1222_l  = 0.0;
    field_1221_m  = 0.0;
    field_1220_n  = 0.0;
    registerRenderers();
}

RenderManager::~RenderManager()
{
    std::unordered_set<Render *> uniqueRenderers;
    for (const auto &pair : entityRenderMap)
    {
        if (pair.second != nullptr)
            uniqueRenderers.insert(pair.second);
    }
    for (Render *render : uniqueRenderers)
        delete render;
    entityRenderMap.clear();
    delete itemRenderer;
    itemRenderer = nullptr;
    if (instance == this)
        instance = nullptr;
}


void RenderManager::registerRenderers() {
    entityRenderMap[std::type_index(typeid(EntitySpider))] = new RenderSpider();
    entityRenderMap[std::type_index(typeid(EntityCaveSpider))] = new RenderSpider();
    entityRenderMap[std::type_index(typeid(EntityPig))] = new RenderPig(new ModelPig(), new ModelPig(0.5f), 0.7f);
    entityRenderMap[std::type_index(typeid(EntitySheep))] = new RenderSheep(new ModelSheep2(), new ModelSheep1(), 0.7f);
    entityRenderMap[std::type_index(typeid(EntityCow))] = new RenderCow(new ModelCow(), 0.7f);
    entityRenderMap[std::type_index(typeid(EntityWolf))] = new RenderWolf(new ModelWolf(), 0.5f);
    entityRenderMap[std::type_index(typeid(EntityOcelot))] = new RenderOcelot(new ModelOcelot(), 0.4f);
    entityRenderMap[std::type_index(typeid(EntityChicken))] = new RenderChicken(new ModelChicken(), 0.3f);
    entityRenderMap[std::type_index(typeid(EntityCreeper))] = new RenderCreeper();
    entityRenderMap[std::type_index(typeid(EntitySkeleton))] = new RenderBiped(new ModelSkeleton(), 0.5f);
    Render *zombieRenderer = new RenderBiped(new ModelZombie(), 0.5f);
    entityRenderMap[std::type_index(typeid(EntityZombie))] = zombieRenderer;
    entityRenderMap[std::type_index(typeid(EntityPigZombie))] = zombieRenderer;
    entityRenderMap[std::type_index(typeid(EntitySlime))] = new RenderSlime(new ModelSlime(16), new ModelSlime(0), 0.25f);
    entityRenderMap[std::type_index(typeid(EntityMagmaCube))] = new RenderMagmaCube();
    entityRenderMap[std::type_index(typeid(EntityBlaze))] = new RenderBlaze();
    entityRenderMap[std::type_index(typeid(EntitySilverfish))] = new RenderSilverfish();
    entityRenderMap[std::type_index(typeid(EntitySnowman))] = new RenderSnowMan();
    entityRenderMap[std::type_index(typeid(EntityVillager))] = new RenderVillager();
    entityRenderMap[std::type_index(typeid(EntityMooshroom))] = new RenderMooshroom(new ModelCow(), 0.7f);
    entityRenderMap[std::type_index(typeid(EntityEnderman))] = new RenderEnderman();
    entityRenderMap[std::type_index(typeid(EntityIronGolem))] = new RenderIronGolem();
    entityRenderMap[std::type_index(typeid(EntityXPOrb))] = new RenderXPOrb();
    entityRenderMap[std::type_index(typeid(EntityDragon))] = new RenderDragon();
    RenderPlayer *renderPlayer = new RenderPlayer();
    entityRenderMap[std::type_index(typeid(EntityPlayer))]         = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityPlayerSP))]       = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityClientPlayerMP))] = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityOtherPlayerMP))]  = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityGiantZombie))] = new RenderGiantZombie(new ModelZombie(), 0.5f, 6.0f);
    entityRenderMap[std::type_index(typeid(EntityGhast))] = new RenderGhast();
    entityRenderMap[std::type_index(typeid(EntitySquid))] = new RenderSquid(new ModelSquid(), 0.7f);
    entityRenderMap[std::type_index(typeid(EntityLiving))] = new RenderLiving(new ModelBiped(), 0.5f);
    entityRenderMap[std::type_index(typeid(Entity))] = new RenderEntity();
    entityRenderMap[std::type_index(typeid(EntityPainting))] = new RenderPainting();
    entityRenderMap[std::type_index(typeid(EntityArrow))] = new RenderArrow();
    entityRenderMap[std::type_index(typeid(EntitySnowball))] = new RenderSnowball(Item::snowball->getIconFromDamage(0));
    entityRenderMap[std::type_index(typeid(EntityEnderPearl))] = new RenderSnowball(Item::enderPearl->getIconFromDamage(0));
    entityRenderMap[std::type_index(typeid(EntityEnderEye))] = new RenderSnowball(Item::eyeOfEnder->getIconFromDamage(0));
    entityRenderMap[std::type_index(typeid(EntityEgg))] = new RenderSnowball(Item::egg->getIconFromDamage(0));
    entityRenderMap[std::type_index(typeid(EntityPotion))] = new RenderSnowball(154);
    entityRenderMap[std::type_index(typeid(EntityExpBottle))] = new RenderSnowball(Item::expBottle->getIconFromDamage(0));
    entityRenderMap[std::type_index(typeid(EntityFireball))] = new RenderFireball(2.0f);
    entityRenderMap[std::type_index(typeid(EntitySmallFireball))] = new RenderFireball(0.5f);
    entityRenderMap[std::type_index(typeid(EntityItem))] = new RenderItem();
    entityRenderMap[std::type_index(typeid(EntityTNTPrimed))] = new RenderTNTPrimed();
    entityRenderMap[std::type_index(typeid(EntityFallingSand))] = new RenderFallingSand();
    entityRenderMap[std::type_index(typeid(EntityMinecart))] = new RenderMinecart();
    entityRenderMap[std::type_index(typeid(EntityBoat))] = new RenderBoat();
    entityRenderMap[std::type_index(typeid(EntityFish))] = new RenderFish();
    entityRenderMap[std::type_index(typeid(EntityLightningBolt))] = new RenderLightningBolt();
    entityRenderMap[std::type_index(typeid(EntityEnderCrystal))] = new RenderEnderCrystal();

    for (auto& pair : entityRenderMap) {
        pair.second->setRenderManager(this);
    }
}

namespace
{
std::type_index parentEntityType(std::type_index type)
{
#define PARENT_OF(Derived, Base) if (type == std::type_index(typeid(Derived))) return std::type_index(typeid(Base))
    PARENT_OF(EntityCaveSpider, EntitySpider);
    PARENT_OF(EntitySpider, EntityMob);
    PARENT_OF(EntityPig, EntityAnimal);
    PARENT_OF(EntitySheep, EntityAnimal);
    PARENT_OF(EntityCow, EntityAnimal);
    PARENT_OF(EntityWolf, EntityAnimal);
    PARENT_OF(EntityOcelot, EntityAnimal);
    PARENT_OF(EntityChicken, EntityAnimal);
    PARENT_OF(EntityCreeper, EntityMob);
    PARENT_OF(EntitySkeleton, EntityMob);
    PARENT_OF(EntityPigZombie, EntityZombie);
    PARENT_OF(EntityZombie, EntityMob);
    PARENT_OF(EntityGiantZombie, EntityMob);
    PARENT_OF(EntityMob, EntityCreature);
    PARENT_OF(EntityAnimal, EntityCreature);
    PARENT_OF(EntityWaterMob, EntityCreature);
    PARENT_OF(EntityCreature, EntityLiving);
    PARENT_OF(EntityGhast, EntityFlying);
    PARENT_OF(EntityFlying, EntityLiving);
    PARENT_OF(EntitySlime, EntityLiving);
    PARENT_OF(EntityMagmaCube, EntitySlime);
    PARENT_OF(EntityBlaze, EntityMob);
    PARENT_OF(EntitySilverfish, EntityMob);
    PARENT_OF(EntitySnowman, EntityGolem);
    PARENT_OF(EntityIronGolem, EntityGolem);
    PARENT_OF(EntityGolem, EntityCreature);
    PARENT_OF(EntityVillager, EntityAgeable);
    PARENT_OF(EntityAgeable, EntityCreature);
    PARENT_OF(EntityMooshroom, EntityCow);
    PARENT_OF(EntityEnderman, EntityMob);
    PARENT_OF(EntityDragon, EntityDragonBase);
    PARENT_OF(EntityDragonBase, EntityLiving);
    PARENT_OF(EntityXPOrb, Entity);
    PARENT_OF(EntityEnderCrystal, Entity);
    PARENT_OF(EntityPlayer, EntityLiving);
    PARENT_OF(EntitySquid, EntityWaterMob);
    PARENT_OF(EntityLiving, Entity);
    PARENT_OF(EntityPainting, Entity);
    PARENT_OF(EntityArrow, Entity);
    PARENT_OF(EntitySnowball, Entity);
    PARENT_OF(EntityEnderPearl, Entity);
    PARENT_OF(EntityEnderEye, Entity);
    PARENT_OF(EntityEgg, Entity);
    PARENT_OF(EntityPotion, Entity);
    PARENT_OF(EntityExpBottle, Entity);
    PARENT_OF(EntitySmallFireball, EntityFireball);
    PARENT_OF(EntityFireball, Entity);
    PARENT_OF(EntityItem, Entity);
    PARENT_OF(EntityTNTPrimed, Entity);
    PARENT_OF(EntityFallingSand, Entity);
    PARENT_OF(EntityMinecart, Entity);
    PARENT_OF(EntityBoat, Entity);
    PARENT_OF(EntityFish, Entity);
    PARENT_OF(EntityLightningBolt, EntityWeatherEffect);
    PARENT_OF(EntityWeatherEffect, Entity);
#undef PARENT_OF
    return std::type_index(typeid(void));
}
}

Render* RenderManager::getEntityClassRenderObject(std::type_index classType) {
    auto it = entityRenderMap.find(classType);
    if (it != entityRenderMap.end()) return it->second;
    if (classType == std::type_index(typeid(Entity))) return nullptr;

    const std::type_index parent = parentEntityType(classType);
    if (parent == std::type_index(typeid(void))) return nullptr;
    Render *render = getEntityClassRenderObject(parent);
    if (render != nullptr) entityRenderMap[classType] = render; // Java caches inherited resolution.
    return render;
}

Render* RenderManager::getEntityRenderObject(Entity* entity) {
    if (entity == nullptr) return nullptr;
    const std::type_index type(typeid(*entity));
    auto exact = entityRenderMap.find(type);
    if (exact != entityRenderMap.end()) return exact->second;

    // Java recursively walks the superclass chain and caches the first renderer
    // found. Reproduce that behavior for subclasses not explicitly registered.
    Render *resolved = nullptr;
#define RESOLVE_BASE(T) if (resolved == nullptr && dynamic_cast<T*>(entity) != nullptr) resolved = entityRenderMap[std::type_index(typeid(T))]
    RESOLVE_BASE(EntityCaveSpider);
    RESOLVE_BASE(EntitySpider);
    RESOLVE_BASE(EntityPig);
    RESOLVE_BASE(EntitySheep);
    RESOLVE_BASE(EntityCow);
    RESOLVE_BASE(EntityWolf);
    RESOLVE_BASE(EntityOcelot);
    RESOLVE_BASE(EntityChicken);
    RESOLVE_BASE(EntityCreeper);
    RESOLVE_BASE(EntitySkeleton);
    RESOLVE_BASE(EntityPigZombie);
    RESOLVE_BASE(EntityZombie);
    RESOLVE_BASE(EntitySlime);
    RESOLVE_BASE(EntityMagmaCube);
    RESOLVE_BASE(EntityBlaze);
    RESOLVE_BASE(EntitySilverfish);
    RESOLVE_BASE(EntitySnowman);
    RESOLVE_BASE(EntityIronGolem);
    RESOLVE_BASE(EntityVillager);
    RESOLVE_BASE(EntityMooshroom);
    RESOLVE_BASE(EntityEnderman);
    RESOLVE_BASE(EntityDragon);
    RESOLVE_BASE(EntityXPOrb);
    RESOLVE_BASE(EntityEnderCrystal);
    RESOLVE_BASE(EntityPlayer);
    RESOLVE_BASE(EntityGiantZombie);
    RESOLVE_BASE(EntityGhast);
    RESOLVE_BASE(EntitySquid);
    RESOLVE_BASE(EntityPainting);
    RESOLVE_BASE(EntityArrow);
    RESOLVE_BASE(EntitySnowball);
    RESOLVE_BASE(EntityEnderPearl);
    RESOLVE_BASE(EntityEnderEye);
    RESOLVE_BASE(EntityEgg);
    RESOLVE_BASE(EntityPotion);
    RESOLVE_BASE(EntityExpBottle);
    RESOLVE_BASE(EntitySmallFireball);
    RESOLVE_BASE(EntityFireball);
    RESOLVE_BASE(EntityItem);
    RESOLVE_BASE(EntityTNTPrimed);
    RESOLVE_BASE(EntityFallingSand);
    RESOLVE_BASE(EntityMinecart);
    RESOLVE_BASE(EntityBoat);
    RESOLVE_BASE(EntityFish);
    RESOLVE_BASE(EntityLightningBolt);
    RESOLVE_BASE(EntityLiving);
    RESOLVE_BASE(Entity);
#undef RESOLVE_BASE
    entityRenderMap[type] = resolved;
    return resolved;
}

void RenderManager::cacheActiveRenderInfo(World* world, RenderEngine* renderengine, FontRenderer* fontrenderer, EntityLiving* entityliving, GameSettings* gamesettings, float f) {
    worldObj = world;
    renderEngine = renderengine;
    options = gamesettings;
    livingPlayer = entityliving;
    fontRenderer = fontrenderer;
    if (entityliving->isPlayerSleeping()) {
        int i = world->getBlockId(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));
        if (i == Block::blockBed->blockID) {
            int j = world->getBlockMetadata(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));
            int k = j & 3;
            playerViewY = k * 90 + 180;
            playerViewX = 0.0f;
        }
    } else {
        playerViewY = entityliving->prevRotationYaw + (entityliving->rotationYaw - entityliving->prevRotationYaw) * f;
        playerViewX = entityliving->prevRotationPitch + (entityliving->rotationPitch - entityliving->prevRotationPitch) * f;
    }
    if (gamesettings->thirdPersonView == 2) {
        playerViewY += 180.0f;
    }
    field_1222_l = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
    field_1221_m = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
    field_1220_n = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;
}

void RenderManager::renderEntity(Entity* entity, float f) {
    double d = entity->lastTickPosX + (entity->posX - entity->lastTickPosX) * (double)f;
    double d1 = entity->lastTickPosY + (entity->posY - entity->lastTickPosY) * (double)f;
    double d2 = entity->lastTickPosZ + (entity->posZ - entity->lastTickPosZ) * (double)f;
    float f1 = entity->prevRotationYaw + (entity->rotationYaw - entity->prevRotationYaw) * f;
    int_t brightness = entity->getBrightnessForRender(f);
    if (entity->isBurning())
        brightness = 0xF000F0;
    int_t lightU = brightness % 65536;
    int_t lightV = brightness / 65536;
    OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, (float)lightU, (float)lightV);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderEntityWithPosYaw(entity, d - renderPosX, d1 - renderPosY, d2 - renderPosZ, f1, f);
}

void RenderManager::renderEntityWithPosYaw(Entity* entity, double d, double d1, double d2, float f, float f1) {
    Render* render = getEntityRenderObject(entity);
    if (render != nullptr) {
        render->doRender(entity, d, d1, d2, f, f1);
        render->doRenderShadowAndFire(entity, d, d1, d2, f, f1);
    }
}

void RenderManager::setWorld(World* world) {
    worldObj = world;
}

double RenderManager::getDistanceToCamera(double d, double d1, double d2) {
    double d3 = d - field_1222_l;
    double d4 = d1 - field_1221_m;
    double d5 = d2 - field_1220_n;
    return d3 * d3 + d4 * d4 + d5 * d5;
}

FontRenderer* RenderManager::getFontRenderer() {
    return fontRenderer;
}
