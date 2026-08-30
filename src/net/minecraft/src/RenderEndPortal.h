#pragma once

#include "TileEntitySpecialRenderer.h"

class TileEntity;
class TileEntityEndPortal;

// net.minecraft.src.RenderEndPortal
class RenderEndPortal : public TileEntitySpecialRenderer
{
public:
    RenderEndPortal() = default;

    void renderEndPortal(TileEntityEndPortal *portal, double x, double y, double z, float partialTick);
    void renderTileEntityAt(TileEntity *tileentity, double x, double y, double z, float partialTick) override;

private:
    void renderConsoleLayers(double x, double y, double z, float partialTick);
};
