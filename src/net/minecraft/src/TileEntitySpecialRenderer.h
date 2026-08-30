#pragma once

#include <string>

class TileEntity;
class TileEntityRenderer;
class FontRenderer;
class World;

class TileEntitySpecialRenderer {
public:
    TileEntitySpecialRenderer();
    virtual ~TileEntitySpecialRenderer() = default;

    virtual void renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f) = 0;
    virtual void onWorldChanged(World* world) {}  // called when world changes; piston renderer creates new RenderBlocks
    void setTileEntityRenderer(TileEntityRenderer* renderer);
    FontRenderer* getFontRenderer();
    void bindTextureByName(const std::string& path);

    TileEntityRenderer* tileEntityRenderer;
};
