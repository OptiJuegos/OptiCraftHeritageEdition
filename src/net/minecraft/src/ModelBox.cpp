#include "ModelBox.h"

#include "ModelRenderer.h"
#include "PositionTextureVertex.h"
#include "TexturedQuad.h"

ModelBox::ModelBox(ModelRenderer* renderer, int textureOffsetX, int textureOffsetY,
                   float x, float y, float z, int sizeX, int sizeY, int sizeZ, float scale) :
    posX1(x),
    posY1(y),
    posZ1(z),
    posX2(x + static_cast<float>(sizeX)),
    posY2(y + static_cast<float>(sizeY)),
    posZ2(z + static_cast<float>(sizeZ)),
    boxName()
{
    for (int i = 0; i < 8; ++i)
        vertexPositions[i] = nullptr;
    for (int i = 0; i < 6; ++i)
        quadList[i] = nullptr;

    float x2 = x + static_cast<float>(sizeX);
    float y2 = y + static_cast<float>(sizeY);
    float z2 = z + static_cast<float>(sizeZ);
    x -= scale;
    y -= scale;
    z -= scale;
    x2 += scale;
    y2 += scale;
    z2 += scale;

    if (renderer != nullptr && renderer->mirror)
    {
        const float swap = x2;
        x2 = x;
        x = swap;
    }

    PositionTextureVertex* v0 = new PositionTextureVertex(x, y, z, 0.0f, 0.0f);
    PositionTextureVertex* v1 = new PositionTextureVertex(x2, y, z, 0.0f, 8.0f);
    PositionTextureVertex* v2 = new PositionTextureVertex(x2, y2, z, 8.0f, 8.0f);
    PositionTextureVertex* v3 = new PositionTextureVertex(x, y2, z, 8.0f, 0.0f);
    PositionTextureVertex* v4 = new PositionTextureVertex(x, y, z2, 0.0f, 0.0f);
    PositionTextureVertex* v5 = new PositionTextureVertex(x2, y, z2, 0.0f, 8.0f);
    PositionTextureVertex* v6 = new PositionTextureVertex(x2, y2, z2, 8.0f, 8.0f);
    PositionTextureVertex* v7 = new PositionTextureVertex(x, y2, z2, 8.0f, 0.0f);

    vertexPositions[0] = v0;
    vertexPositions[1] = v1;
    vertexPositions[2] = v2;
    vertexPositions[3] = v3;
    vertexPositions[4] = v4;
    vertexPositions[5] = v5;
    vertexPositions[6] = v6;
    vertexPositions[7] = v7;

    const float textureWidth = renderer != nullptr ? renderer->textureWidth : 64.0f;
    const float textureHeight = renderer != nullptr ? renderer->textureHeight : 32.0f;

    quadList[0] = new TexturedQuad(new PositionTextureVertex*[4]{v5, v1, v2, v6},
        textureOffsetX + sizeZ + sizeX, textureOffsetY + sizeZ,
        textureOffsetX + sizeZ + sizeX + sizeZ, textureOffsetY + sizeZ + sizeY,
        textureWidth, textureHeight);
    quadList[1] = new TexturedQuad(new PositionTextureVertex*[4]{v0, v4, v7, v3},
        textureOffsetX, textureOffsetY + sizeZ,
        textureOffsetX + sizeZ, textureOffsetY + sizeZ + sizeY,
        textureWidth, textureHeight);
    quadList[2] = new TexturedQuad(new PositionTextureVertex*[4]{v5, v4, v0, v1},
        textureOffsetX + sizeZ, textureOffsetY,
        textureOffsetX + sizeZ + sizeX, textureOffsetY + sizeZ,
        textureWidth, textureHeight);
    quadList[3] = new TexturedQuad(new PositionTextureVertex*[4]{v2, v3, v7, v6},
        textureOffsetX + sizeZ + sizeX, textureOffsetY + sizeZ,
        textureOffsetX + sizeZ + sizeX + sizeX, textureOffsetY,
        textureWidth, textureHeight);
    quadList[4] = new TexturedQuad(new PositionTextureVertex*[4]{v1, v0, v3, v2},
        textureOffsetX + sizeZ, textureOffsetY + sizeZ,
        textureOffsetX + sizeZ + sizeX, textureOffsetY + sizeZ + sizeY,
        textureWidth, textureHeight);
    quadList[5] = new TexturedQuad(new PositionTextureVertex*[4]{v4, v5, v6, v7},
        textureOffsetX + sizeZ + sizeX + sizeZ, textureOffsetY + sizeZ,
        textureOffsetX + sizeZ + sizeX + sizeZ + sizeX, textureOffsetY + sizeZ + sizeY,
        textureWidth, textureHeight);

    if (renderer != nullptr && renderer->mirror)
    {
        for (int i = 0; i < 6; ++i)
            quadList[i]->flipFace();
    }
}

ModelBox::~ModelBox()
{
    for (int i = 0; i < 6; ++i)
    {
        delete quadList[i];
        quadList[i] = nullptr;
    }
    for (int i = 0; i < 8; ++i)
    {
        delete vertexPositions[i];
        vertexPositions[i] = nullptr;
    }
}

void ModelBox::render(Tessellator* tessellator, float scale) const
{
    for (int i = 0; i < 6; ++i)
        quadList[i]->draw(tessellator, scale);
}

void ModelBox::emitInto(Tessellator* tessellator, float scale) const
{
    for (int i = 0; i < 6; ++i)
        quadList[i]->emitInto(tessellator, scale);
}

ModelBox* ModelBox::setBoxName(const std::string& name)
{
    boxName = name;
    return this;
}

TexturedQuad* ModelBox::getQuad(int index) const
{
    return index >= 0 && index < 6 ? quadList[index] : nullptr;
}
