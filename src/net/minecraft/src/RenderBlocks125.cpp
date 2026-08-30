#include "RenderBlocks.h"

#include <utility>

#include "Block.h"
#include "Config.h"
#include "ConnectedTextures.h"
#include "CustomColorizer.h"
#include "BlockBrewingStand.h"
#include "BlockCauldron.h"
#include "BlockDirectional.h"
#include "BlockDragonEgg.h"
#include "BlockEndPortalFrame.h"
#include "BlockFenceGate.h"
#include "BlockLilyPad.h"
#include "BlockPane.h"
#include "BlockStem.h"
#include "EntityRenderer.h"
#include "IBlockAccess.h"
#include "Tessellator.h"
#include "java/Arithmetic.h"

namespace
{
void applyAnaglyph(float &red, float &green, float &blue)
{
    if (!EntityRenderer::anaglyphEnabled)
        return;

    const float convertedRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
    const float convertedGreen = (red * 30.0f + green * 70.0f) / 100.0f;
    const float convertedBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
    red = convertedRed;
    green = convertedGreen;
    blue = convertedBlue;
}

void setBlockColor(Tessellator &tessellator, Block *block, IBlockAccess *access,
                   int_t x, int_t y, int_t z)
{
    const int_t color = block->colorMultiplier(access, x, y, z);
    float red = static_cast<float>((color >> 16) & 255) / 255.0f;
    float green = static_cast<float>((color >> 8) & 255) / 255.0f;
    float blue = static_cast<float>(color & 255) / 255.0f;
    applyAnaglyph(red, green, blue);
    tessellator.setColorOpaque_F(red, green, blue);
}
}

bool RenderBlocks::renderBlockStem(Block *block, int_t x, int_t y, int_t z)
{
    BlockStem *stem = static_cast<BlockStem *>(block);
    Tessellator &tessellator = Tessellator::instance;
    tessellator.setBrightness(stem->getMixedBrightnessForBlock(blockAccess, x, y, z));
    int_t stemColor = CustomColorizer::getStemColorMultiplier(stem, blockAccess, x, y, z);
    float stemRed = static_cast<float>((stemColor >> 16) & 255) / 255.0f;
    float stemGreen = static_cast<float>((stemColor >> 8) & 255) / 255.0f;
    float stemBlue = static_cast<float>(stemColor & 255) / 255.0f;
    applyAnaglyph(stemRed, stemGreen, stemBlue);
    tessellator.setColorOpaque_F(stemRed, stemGreen, stemBlue);
    stem->setBlockBoundsBasedOnState(blockAccess, x, y, z);

    const int_t metadata = accessGetBlockMetadata(x, y, z);
    const int_t direction = stem->getStateForNeighbor(blockAccess, x, y, z);
    if (direction < 0)
        renderBlockStemSmall(stem, metadata, stem->maxY, x, y, z);
    else
    {
        renderBlockStemSmall(stem, metadata, 0.5, x, y, z);
        renderBlockStemBig(stem, metadata, direction, stem->maxY, x, y, z);
    }
    return true;
}

void RenderBlocks::renderBlockStemSmall(Block *block, int_t metadata, double height,
                                        double x, double y, double z)
{
    Tessellator &tessellator = Tessellator::instance;
    int_t texture = block->getBlockTextureFromSideAndMetadata(0, metadata);
    if (overrideBlockTexture >= 0)
        texture = overrideBlockTexture;

    const int_t textureX = (texture & 15) << 4;
    const int_t textureY = texture & 240;
    const tess_coord_t renderHeight = static_cast<tess_coord_t>(height);
    const tess_coord_t renderX = static_cast<tess_coord_t>(x);
    const tess_coord_t renderY = static_cast<tess_coord_t>(y);
    const tess_coord_t renderZ = static_cast<tess_coord_t>(z);
    const tess_coord_t u0 = static_cast<tess_coord_t>(textureX) / 256.0f;
    const tess_coord_t u1 = (static_cast<tess_coord_t>(textureX) + static_cast<tess_coord_t>(15.99)) / 256.0f;
    const tess_coord_t v0 = static_cast<tess_coord_t>(textureY) / 256.0f;
    const tess_coord_t v1 = (static_cast<tess_coord_t>(textureY) + static_cast<tess_coord_t>(15.99) * renderHeight) / 256.0f;
    const tess_coord_t x0 = renderX + 0.5f - static_cast<tess_coord_t>(0.45);
    const tess_coord_t x1 = renderX + 0.5f + static_cast<tess_coord_t>(0.45);
    const tess_coord_t z0 = renderZ + 0.5f - static_cast<tess_coord_t>(0.45);
    const tess_coord_t z1 = renderZ + 0.5f + static_cast<tess_coord_t>(0.45);

    tessellator.addVertexWithUV(x0, renderY + renderHeight, z0, u0, v0);
    tessellator.addVertexWithUV(x0, renderY, z0, u0, v1);
    tessellator.addVertexWithUV(x1, renderY, z1, u1, v1);
    tessellator.addVertexWithUV(x1, renderY + renderHeight, z1, u1, v0);
    tessellator.addVertexWithUV(x1, renderY + renderHeight, z1, u0, v0);
    tessellator.addVertexWithUV(x1, renderY, z1, u0, v1);
    tessellator.addVertexWithUV(x0, renderY, z0, u1, v1);
    tessellator.addVertexWithUV(x0, renderY + renderHeight, z0, u1, v0);
    tessellator.addVertexWithUV(x0, renderY + renderHeight, z1, u0, v0);
    tessellator.addVertexWithUV(x0, renderY, z1, u0, v1);
    tessellator.addVertexWithUV(x1, renderY, z0, u1, v1);
    tessellator.addVertexWithUV(x1, renderY + renderHeight, z0, u1, v0);
    tessellator.addVertexWithUV(x1, renderY + renderHeight, z0, u0, v0);
    tessellator.addVertexWithUV(x1, renderY, z0, u0, v1);
    tessellator.addVertexWithUV(x0, renderY, z1, u1, v1);
    tessellator.addVertexWithUV(x0, renderY + renderHeight, z1, u1, v0);
}

