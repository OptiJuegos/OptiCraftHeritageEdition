#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "java/Type.h"

class Entity;
class IBlockAccess;
class MCHash;
class Path;
class PathEntity;
class PathPoint;

class Pathfinder
{
public:
    explicit Pathfinder(IBlockAccess *blockAccess);
    Pathfinder(IBlockAccess *blockAccess, bool woodenDoorAllowed, bool movementBlockAllowed,
               bool pathingInWater, bool entityCanDrown);
    ~Pathfinder();

    void reset(IBlockAccess *blockAccess, bool woodenDoorAllowed, bool movementBlockAllowed,
               bool pathingInWater, bool entityCanDrown);

    PathEntity *createEntityPathTo(Entity *entity, Entity *target, float maxDistance);
    PathEntity *createEntityPathTo(Entity *entity, int_t x, int_t y, int_t z, float maxDistance);
    PathEntity *createEntityPathTo(Entity *entity, double x, double y, double z, float maxDistance);

private:
    struct PointMapSlot
    {
        int_t hash = 0;
        PathPoint *point = nullptr;
        uint32_t generation = 0;
    };

    PathEntity *addToPath(Entity *entity, PathPoint *start, PathPoint *target,
                          PathPoint *entitySize, float maxDistance);
    int_t findPathOptions(Entity *entity, PathPoint *current, PathPoint *entitySize,
                          PathPoint *target, float maxDistance);
    PathPoint *getSafePoint(Entity *entity, int_t x, int_t y, int_t z,
                            PathPoint *entitySize, int_t stepHeight);
    PathPoint *openPoint(int_t x, int_t y, int_t z);
    PathPoint *acquirePoint(int_t x, int_t y, int_t z);
    PathPoint *lookupPointTable(int_t hash) const;
    bool insertPointTable(int_t hash, PathPoint *point);
    PathPoint *lookupActivePoint(int_t hash) const;
    void beginPointTableSearch();
    int_t getVerticalOffset(Entity *entity, int_t x, int_t y, int_t z,
                            PathPoint *entitySize);
    PathEntity *createEntityPath(PathPoint *start, PathPoint *target);

    IBlockAccess *worldMap;
    Path *path;
    MCHash *pointMap = nullptr;
    bool isWoodenDoorAllowed;
    bool isMovementBlockAllowed;
    bool isPathingInWater;
    bool canEntityDrown;
    std::array<PathPoint *, 32> pathOptions{};
    std::vector<PathPoint *> allocatedPoints;
    std::vector<PointMapSlot> pointTable;
    uint32_t pointTableGeneration = 0;
    int_t pointTableMask = 0;
    bool pointTableOverflow = false;
    int_t activePointCount = 0;
};

using PathFinder = Pathfinder;
