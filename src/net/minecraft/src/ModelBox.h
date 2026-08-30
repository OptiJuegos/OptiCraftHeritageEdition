#pragma once

#include <string>

class ModelRenderer;
class PositionTextureVertex;
class Tessellator;
class TexturedQuad;

// net.minecraft.src.ModelBox
class ModelBox
{
public:
    ModelBox(ModelRenderer* renderer, int textureOffsetX, int textureOffsetY,
             float x, float y, float z, int sizeX, int sizeY, int sizeZ, float scale);
    ~ModelBox();

    void render(Tessellator* tessellator, float scale) const;
    void emitInto(Tessellator* tessellator, float scale) const;
    ModelBox* setBoxName(const std::string& name);
    TexturedQuad* getQuad(int index) const;

    const float posX1;
    const float posY1;
    const float posZ1;
    const float posX2;
    const float posY2;
    const float posZ2;
    std::string boxName;

private:
    PositionTextureVertex* vertexPositions[8];
    TexturedQuad* quadList[6];
};
