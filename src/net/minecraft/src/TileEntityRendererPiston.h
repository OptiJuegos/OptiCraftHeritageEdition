#pragma once

#include "TileEntitySpecialRenderer.h"

class RenderBlocks;
class TileEntityPiston;
class World;

class TileEntityRendererPiston : public TileEntitySpecialRenderer {
public:
    TileEntityRendererPiston();
    ~TileEntityRendererPiston() override;

    void renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f) override;
    void onWorldChanged(World* world) override;

private:
    void renderPistonAt(TileEntityPiston* tileentitypiston, double d, double d1, double d2, float f);

    RenderBlocks* renderBlocks; // field_31071_b
};