void RenderBlocks::renderBlockStemBig(Block *block, int_t metadata, int_t direction,
                                      double height, double x, double y, double z)
{
    Tessellator &tessellator = Tessellator::instance;
    int_t texture = block->getBlockTextureFromSideAndMetadata(0, metadata) + 16;
    if (overrideBlockTexture >= 0)
        texture = overrideBlockTexture;

    const int_t textureX = (texture & 15) << 4;
    const int_t textureY = texture & 240;
    const tess_coord_t renderHeight = static_cast<tess_coord_t>(height);
    const tess_coord_t renderX = static_cast<tess_coord_t>(x);
    const tess_coord_t renderY = static_cast<tess_coord_t>(y);
    const tess_coord_t renderZ = static_cast<tess_coord_t>(z);
    tess_coord_t u0 = static_cast<tess_coord_t>(textureX) / 256.0f;
    tess_coord_t u1 = (static_cast<tess_coord_t>(textureX) + static_cast<tess_coord_t>(15.99)) / 256.0f;
    const tess_coord_t v0 = static_cast<tess_coord_t>(textureY) / 256.0f;
    const tess_coord_t v1 = (static_cast<tess_coord_t>(textureY) + static_cast<tess_coord_t>(15.99) * renderHeight) / 256.0f;
    const tess_coord_t x0 = renderX;
    const tess_coord_t x1 = renderX + 1.0f;
    const tess_coord_t z0 = renderZ;
    const tess_coord_t z1 = renderZ + 1.0f;
    const tess_coord_t centerX = renderX + 0.5f;
    const tess_coord_t centerZ = renderZ + 0.5f;

    if (((direction + 1) / 2) % 2 == 1)
        std::swap(u0, u1);

    if (direction < 2)
    {
        tessellator.addVertexWithUV(x0, renderY + renderHeight, centerZ, u0, v0);
        tessellator.addVertexWithUV(x0, renderY, centerZ, u0, v1);
        tessellator.addVertexWithUV(x1, renderY, centerZ, u1, v1);
        tessellator.addVertexWithUV(x1, renderY + renderHeight, centerZ, u1, v0);
        tessellator.addVertexWithUV(x1, renderY + renderHeight, centerZ, u1, v0);
        tessellator.addVertexWithUV(x1, renderY, centerZ, u1, v1);
        tessellator.addVertexWithUV(x0, renderY, centerZ, u0, v1);
        tessellator.addVertexWithUV(x0, renderY + renderHeight, centerZ, u0, v0);
    }
    else
    {
        tessellator.addVertexWithUV(centerX, renderY + renderHeight, z1, u0, v0);
        tessellator.addVertexWithUV(centerX, renderY, z1, u0, v1);
        tessellator.addVertexWithUV(centerX, renderY, z0, u1, v1);
        tessellator.addVertexWithUV(centerX, renderY + renderHeight, z0, u1, v0);
        tessellator.addVertexWithUV(centerX, renderY + renderHeight, z0, u1, v0);
        tessellator.addVertexWithUV(centerX, renderY, z0, u1, v1);
        tessellator.addVertexWithUV(centerX, renderY, z1, u0, v1);
        tessellator.addVertexWithUV(centerX, renderY + renderHeight, z1, u0, v0);
    }
}

bool RenderBlocks::renderBlockVine(Block *block, int_t x, int_t y, int_t z)
{
    Tessellator &tessellator = Tessellator::instance;
    int_t texture = block->getBlockTextureFromSide(0);
    if (overrideBlockTexture >= 0)
        texture = overrideBlockTexture;

    tessellator.setBrightness(block->getMixedBrightnessForBlock(blockAccess, x, y, z));
    setBlockColor(tessellator, block, blockAccess, x, y, z);

    const int_t textureX = (texture & 15) << 4;
    const int_t textureY = texture & 240;
    const tess_coord_t u0 = static_cast<tess_coord_t>(textureX) / 256.0f;
    const tess_coord_t u1 = (static_cast<tess_coord_t>(textureX) + static_cast<tess_coord_t>(15.99)) / 256.0f;
    const tess_coord_t v0 = static_cast<tess_coord_t>(textureY) / 256.0f;
    const tess_coord_t v1 = (static_cast<tess_coord_t>(textureY) + static_cast<tess_coord_t>(15.99)) / 256.0f;
    const tess_coord_t inset = static_cast<tess_coord_t>(0.05);
    const int_t metadata = accessGetBlockMetadata(x, y, z);

    if ((metadata & 2) != 0)
    {
        tessellator.addVertexWithUV(x + inset, y + 1, z + 1, u0, v0);
        tessellator.addVertexWithUV(x + inset, y, z + 1, u0, v1);
        tessellator.addVertexWithUV(x + inset, y, z, u1, v1);
        tessellator.addVertexWithUV(x + inset, y + 1, z, u1, v0);
        tessellator.addVertexWithUV(x + inset, y + 1, z, u1, v0);
        tessellator.addVertexWithUV(x + inset, y, z, u1, v1);
        tessellator.addVertexWithUV(x + inset, y, z + 1, u0, v1);
        tessellator.addVertexWithUV(x + inset, y + 1, z + 1, u0, v0);
    }
    if ((metadata & 8) != 0)
    {
        tessellator.addVertexWithUV(x + 1 - inset, y, z + 1, u1, v1);
        tessellator.addVertexWithUV(x + 1 - inset, y + 1, z + 1, u1, v0);
        tessellator.addVertexWithUV(x + 1 - inset, y + 1, z, u0, v0);
        tessellator.addVertexWithUV(x + 1 - inset, y, z, u0, v1);
        tessellator.addVertexWithUV(x + 1 - inset, y, z, u0, v1);
        tessellator.addVertexWithUV(x + 1 - inset, y + 1, z, u0, v0);
        tessellator.addVertexWithUV(x + 1 - inset, y + 1, z + 1, u1, v0);
        tessellator.addVertexWithUV(x + 1 - inset, y, z + 1, u1, v1);
    }
    if ((metadata & 4) != 0)
    {
        tessellator.addVertexWithUV(x + 1, y, z + inset, u1, v1);
        tessellator.addVertexWithUV(x + 1, y + 1, z + inset, u1, v0);
        tessellator.addVertexWithUV(x, y + 1, z + inset, u0, v0);
        tessellator.addVertexWithUV(x, y, z + inset, u0, v1);
        tessellator.addVertexWithUV(x, y, z + inset, u0, v1);
        tessellator.addVertexWithUV(x, y + 1, z + inset, u0, v0);
        tessellator.addVertexWithUV(x + 1, y + 1, z + inset, u1, v0);
        tessellator.addVertexWithUV(x + 1, y, z + inset, u1, v1);
    }
    if ((metadata & 1) != 0)
    {
        tessellator.addVertexWithUV(x + 1, y + 1, z + 1 - inset, u0, v0);
        tessellator.addVertexWithUV(x + 1, y, z + 1 - inset, u0, v1);
        tessellator.addVertexWithUV(x, y, z + 1 - inset, u1, v1);
        tessellator.addVertexWithUV(x, y + 1, z + 1 - inset, u1, v0);
        tessellator.addVertexWithUV(x, y + 1, z + 1 - inset, u1, v0);
        tessellator.addVertexWithUV(x, y, z + 1 - inset, u1, v1);
        tessellator.addVertexWithUV(x + 1, y, z + 1 - inset, u0, v1);
        tessellator.addVertexWithUV(x + 1, y + 1, z + 1 - inset, u0, v0);
    }
    if (accessIsBlockNormalCube(x, y + 1, z))
    {
        tessellator.addVertexWithUV(x + 1, y + 1 - inset, z, u0, v0);
        tessellator.addVertexWithUV(x + 1, y + 1 - inset, z + 1, u0, v1);
        tessellator.addVertexWithUV(x, y + 1 - inset, z + 1, u1, v1);
        tessellator.addVertexWithUV(x, y + 1 - inset, z, u1, v0);
    }
    renderBetterSnow(x, y, z);
    return true;
}

