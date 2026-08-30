#pragma once

#include <string>
#include <vector>

#include "java/Type.h"
#include "platform/PlatformConfig.h"

class ModelBase;
class ModelBox;
class Tessellator;

// net.minecraft.src.ModelRenderer
class ModelRenderer
{
public:
    ModelRenderer(int_t i, int_t j);
    explicit ModelRenderer(ModelBase* baseModel);
    ModelRenderer(ModelBase* baseModel, int_t i, int_t j);
    ModelRenderer(ModelBase* baseModel, const std::string& boxName);
    ~ModelRenderer();

    ModelRenderer* addBox(float f, float f1, float f2, int_t i, int_t j, int_t k);
    ModelRenderer* addBox(float f, float f1, float f2, int_t i, int_t j, int_t k, float f3);
    ModelRenderer* addBox(const std::string& name, float f, float f1, float f2, int_t i, int_t j, int_t k);
    void addChild(ModelRenderer* child);
    ModelRenderer* setTextureOffset(int_t x, int_t y);
    ModelRenderer* setTextureSize(int_t width, int_t height);
    void setRotationPoint(float f, float f1, float f2);
    void render(float f);
    void renderWithRotation(float f);
    void postRender(float f);

private:
    void init(ModelBase* baseModel, const std::string& boxName, int_t textureX, int_t textureY);
    void invalidateCompiledGeometry();
    void compileDisplayList(float f);
    void drawGeometry(float f);
    void renderChildren(float f);
#if PLATFORM_MODEL_IMMEDIATE || PLATFORM_MODEL_PERSISTENT_MESH
    void renderImmediate(float f);
#endif

    int_t textureOffsetX;
    int_t textureOffsetY;
    bool compiled;
    int_t displayList;
    ModelBase* baseModel;
    std::string boxName;
    std::vector<ModelBox*> cubeList;
    std::vector<ModelRenderer*> childModels;

public:
    float textureWidth;
    float textureHeight;
    float rotationPointX;
    float rotationPointY;
    float rotationPointZ;
    float rotateAngleX;
    float rotateAngleY;
    float rotateAngleZ;
    bool mirror;
    bool showModel;
    bool field_1402_i;
};
