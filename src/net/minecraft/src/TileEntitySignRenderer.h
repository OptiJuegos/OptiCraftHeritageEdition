#pragma once

#include "TileEntitySpecialRenderer.h"

class SignModel;
class TileEntitySign;

class TileEntitySignRenderer : public TileEntitySpecialRenderer {
public:
    TileEntitySignRenderer();
    ~TileEntitySignRenderer() override;

    void renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f) override;

private:
    void renderTileEntitySignAt(TileEntitySign* tileentitysign, double d, double d1, double d2, float f);

    SignModel* signModel;
};
