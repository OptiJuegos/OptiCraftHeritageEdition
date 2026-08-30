#include "RailLogic.h"

#include "BlockRail.h"
#include "Block.h"
#include "World.h"

#include <memory>

RailLogic::RailLogic(BlockRail *blockrail, World *world, int_t i, int_t j, int_t k) :
    rail(blockrail),
    worldObj(world),
    trackX(i),
    trackY(j),
    trackZ(k),
    isPoweredRail(false)
{
    int_t l  = world->getBlockId(i, j, k);
    int_t i1 = world->getBlockMetadata(i, j, k);

    bool powered = false;
    if (BlockRail::isPoweredBlockRail((BlockRail*)Block::blocksList[l]))
    {
        powered = true;
        i1 &= ~8;
    }

    // isPoweredRail is const — assign via const_cast in constructor only
    const_cast<bool&>(isPoweredRail) = powered;

    setConnections(i1);
}

void RailLogic::setConnections(int_t i)
{
    connectedTracks.clear();
    switch (i)
    {
    case 0:
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ - 1));
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ + 1));
        break;
    case 1:
        connectedTracks.push_back(ChunkPosition(trackX - 1, trackY,     trackZ));
        connectedTracks.push_back(ChunkPosition(trackX + 1, trackY,     trackZ));
        break;
    case 2:
        connectedTracks.push_back(ChunkPosition(trackX - 1, trackY,     trackZ));
        connectedTracks.push_back(ChunkPosition(trackX + 1, trackY + 1, trackZ));
        break;
    case 3:
        connectedTracks.push_back(ChunkPosition(trackX - 1, trackY + 1, trackZ));
        connectedTracks.push_back(ChunkPosition(trackX + 1, trackY,     trackZ));
        break;
    case 4:
        connectedTracks.push_back(ChunkPosition(trackX,     trackY + 1, trackZ - 1));
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ + 1));
        break;
    case 5:
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ - 1));
        connectedTracks.push_back(ChunkPosition(trackX,     trackY + 1, trackZ + 1));
        break;
    case 6:
        connectedTracks.push_back(ChunkPosition(trackX + 1, trackY,     trackZ));
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ + 1));
        break;
    case 7:
        connectedTracks.push_back(ChunkPosition(trackX - 1, trackY,     trackZ));
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ + 1));
        break;
    case 8:
        connectedTracks.push_back(ChunkPosition(trackX - 1, trackY,     trackZ));
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ - 1));
        break;
    case 9:
        connectedTracks.push_back(ChunkPosition(trackX + 1, trackY,     trackZ));
        connectedTracks.push_back(ChunkPosition(trackX,     trackY,     trackZ - 1));
        break;
    }
}

// func_785_b — remove connections that are no longer valid or not connected back
void RailLogic::removeInvalidConnections()
{
    for (int_t i = 0; i < (int_t)connectedTracks.size(); i++)
    {
        std::unique_ptr<RailLogic> other(getMinecartTrackLogic(connectedTracks[i]));
        if (other == nullptr || !other->isConnectedTo(this))
        {
            connectedTracks.erase(connectedTracks.begin() + i);
            i--;
        }
        else
        {
            connectedTracks[i] = ChunkPosition(other->trackX, other->trackY, other->trackZ);
        }
    }
}

bool RailLogic::isMinecartTrack(int_t i, int_t j, int_t k)
{
    if (BlockRail::isRailBlockAt(worldObj, i, j,     k)) return true;
    if (BlockRail::isRailBlockAt(worldObj, i, j + 1, k)) return true;
    return BlockRail::isRailBlockAt(worldObj, i, j - 1, k);
}

RailLogic* RailLogic::getMinecartTrackLogic(const ChunkPosition &pos)
{
    if (BlockRail::isRailBlockAt(worldObj, pos.x, pos.y,     pos.z))
        return new RailLogic(rail, worldObj, pos.x, pos.y,     pos.z);
    if (BlockRail::isRailBlockAt(worldObj, pos.x, pos.y + 1, pos.z))
        return new RailLogic(rail, worldObj, pos.x, pos.y + 1, pos.z);
    if (BlockRail::isRailBlockAt(worldObj, pos.x, pos.y - 1, pos.z))
        return new RailLogic(rail, worldObj, pos.x, pos.y - 1, pos.z);
    return nullptr;
}

bool RailLogic::isConnectedTo(RailLogic *other)
{
    for (const ChunkPosition &pos : connectedTracks)
    {
        if (pos.x == other->trackX && pos.z == other->trackZ)
            return true;
    }
    return false;
}

bool RailLogic::isInTrack(int_t i, int_t j, int_t k)
{
    for (const ChunkPosition &pos : connectedTracks)
    {
        if (pos.x == i && pos.z == k)
            return true;
    }
    return false;
}

int_t RailLogic::getAdjacentTracks()
{
    int_t i = 0;
    if (isMinecartTrack(trackX,     trackY, trackZ - 1)) i++;
    if (isMinecartTrack(trackX,     trackY, trackZ + 1)) i++;
    if (isMinecartTrack(trackX - 1, trackY, trackZ))     i++;
    if (isMinecartTrack(trackX + 1, trackY, trackZ))     i++;
    return i;
}

// handleKeyPress (returns true if this rail can connect to the given neighbour)
bool RailLogic::tryConnect(RailLogic *other)
{
    if (isConnectedTo(other))
        return true;
    if ((int_t)connectedTracks.size() == 2)
        return false;
    if (connectedTracks.empty())
        return true;
    // Java always returns true in the else case here
    return true;
}

