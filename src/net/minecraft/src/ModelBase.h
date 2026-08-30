#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "TextureOffset.h"

class Entity;
class EntityLiving;
class ModelRenderer;

// net.minecraft.src.ModelBase
class ModelBase
{
public:
    ModelBase();
    virtual ~ModelBase() = default;

    virtual void render(float f, float f1, float f2, float f3, float f4, float f5);
    virtual void render(Entity* entity, float f, float f1, float f2, float f3, float f4, float f5);
    virtual void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5);
    virtual void setLivingAnimations(EntityLiving* entityliving, float f, float f1, float f2);

    void setTextureOffset(const std::string& name, int x, int y);
    const TextureOffset* getTextureOffset(const std::string& name) const;

    float onGround;
    bool isRiding;
    bool isChild;
    int textureWidth;
    int textureHeight;
    std::vector<ModelRenderer*> boxList;

private:
    std::unordered_map<std::string, TextureOffset> modelTextureMap;
};