bool RenderBlocks::renderBlockFenceGate(Block *block, int_t x, int_t y, int_t z)
{
    BlockFenceGate *gate = static_cast<BlockFenceGate *>(block);
    const int_t metadata = accessGetBlockMetadata(x, y, z);
    const bool open = BlockFenceGate::isFenceGateOpen(metadata);
    const int_t direction = BlockDirectional::getDirection(metadata);
    float minA, maxA, minB, maxB;

    if (direction != 3 && direction != 1)
    {
        minA = 0.0f; maxA = 2.0f / 16.0f; minB = 7.0f / 16.0f; maxB = 9.0f / 16.0f;
        gate->setBlockBounds(minA, 5.0f / 16.0f, minB, maxA, 1.0f, maxB); renderStandardBlock(gate, x, y, z);
        minA = 14.0f / 16.0f; maxA = 1.0f;
        gate->setBlockBounds(minA, 5.0f / 16.0f, minB, maxA, 1.0f, maxB); renderStandardBlock(gate, x, y, z);
    }
    else
    {
        minA = 7.0f / 16.0f; maxA = 9.0f / 16.0f; minB = 0.0f; maxB = 2.0f / 16.0f;
        gate->setBlockBounds(minA, 5.0f / 16.0f, minB, maxA, 1.0f, maxB); renderStandardBlock(gate, x, y, z);
        minB = 14.0f / 16.0f; maxB = 1.0f;
        gate->setBlockBounds(minA, 5.0f / 16.0f, minB, maxA, 1.0f, maxB); renderStandardBlock(gate, x, y, z);
    }

    if (!open)
    {
        if (direction != 3 && direction != 1)
        {
            minA = 6.0f / 16.0f; maxA = 0.5f; minB = 7.0f / 16.0f; maxB = 9.0f / 16.0f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            minA = 0.5f; maxA = 10.0f / 16.0f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            minA = 10.0f / 16.0f; maxA = 14.0f / 16.0f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 9.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            gate->setBlockBounds(minA, 12.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            minA = 2.0f / 16.0f; maxA = 6.0f / 16.0f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 9.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            gate->setBlockBounds(minA, 12.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
        }
        else
        {
            minA = 7.0f / 16.0f; maxA = 9.0f / 16.0f; minB = 6.0f / 16.0f; maxB = 0.5f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            minB = 0.5f; maxB = 10.0f / 16.0f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            minB = 10.0f / 16.0f; maxB = 14.0f / 16.0f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 9.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            gate->setBlockBounds(minA, 12.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            minB = 2.0f / 16.0f; maxB = 6.0f / 16.0f;
            gate->setBlockBounds(minA, 6.0f / 16.0f, minB, maxA, 9.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
            gate->setBlockBounds(minA, 12.0f / 16.0f, minB, maxA, 15.0f / 16.0f, maxB); renderStandardBlock(gate, x, y, z);
        }
    }
    else if (direction == 3)
    {
        gate->setBlockBounds(13.0f/16,6.0f/16,0,15.0f/16,15.0f/16,2.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(13.0f/16,6.0f/16,14.0f/16,15.0f/16,15.0f/16,1); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(9.0f/16,6.0f/16,0,13.0f/16,9.0f/16,2.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(9.0f/16,6.0f/16,14.0f/16,13.0f/16,9.0f/16,1); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(9.0f/16,12.0f/16,0,13.0f/16,15.0f/16,2.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(9.0f/16,12.0f/16,14.0f/16,13.0f/16,15.0f/16,1); renderStandardBlock(gate,x,y,z);
    }
    else if (direction == 1)
    {
        gate->setBlockBounds(1.0f/16,6.0f/16,0,3.0f/16,15.0f/16,2.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(1.0f/16,6.0f/16,14.0f/16,3.0f/16,15.0f/16,1); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(3.0f/16,6.0f/16,0,7.0f/16,9.0f/16,2.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(3.0f/16,6.0f/16,14.0f/16,7.0f/16,9.0f/16,1); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(3.0f/16,12.0f/16,0,7.0f/16,15.0f/16,2.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(3.0f/16,12.0f/16,14.0f/16,7.0f/16,15.0f/16,1); renderStandardBlock(gate,x,y,z);
    }
    else if (direction == 0)
    {
        gate->setBlockBounds(0,6.0f/16,13.0f/16,2.0f/16,15.0f/16,15.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(14.0f/16,6.0f/16,13.0f/16,1,15.0f/16,15.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(0,6.0f/16,9.0f/16,2.0f/16,9.0f/16,13.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(14.0f/16,6.0f/16,9.0f/16,1,9.0f/16,13.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(0,12.0f/16,9.0f/16,2.0f/16,15.0f/16,13.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(14.0f/16,12.0f/16,9.0f/16,1,15.0f/16,13.0f/16); renderStandardBlock(gate,x,y,z);
    }
    else if (direction == 2)
    {
        gate->setBlockBounds(0,6.0f/16,1.0f/16,2.0f/16,15.0f/16,3.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(14.0f/16,6.0f/16,1.0f/16,1,15.0f/16,3.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(0,6.0f/16,3.0f/16,2.0f/16,9.0f/16,7.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(14.0f/16,6.0f/16,3.0f/16,1,9.0f/16,7.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(0,12.0f/16,3.0f/16,2.0f/16,15.0f/16,7.0f/16); renderStandardBlock(gate,x,y,z);
        gate->setBlockBounds(14.0f/16,12.0f/16,3.0f/16,1,15.0f/16,7.0f/16); renderStandardBlock(gate,x,y,z);
    }

    gate->setBlockBounds(0, 0, 0, 1, 1, 1);
    return true;
}

bool RenderBlocks::renderBlockLilyPad(Block *block, int_t x, int_t y, int_t z)
{
    Tessellator &tessellator = Tessellator::instance;
    int_t texture = block->blockIndexInTexture;
    if (overrideBlockTexture >= 0)
        texture = overrideBlockTexture;

    const int_t textureX = (texture & 15) << 4;
    const int_t textureY = texture & 240;
    const float yOffset = 0.015625f;
    const tess_coord_t u0 = static_cast<tess_coord_t>(textureX) / 256.0f;
    const tess_coord_t u1 = (static_cast<tess_coord_t>(textureX) + static_cast<tess_coord_t>(15.99)) / 256.0f;
    const tess_coord_t v0 = static_cast<tess_coord_t>(textureY) / 256.0f;
    const tess_coord_t v1 = (static_cast<tess_coord_t>(textureY) + static_cast<tess_coord_t>(15.99)) / 256.0f;

    const int_t xHash = JavaArithmetic::intFromBits(static_cast<uint_t>(x) * UINT32_C(3129871));
    const long_t zHash = JavaArithmetic::longFromBits(static_cast<ulong_t>(static_cast<long_t>(z)) * UINT64_C(116129781));
    const ulong_t initialBits = static_cast<ulong_t>(static_cast<long_t>(xHash)) ^
                                static_cast<ulong_t>(zHash) ^
                                static_cast<ulong_t>(static_cast<long_t>(y));
    long_t seed = JavaArithmetic::longFromBits(initialBits);
    const ulong_t seedBits = static_cast<ulong_t>(seed);
    seed = JavaArithmetic::longFromBits(seedBits * seedBits * 42317861ULL + seedBits * 11ULL);
    const int_t rotation = static_cast<int_t>((static_cast<ulong_t>(seed) >> 16) & 3ULL);

    tessellator.setBrightness(block->getMixedBrightnessForBlock(blockAccess, x, y, z));
    const float centerX = static_cast<float>(x) + 0.5f;
    const float centerZ = static_cast<float>(z) + 0.5f;
    const float dx = static_cast<float>(rotation & 1) * 0.5f * static_cast<float>(1 - (rotation / 2 % 2) * 2);
    const int_t nextRotation = rotation + 1;
    const float dz = static_cast<float>(nextRotation & 1) * 0.5f * static_cast<float>(1 - (nextRotation / 2 % 2) * 2);

    tessellator.setColorOpaque_I(CustomColorizer::getLilypadColor());
    tessellator.addVertexWithUV(centerX + dx - dz, y + yOffset, centerZ + dx + dz, u0, v0);
    tessellator.addVertexWithUV(centerX + dx + dz, y + yOffset, centerZ - dx + dz, u1, v0);
    tessellator.addVertexWithUV(centerX - dx + dz, y + yOffset, centerZ - dx - dz, u1, v1);
    tessellator.addVertexWithUV(centerX - dx - dz, y + yOffset, centerZ + dx - dz, u0, v1);
    tessellator.setColorOpaque_I((CustomColorizer::getLilypadColor() & 16711422) >> 1);
    tessellator.addVertexWithUV(centerX - dx - dz, y + yOffset, centerZ + dx - dz, u0, v1);
    tessellator.addVertexWithUV(centerX - dx + dz, y + yOffset, centerZ - dx - dz, u1, v1);
    tessellator.addVertexWithUV(centerX + dx + dz, y + yOffset, centerZ - dx + dz, u1, v0);
    tessellator.addVertexWithUV(centerX + dx - dz, y + yOffset, centerZ + dx + dz, u0, v0);
    return true;
}

bool RenderBlocks::renderBlockCauldron(Block *block, int_t x, int_t y, int_t z)
{
    renderStandardBlock(block, x, y, z);
    Tessellator &tessellator = Tessellator::instance;
    tessellator.setBrightness(block->getMixedBrightnessForBlock(blockAccess, x, y, z));
    setBlockColor(tessellator, block, blockAccess, x, y, z);

    const float inset = 2.0f / 16.0f;
    renderSouthFace(block, x - 1.0f + inset, y, z, 154);
    renderNorthFace(block, x + 1.0f - inset, y, z, 154);
    renderWestFace(block, x, y, z - 1.0f + inset, 154);
    renderEastFace(block, x, y, z + 1.0f - inset, 154);
    renderTopFace(block, x, y - 1.0f + 0.25f, z, 139);
    renderBottomFace(block, x, y + 1.0f - 12.0f / 16.0f, z, 139);

    int_t metadata = accessGetBlockMetadata(x, y, z);
    if (metadata > 0)
    {
        if (metadata > 3)
            metadata = 3;
        renderTopFace(block, x, y - 1.0f + (6.0f + metadata * 3.0f) / 16.0f, z, 205);
    }
    return true;
}

bool RenderBlocks::renderBlockBrewingStand(Block *block, int_t x, int_t y, int_t z)
{
    block->setBlockBounds(7.0f/16,0,7.0f/16,9.0f/16,14.0f/16,9.0f/16); renderStandardBlock(block,x,y,z);
    overrideBlockTexture = 156;
    block->setBlockBounds(9.0f/16,0,5.0f/16,15.0f/16,2.0f/16,11.0f/16); renderStandardBlock(block,x,y,z);
    block->setBlockBounds(2.0f/16,0,1.0f/16,0.5f,2.0f/16,7.0f/16); renderStandardBlock(block,x,y,z);
    block->setBlockBounds(2.0f/16,0,9.0f/16,0.5f,2.0f/16,15.0f/16); renderStandardBlock(block,x,y,z);
    clearOverrideBlockTexture();

    Tessellator &tessellator = Tessellator::instance;
    tessellator.setBrightness(block->getMixedBrightnessForBlock(blockAccess, x, y, z));
    setBlockColor(tessellator, block, blockAccess, x, y, z);
    int_t texture = block->getBlockTextureFromSideAndMetadata(0, 0);
    if (overrideBlockTexture >= 0)
        texture = overrideBlockTexture;
    const int_t textureX = (texture & 15) << 4;
    const int_t textureY = texture & 240;
    const tess_coord_t v0 = static_cast<tess_coord_t>(textureY) / 256.0f;
    const tess_coord_t v1 = (static_cast<tess_coord_t>(textureY) + static_cast<tess_coord_t>(15.99)) / 256.0f;
    const int_t metadata = accessGetBlockMetadata(x, y, z);
    // These are sin/cos(angle) * 0.5 for the three fixed brewing-stand
    // arms (90, 210 and 330 degrees). Avoiding libm here matters on the R5900:
    // this code runs while building terrain and the geometry never changes.
    static const tess_coord_t armOffsetX[3] = { 0.5f, -0.25f, -0.25f };
    static const tess_coord_t armOffsetZ[3] = { 0.0f, -static_cast<tess_coord_t>(0.4330127018922193), static_cast<tess_coord_t>(0.4330127018922193) };

    for (int_t arm = 0; arm < 3; ++arm)
    {
        tess_coord_t u0 = (static_cast<tess_coord_t>(textureX) + 8.0f) / 256.0f;
        tess_coord_t u1 = (static_cast<tess_coord_t>(textureX) + static_cast<tess_coord_t>(15.99)) / 256.0f;
        if ((metadata & (1 << arm)) != 0)
        {
            u0 = (static_cast<tess_coord_t>(textureX) + static_cast<tess_coord_t>(7.99)) / 256.0f;
            u1 = static_cast<tess_coord_t>(textureX) / 256.0f;
        }
        const tess_coord_t centerX = x + 0.5f;
        const tess_coord_t outerX = x + 0.5f + armOffsetX[arm];
        const tess_coord_t centerZ = z + 0.5f;
        const tess_coord_t outerZ = z + 0.5f + armOffsetZ[arm];
        tessellator.addVertexWithUV(centerX,y+1,centerZ,u0,v0);
        tessellator.addVertexWithUV(centerX,y,centerZ,u0,v1);
        tessellator.addVertexWithUV(outerX,y,outerZ,u1,v1);
        tessellator.addVertexWithUV(outerX,y+1,outerZ,u1,v0);
        tessellator.addVertexWithUV(outerX,y+1,outerZ,u1,v0);
        tessellator.addVertexWithUV(outerX,y,outerZ,u1,v1);
        tessellator.addVertexWithUV(centerX,y,centerZ,u0,v1);
        tessellator.addVertexWithUV(centerX,y+1,centerZ,u0,v0);
    }
    block->setBlockBoundsForItemRender();
    return true;
}

bool RenderBlocks::renderBlockEndPortalFrame(Block *block, int_t x, int_t y, int_t z)
{
    const int_t metadata = accessGetBlockMetadata(x, y, z);
    const int_t direction = metadata & 3;
    if (direction == 0) topFaceRotation = 3;
    else if (direction == 3) topFaceRotation = 1;
    else if (direction == 1) topFaceRotation = 2;

    block->setBlockBounds(0, 0, 0, 1, 13.0f / 16.0f, 1);
    renderStandardBlock(block, x, y, z);
    if (BlockEndPortalFrame::isEnderEyeInserted(metadata))
    {
        overrideBlockTexture = 174;
        block->setBlockBounds(0.25f, 13.0f/16, 0.25f, 12.0f/16, 1, 12.0f/16);
        renderStandardBlock(block, x, y, z);
        clearOverrideBlockTexture();
    }
    block->setBlockBoundsForItemRender();
    topFaceRotation = 0;
    return true;
}

bool RenderBlocks::renderBlockDragonEgg(Block *block, int_t x, int_t y, int_t z)
{
    int_t accumulatedHeight = 0;
    for (int_t layer = 0; layer < 8; ++layer)
    {
        int_t radiusUnits = 0;
        int_t heightUnits = 1;
        switch (layer)
        {
        case 0: radiusUnits = 2; break;
        case 1: radiusUnits = 3; break;
        case 2: radiusUnits = 4; break;
        case 3: radiusUnits = 5; heightUnits = 2; break;
        case 4: radiusUnits = 6; heightUnits = 3; break;
        case 5: radiusUnits = 7; heightUnits = 5; break;
        case 6: radiusUnits = 6; heightUnits = 2; break;
        case 7: radiusUnits = 3; break;
        }
        const float radius = static_cast<float>(radiusUnits) / 16.0f;
        const float top = 1.0f - static_cast<float>(accumulatedHeight) / 16.0f;
        const float bottom = 1.0f - static_cast<float>(accumulatedHeight + heightUnits) / 16.0f;
        accumulatedHeight += heightUnits;
        block->setBlockBounds(0.5f-radius,bottom,0.5f-radius,0.5f+radius,top,0.5f+radius);
        renderStandardBlock(block,x,y,z);
    }
    block->setBlockBounds(0,0,0,1,1,1);
    return true;
}

bool RenderBlocks::renderBlockPane(Block *block, int_t var2, int_t var3, int_t var4)
{
    BlockPane *var1 = static_cast<BlockPane *>(block);

    int_t var5 = blockAccess->getHeight();
    Tessellator &var6 = Tessellator::instance;
    bool connected = block == Block::thinGlass && ConnectedTextures::isConnectedGlassPanes();
    Tessellator *paneFront = &var6;
    var6.setBrightness(var1->getMixedBrightnessForBlock(blockAccess, var2, var3, var4));
    float var7 = 1.0F;
    int_t var8 = var1->colorMultiplier(blockAccess, var2, var3, var4);
    float var9 = (float)(var8 >> 16 & 255) / 255.0F;
    float var10 = (float)(var8 >> 8 & 255) / 255.0F;
    float var11 = (float)(var8 & 255) / 255.0F;
    if(EntityRenderer::anaglyphEnabled) {
    	float var12 = (var9 * 30.0F + var10 * 59.0F + var11 * 11.0F) / 100.0F;
    	float var13 = (var9 * 30.0F + var10 * 70.0F) / 100.0F;
    	float var14 = (var9 * 30.0F + var11 * 70.0F) / 100.0F;
    	var9 = var12;
    	var10 = var13;
    	var11 = var14;
    }

    var6.setColorOpaque_F(var7 * var9, var7 * var10, var7 * var11);
    bool var64 = false;
    bool var66 = false;
    int_t var65;
    int_t var67;
    int_t var68;
    if(overrideBlockTexture >= 0) {
    	var65 = overrideBlockTexture;
    	var67 = overrideBlockTexture;
		connected = false;
    } else {
    	var68 = accessGetBlockMetadata(var2, var3, var4);
    	var65 = var1->getBlockTextureFromSideAndMetadata(0, var68);
    	var67 = var1->getSideTextureIndex();
		if (connected)
		{
			const int_t ctmTexture = ConnectedTextures::getCtmTextureId();
			if (ctmTexture >= 0)
			{
				paneFront = var6.getSubTessellator(ctmTexture);
				var65 = 0;
			}
			else
			{
				connected = false;
			}
		}
    }

	int_t paneTextureX = var65;
	int_t paneTextureXReverse = var65;
	int_t paneTextureZ = var65;
	int_t paneTextureZReverse = var65;
	if (connected)
	{
		const int_t glassPaneId = Block::thinGlass->blockID;
		const bool linkXp = accessGetBlockId(var2 + 1, var3, var4) == glassPaneId;
		const bool linkXn = accessGetBlockId(var2 - 1, var3, var4) == glassPaneId;
		const bool linkYp = accessGetBlockId(var2, var3 + 1, var4) == glassPaneId;
		const bool linkYn = accessGetBlockId(var2, var3 - 1, var4) == glassPaneId;
		const bool linkZp = accessGetBlockId(var2, var3, var4 + 1) == glassPaneId;
		const bool linkZn = accessGetBlockId(var2, var3, var4 - 1) == glassPaneId;
		paneTextureX = ConnectedTextures::getGlassPaneTexture(linkXp, linkXn, linkYp, linkYn);
		paneTextureXReverse = ConnectedTextures::getReverseGlassPaneTexture(paneTextureX);
		paneTextureZ = ConnectedTextures::getGlassPaneTexture(linkZp, linkZn, linkYp, linkYn);
		paneTextureZReverse = ConnectedTextures::getReverseGlassPaneTexture(paneTextureZ);
	}

    var68 = (paneTextureX & 15) << 4;
    int_t var15 = paneTextureX & 240;
    tess_coord_t var16 = (tess_coord_t)((float)var68 / 256.0F);
    tess_coord_t var18 = (tess_coord_t)(((float)var68 + 7.99F) / 256.0F);
    tess_coord_t var20 = (tess_coord_t)(((float)var68 + 15.99F) / 256.0F);
    tess_coord_t var22 = (tess_coord_t)((float)var15 / 256.0F);
    tess_coord_t var24 = (tess_coord_t)(((float)var15 + 15.99F) / 256.0F);
	int_t paneReverseU = (paneTextureXReverse & 15) << 4;
	int_t paneReverseV = paneTextureXReverse & 240;
	tess_coord_t paneXReverse0 = (tess_coord_t)((float)paneReverseU / 256.0F);
	tess_coord_t paneXReverseHalf = (tess_coord_t)(((float)paneReverseU + 7.99F) / 256.0F);
	tess_coord_t paneXReverse1 = (tess_coord_t)(((float)paneReverseU + 15.99F) / 256.0F);
	tess_coord_t paneXReverseV0 = (tess_coord_t)((float)paneReverseV / 256.0F);
	tess_coord_t paneXReverseV1 = (tess_coord_t)(((float)paneReverseV + 15.99F) / 256.0F);

	int_t paneZU = (paneTextureZ & 15) << 4;
	int_t paneZV = paneTextureZ & 240;
	tess_coord_t paneZ0 = (tess_coord_t)((float)paneZU / 256.0F);
	tess_coord_t paneZHalf = (tess_coord_t)(((float)paneZU + 7.99F) / 256.0F);
	tess_coord_t paneZ1 = (tess_coord_t)(((float)paneZU + 15.99F) / 256.0F);
	tess_coord_t paneZV0 = (tess_coord_t)((float)paneZV / 256.0F);
	tess_coord_t paneZV1 = (tess_coord_t)(((float)paneZV + 15.99F) / 256.0F);

	int_t paneZReverseU = (paneTextureZReverse & 15) << 4;
	int_t paneZReverseV = paneTextureZReverse & 240;
	tess_coord_t paneZReverse0 = (tess_coord_t)((float)paneZReverseU / 256.0F);
	tess_coord_t paneZReverseHalf = (tess_coord_t)(((float)paneZReverseU + 7.99F) / 256.0F);
	tess_coord_t paneZReverse1 = (tess_coord_t)(((float)paneZReverseU + 15.99F) / 256.0F);
	tess_coord_t paneZReverseV0 = (tess_coord_t)((float)paneZReverseV / 256.0F);
	tess_coord_t paneZReverseV1 = (tess_coord_t)(((float)paneZReverseV + 15.99F) / 256.0F);
    int_t var26 = (var67 & 15) << 4;
    int_t var27 = var67 & 240;
    tess_coord_t var28 = (tess_coord_t)((float)(var26 + 7) / 256.0F);
    tess_coord_t var30 = (tess_coord_t)(((float)var26 + 8.99F) / 256.0F);
    tess_coord_t var32 = (tess_coord_t)((float)var27 / 256.0F);
    tess_coord_t var34 = (tess_coord_t)((float)(var27 + 8) / 256.0F);
    tess_coord_t var36 = (tess_coord_t)(((float)var27 + 15.99F) / 256.0F);
    tess_coord_t var38 = (tess_coord_t)var2;
    tess_coord_t var40 = (tess_coord_t)var2 + 0.5f;
    tess_coord_t var42 = (tess_coord_t)(var2 + 1);
    tess_coord_t var44 = (tess_coord_t)var4;
    tess_coord_t var46 = (tess_coord_t)var4 + 0.5f;
    tess_coord_t var48 = (tess_coord_t)(var4 + 1);
    tess_coord_t var50 = (tess_coord_t)var2 + 0.5f - 1.0f / 16.0f;
    tess_coord_t var52 = (tess_coord_t)var2 + 0.5f + 1.0f / 16.0f;
    tess_coord_t var54 = (tess_coord_t)var4 + 0.5f - 1.0f / 16.0f;
    tess_coord_t var56 = (tess_coord_t)var4 + 0.5f + 1.0f / 16.0f;
    bool var58 = var1->canThisPaneConnectToThisBlockID(accessGetBlockId(var2, var3, var4 - 1));
    bool var59 = var1->canThisPaneConnectToThisBlockID(accessGetBlockId(var2, var3, var4 + 1));
    bool var60 = var1->canThisPaneConnectToThisBlockID(accessGetBlockId(var2 - 1, var3, var4));
    bool var61 = var1->canThisPaneConnectToThisBlockID(accessGetBlockId(var2 + 1, var3, var4));
    bool var62 = var1->shouldSideBeRendered(blockAccess, var2, var3 + 1, var4, 1);
    bool var63 = var1->shouldSideBeRendered(blockAccess, var2, var3 - 1, var4, 0);
    if((!var60 || !var61) && (var60 || var61 || var58 || var59)) {
    	if(var60 && !var61) {
	    		paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 1), var46, var16, var22);
	    		paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 0), var46, var16, var24);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, var18, var24);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, var18, var22);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, paneXReverseHalf, paneXReverseV0);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, paneXReverseHalf, paneXReverseV1);
	    		paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 0), var46, paneXReverse1, paneXReverseV1);
	    		paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 1), var46, paneXReverse1, paneXReverseV0);
    		if(!var59 && !var58) {
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var56, var28, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var56, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var54, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var54, var30, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var54, var28, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var54, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var56, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var56, var30, var32);
    		}

    		if(var62 || var3 < var5 - 1 && accessIsAirBlock(var2 - 1, var3 + 1, var4)) {
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    		}

    		if(var63 || var3 > 1 && accessIsAirBlock(var2 - 1, var3 - 1, var4)) {
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    		}
    	} else if(!var60 && var61) {
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, var18, var22);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, var18, var24);
	    		paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 0), var46, var20, var24);
	    		paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 1), var46, var20, var22);
	    		paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 1), var46, paneXReverse0, paneXReverseV0);
	    		paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 0), var46, paneXReverse0, paneXReverseV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, paneXReverseHalf, paneXReverseV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, paneXReverseHalf, paneXReverseV0);
    		if(!var59 && !var58) {
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var54, var28, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var54, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var56, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var56, var30, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var56, var28, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var56, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var54, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var54, var30, var32);
    		}

    		if(var62 || var3 < var5 - 1 && accessIsAirBlock(var2 + 1, var3 + 1, var4)) {
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		}

    		if(var63 || var3 > 1 && accessIsAirBlock(var2 + 1, var3 - 1, var4)) {
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		}
    	}
    } else {
	    	paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 1), var46, var16, var22);
	    	paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 0), var46, var16, var24);
	    	paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 0), var46, var20, var24);
	    	paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 1), var46, var20, var22);
	    	paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 1), var46, paneXReverse0, paneXReverseV0);
	    	paneFront->addVertexWithUV(var42, (tess_coord_t)(var3 + 0), var46, paneXReverse0, paneXReverseV1);
	    	paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 0), var46, paneXReverse1, paneXReverseV1);
	    	paneFront->addVertexWithUV(var38, (tess_coord_t)(var3 + 1), var46, paneXReverse1, paneXReverseV0);
    	if(var62) {
    		var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var36);
    		var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var32);
    		var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var36);
    		var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var36);
    		var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var32);
    		var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var36);
    	} else {
    		if(var3 < var5 - 1 && accessIsAirBlock(var2 - 1, var3 + 1, var4)) {
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    		}

    		if(var3 < var5 - 1 && accessIsAirBlock(var2 + 1, var3 + 1, var4)) {
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)(var3 + 1) + static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		}
    	}

    	if(var63) {
    		var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var36);
    		var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var32);
    		var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var36);
    		var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var36);
    		var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var32);
    		var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var36);
    	} else {
    		if(var3 > 1 && accessIsAirBlock(var2 - 1, var3 - 1, var4)) {
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var36);
    			var6.addVertexWithUV(var38, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var36);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    		}

    		if(var3 > 1 && accessIsAirBlock(var2 + 1, var3 - 1, var4)) {
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var32);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var32);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var56, var30, var34);
    			var6.addVertexWithUV(var40, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var34);
    			var6.addVertexWithUV(var42, (tess_coord_t)var3 - static_cast<tess_coord_t>(0.01), var54, var28, var32);
    		}
    	}
    }

    if((!var58 || !var59) && (var60 || var61 || var58 || var59)) {
    	if(var58 && !var59) {
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var44, paneZ0, paneZV0);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var44, paneZ0, paneZV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, paneZHalf, paneZV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, paneZHalf, paneZV0);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, paneZReverseHalf, paneZReverseV0);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, paneZReverseHalf, paneZReverseV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var44, paneZReverse1, paneZReverseV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var44, paneZReverse1, paneZReverseV0);
    		if(!var61 && !var60) {
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var28, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 0), var46, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 0), var46, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var30, var32);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var28, var32);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 0), var46, var28, var36);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 0), var46, var30, var36);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var30, var32);
    		}

    		if(var62 || var3 < var5 - 1 && accessIsAirBlock(var2, var3 + 1, var4 - 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var44, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var44, var28, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var44, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var44, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var28, var32);
    		}

    		if(var63 || var3 > 1 && accessIsAirBlock(var2, var3 - 1, var4 - 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var44, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var44, var28, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var44, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var44, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var28, var32);
    		}
    	} else if(!var58 && var59) {
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, paneZHalf, paneZV0);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, paneZHalf, paneZV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var48, paneZ1, paneZV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var48, paneZ1, paneZV0);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var48, paneZReverse0, paneZReverseV0);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var48, paneZReverse0, paneZReverseV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var46, paneZReverseHalf, paneZReverseV1);
	    		paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var46, paneZReverseHalf, paneZReverseV0);
    		if(!var61 && !var60) {
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var28, var32);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 0), var46, var28, var36);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 0), var46, var30, var36);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var28, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 0), var46, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 0), var46, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var30, var32);
    		}

    		if(var62 || var3 < var5 - 1 && accessIsAirBlock(var2, var3 + 1, var4 + 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var48, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var48, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var30, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var48, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var48, var30, var34);
    		}

    		if(var63 || var3 > 1 && accessIsAirBlock(var2, var3 - 1, var4 + 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var48, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var48, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var30, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var48, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var48, var30, var34);
    		}
    	}
    } else {
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var48, paneZReverse0, paneZReverseV0);
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var48, paneZReverse0, paneZReverseV1);
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var44, paneZReverse1, paneZReverseV1);
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var44, paneZReverse1, paneZReverseV0);
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var44, paneZ0, paneZV0);
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var44, paneZ0, paneZV1);
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 0), var48, paneZ1, paneZV1);
	    	paneFront->addVertexWithUV(var40, (tess_coord_t)(var3 + 1), var48, paneZ1, paneZV0);
    	if(var62) {
    		var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var48, var30, var36);
    		var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var44, var30, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var44, var28, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var48, var28, var36);
    		var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var44, var30, var36);
    		var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var48, var30, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var48, var28, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var44, var28, var36);
    	} else {
    		if(var3 < var5 - 1 && accessIsAirBlock(var2, var3 + 1, var4 - 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var44, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var44, var28, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var44, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var44, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var28, var32);
    		}

    		if(var3 < var5 - 1 && accessIsAirBlock(var2, var3 + 1, var4 + 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var48, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var48, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var30, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var48, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)(var3 + 1), var46, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var46, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)(var3 + 1), var48, var30, var34);
    		}
    	}

    	if(var63) {
    		var6.addVertexWithUV(var52, (tess_coord_t)var3, var48, var30, var36);
    		var6.addVertexWithUV(var52, (tess_coord_t)var3, var44, var30, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)var3, var44, var28, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)var3, var48, var28, var36);
    		var6.addVertexWithUV(var52, (tess_coord_t)var3, var44, var30, var36);
    		var6.addVertexWithUV(var52, (tess_coord_t)var3, var48, var30, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)var3, var48, var28, var32);
    		var6.addVertexWithUV(var50, (tess_coord_t)var3, var44, var28, var36);
    	} else {
    		if(var3 > 1 && accessIsAirBlock(var2, var3 - 1, var4 - 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var44, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var44, var28, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var30, var32);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var44, var30, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var44, var28, var34);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var28, var32);
    		}

    		if(var3 > 1 && accessIsAirBlock(var2, var3 - 1, var4 + 1)) {
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var48, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var48, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var30, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var48, var28, var34);
    			var6.addVertexWithUV(var50, (tess_coord_t)var3, var46, var28, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var46, var30, var36);
    			var6.addVertexWithUV(var52, (tess_coord_t)var3, var48, var30, var34);
    		}
    	}
    }

    return true;

}