// func_788_d — add a connection and recalculate metadata
void RailLogic::addConnection(RailLogic *other)
{
    connectedTracks.push_back(ChunkPosition(other->trackX, other->trackY, other->trackZ));

    bool flag  = isInTrack(trackX,     trackY, trackZ - 1);
    bool flag1 = isInTrack(trackX,     trackY, trackZ + 1);
    bool flag2 = isInTrack(trackX - 1, trackY, trackZ);
    bool flag3 = isInTrack(trackX + 1, trackY, trackZ);

    int_t meta = -1;
    if (flag || flag1)  meta = 0;
    if (flag2 || flag3) meta = 1;

    if (!isPoweredRail)
    {
        if (flag1 && flag3 && !flag  && !flag2) meta = 6;
        if (flag1 && flag2 && !flag  && !flag3) meta = 7;
        if (flag  && flag2 && !flag1 && !flag3) meta = 8;
        if (flag  && flag3 && !flag1 && !flag2) meta = 9;
    }

    if (meta == 0)
    {
        if (BlockRail::isRailBlockAt(worldObj, trackX, trackY + 1, trackZ - 1)) meta = 4;
        if (BlockRail::isRailBlockAt(worldObj, trackX, trackY + 1, trackZ + 1)) meta = 5;
    }
    if (meta == 1)
    {
        if (BlockRail::isRailBlockAt(worldObj, trackX + 1, trackY + 1, trackZ)) meta = 2;
        if (BlockRail::isRailBlockAt(worldObj, trackX - 1, trackY + 1, trackZ)) meta = 3;
    }
    if (meta < 0) meta = 0;

    int_t finalMeta = meta;
    if (isPoweredRail)
        finalMeta = (worldObj->getBlockMetadata(trackX, trackY, trackZ) & 8) | meta;

    worldObj->setBlockMetadataWithNotify(trackX, trackY, trackZ, finalMeta);
}

// func_786_c
bool RailLogic::canConnectFrom(int_t i, int_t j, int_t k)
{
    std::unique_ptr<RailLogic> other(getMinecartTrackLogic(ChunkPosition(i, j, k)));
    if (other == nullptr)
        return false;
    other->removeInvalidConnections();
    return other->tryConnect(this);
}

// func_792_a — main update function
void RailLogic::updateRail(bool flag, bool flag1)
{
    bool flag2 = canConnectFrom(trackX,     trackY, trackZ - 1);
    bool flag3 = canConnectFrom(trackX,     trackY, trackZ + 1);
    bool flag4 = canConnectFrom(trackX - 1, trackY, trackZ);
    bool flag5 = canConnectFrom(trackX + 1, trackY, trackZ);

    int_t meta = -1;

    if ((flag2 || flag3) && !flag4 && !flag5) meta = 0;
    if ((flag4 || flag5) && !flag2 && !flag3) meta = 1;

    if (!isPoweredRail)
    {
        if (flag3 && flag5 && !flag2 && !flag4) meta = 6;
        if (flag3 && flag4 && !flag2 && !flag5) meta = 7;
        if (flag2 && flag4 && !flag3 && !flag5) meta = 8;
        if (flag2 && flag5 && !flag3 && !flag4) meta = 9;
    }

    if (meta == -1)
    {
        if (flag2 || flag3) meta = 0;
        if (flag4 || flag5) meta = 1;

        if (!isPoweredRail)
        {
            if (flag)
            {
                if (flag3 && flag5) meta = 6;
                if (flag4 && flag3) meta = 7;
                if (flag5 && flag2) meta = 9;
                if (flag2 && flag4) meta = 8;
            }
            else
            {
                if (flag2 && flag4) meta = 8;
                if (flag5 && flag2) meta = 9;
                if (flag4 && flag3) meta = 7;
                if (flag3 && flag5) meta = 6;
            }
        }
    }

    if (meta == 0)
    {
        if (BlockRail::isRailBlockAt(worldObj, trackX, trackY + 1, trackZ - 1)) meta = 4;
        if (BlockRail::isRailBlockAt(worldObj, trackX, trackY + 1, trackZ + 1)) meta = 5;
    }
    if (meta == 1)
    {
        if (BlockRail::isRailBlockAt(worldObj, trackX + 1, trackY + 1, trackZ)) meta = 2;
        if (BlockRail::isRailBlockAt(worldObj, trackX - 1, trackY + 1, trackZ)) meta = 3;
    }
    if (meta < 0) meta = 0;

    setConnections(meta);

    int_t finalMeta = meta;
    if (isPoweredRail)
        finalMeta = (worldObj->getBlockMetadata(trackX, trackY, trackZ) & 8) | meta;

    if (flag1 || worldObj->getBlockMetadata(trackX, trackY, trackZ) != finalMeta)
    {
        worldObj->setBlockMetadataWithNotify(trackX, trackY, trackZ, finalMeta);

        for (int_t j = 0; j < (int_t)connectedTracks.size(); j++)
        {
            std::unique_ptr<RailLogic> neighbour(getMinecartTrackLogic(connectedTracks[j]));
            if (neighbour == nullptr) continue;

            neighbour->removeInvalidConnections();
            if (neighbour->tryConnect(this))
                neighbour->addConnection(this);
        }
    }
}

int_t RailLogic::getNAdjacentTracks(RailLogic *raillogic)
{
    return raillogic->getAdjacentTracks();
}
