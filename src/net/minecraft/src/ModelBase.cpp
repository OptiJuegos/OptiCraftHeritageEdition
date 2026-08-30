#include "ModelBase.h"

ModelBase::ModelBase() :
    onGround(0.0f),
    isRiding(false),
    isChild(true),
    textureWidth(64),
    textureHeight(32)
{
}

void ModelBase::render(float f, float f1, float f2, float f3, float f4, float f5)
{
}


void ModelBase::render(Entity* entity, float f, float f1, float f2, float f3, float f4, float f5)
{
    (void)entity;
    render(f, f1, f2, f3, f4, f5);
}

void ModelBase::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
}

void ModelBase::setLivingAnimations(EntityLiving* entityliving, float f, float f1, float f2)
{
}

void ModelBase::setTextureOffset(const std::string& name, int x, int y)
{
    modelTextureMap.erase(name);
    modelTextureMap.emplace(name, TextureOffset(x, y));
}

const TextureOffset* ModelBase::getTextureOffset(const std::string& name) const
{
    const auto it = modelTextureMap.find(name);
    return it != modelTextureMap.end() ? &it->second : nullptr;
}
