#pragma once

#ifdef PS2_PLATFORM

#include <cstddef>
#include <vector>

#include "java/Type.h"
#include "ps2/render/Ps2NativeDraw.h"

typedef Ps2NativeClampRun Ps2TerrainTileRun;

enum class Ps2TerrainMeshBuildFailure
{
    None,
    InvalidInput,
    Allocation,
    EmptyRuns
};

struct Ps2TerrainMeshBuildStats
{
    unsigned int successful = 0;
    unsigned int invalidInput = 0;
    unsigned int allocationFailed = 0;
    unsigned int emptyRuns = 0;
};

Ps2TerrainMeshBuildStats ps2TerrainMeshBuildStats();

// Canonical immutable opaque chunk stream prepared once when a WorldRenderer
// mesh is published. VU1 consumes the aligned SoA streams through DMA REF;
// VU0 reads the same fixed-point data when clipping is required.
class Ps2TerrainMesh
{
public:
    Ps2TerrainMesh();

    bool build(const int_t* raw, std::size_t rawIntCount, int_t vertexCount,
               int_t drawMode, bool hasTexture, bool hasColor, bool hasNormals);
    void clearKeepCapacity();
    void release();

    bool valid() const { return m_valid; }
    int_t vertexCount() const { return m_vertexCount; }
    std::size_t ramBytes() const { return m_storage.capacity() + m_runs.capacity() * sizeof(Ps2TerrainTileRun); }
    const short* positions() const;
    const short* texCoords() const;
    const unsigned char* colors() const;
    const std::vector<Ps2TerrainTileRun>& runs() const { return m_runs; }

private:
    std::vector<unsigned char> m_storage;
    std::vector<Ps2TerrainTileRun> m_runs;
    std::size_t m_positionOffset;
    std::size_t m_texCoordOffset;
    std::size_t m_colorOffset;
    int_t m_vertexCount;
    bool m_valid;
};

#endif // PS2_PLATFORM
