#pragma once

#include <vector>
#include "java/Type.h"
#include "ChunkPosition.h"

class BlockRail;
class World;

// net.minecraft.src.RailLogic
class RailLogic
{
public:
    RailLogic(BlockRail *blockrail, World *world, int_t i, int_t j, int_t k);

    void updateRail(bool isNewTrack, bool forceUpdate);

    static int_t getNAdjacentTracks(RailLogic *raillogic);

private:
    void setConnections(int_t meta);
    void removeInvalidConnections();
    bool isMinecartTrack(int_t i, int_t j, int_t k);
    RailLogic* getMinecartTrackLogic(const ChunkPosition &pos);
    bool isConnectedTo(RailLogic *other);
    bool isInTrack(int_t i, int_t j, int_t k);
    int_t getAdjacentTracks();
    bool tryConnect(RailLogic *other);
    void addConnection(RailLogic *other);
    bool canConnectFrom(int_t i, int_t j, int_t k);

    World *worldObj;
    int_t trackX;
    int_t trackY;
    int_t trackZ;
    const bool isPoweredRail;
    std::vector<ChunkPosition> connectedTracks;
    BlockRail *rail;
};
