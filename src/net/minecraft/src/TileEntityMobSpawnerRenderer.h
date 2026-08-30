#pragma once

#include <string>
#include <unordered_map>

#include "TileEntitySpecialRenderer.h"

class Entity;
class TileEntityMobSpawner;

class TileEntityMobSpawnerRenderer : public TileEntitySpecialRenderer {
public:
    TileEntityMobSpawnerRenderer();
    ~TileEntityMobSpawnerRenderer() override;

    void renderTileEntityMobSpawner(TileEntityMobSpawner *tileentitymobspawner, double d, double d1, double d2, float f);
    void renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f) override;

private:
    std::unordered_map<std::string, Entity *> entityHashMap;
};
