#include "ps2/render/Ps2TerrainMesh.h"

#ifdef PS2_PLATFORM

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <new>

#include <kernel.h>

#include "ps2/render/Ps2CaptureLayout.h"
#include "ps2/render/Ps2ClipGuard.h"
#include "ps2/render/Ps2NativeDraw.h"

namespace
{
    static const std::size_t kStreamAlignment = 64u;
    static const float kPositionScale = 1024.0f;
    static const float kUvScale = 4096.0f;

    static inline std::uintptr_t alignUp(std::uintptr_t value, std::size_t alignment)
    {
        return (value + alignment - 1u) & ~(std::uintptr_t)(alignment - 1u);
    }

    static inline float slotFloat(const int_t* raw, std::size_t index)
    {
        float value;
        std::memcpy(&value, raw + index, sizeof(value));
        return value;
    }

    static inline short packSigned16(float value)
    {
        if (value < -32768.0f)
            value = -32768.0f;
        else if (value > 32767.0f)
            value = 32767.0f;
        return (short)value;
    }

    static inline unsigned char gsColor(unsigned char value)
    {
        return (unsigned char)(((unsigned int)value * 128u + 127u) / 255u);
    }

    static inline int atlasTile(float uv)
    {
        int tile = ((int)(uv * 256.0f)) >> 4;
        if (tile < 0)
            return 0;
        if (tile > 15)
            return 15;
        return tile;
    }

    Ps2TerrainMeshBuildStats s_buildStats;
}

Ps2TerrainMeshBuildStats ps2TerrainMeshBuildStats()
{
    return s_buildStats;
}

Ps2TerrainMesh::Ps2TerrainMesh()
    : m_positionOffset(0), m_texCoordOffset(0), m_colorOffset(0),
      m_vertexCount(0), m_valid(false)
{
}

const short* Ps2TerrainMesh::positions() const
{
    return m_valid ? reinterpret_cast<const short*>(m_storage.data() + m_positionOffset) : nullptr;
}

const short* Ps2TerrainMesh::texCoords() const
{
    return m_valid ? reinterpret_cast<const short*>(m_storage.data() + m_texCoordOffset) : nullptr;
}

const unsigned char* Ps2TerrainMesh::colors() const
{
    return m_valid ? m_storage.data() + m_colorOffset : nullptr;
}

bool Ps2TerrainMesh::build(const int_t* raw, std::size_t rawIntCount, int_t vertexCount,
                           int_t drawMode, bool hasTexture, bool hasColor, bool hasNormals)
{
    clearKeepCapacity();

    const std::size_t slotsPerVertex = Ps2CaptureLayout::Slots;
    if (raw == nullptr || vertexCount <= 0 || drawMode != PS2_NATIVE_PRIM_QUADS ||
        (vertexCount & 3) != 0 || !hasTexture || hasNormals ||
        rawIntCount < (std::size_t)vertexCount * slotsPerVertex)
    {
        ++s_buildStats.invalidInput;
        return false;
    }

    try
    {
        const std::size_t positionBytes = (std::size_t)vertexCount * 4u * sizeof(short);
        const std::size_t texCoordBytes = (std::size_t)vertexCount * 2u * sizeof(short);
        const std::size_t colorBytes = (std::size_t)vertexCount * 4u;
        const std::size_t totalBytes = positionBytes + texCoordBytes + colorBytes + kStreamAlignment * 4u;
        if (totalBytes > m_storage.capacity())
        {
            std::vector<unsigned char>().swap(m_storage);
            m_storage.reserve(totalBytes);
        }
        m_storage.resize(totalBytes);

        const std::uintptr_t storageBase = reinterpret_cast<std::uintptr_t>(m_storage.data());
        std::uintptr_t cursor = alignUp(storageBase, kStreamAlignment);
        m_positionOffset = (std::size_t)(cursor - storageBase);
        cursor = alignUp(cursor + positionBytes, kStreamAlignment);
        m_texCoordOffset = (std::size_t)(cursor - storageBase);
        cursor = alignUp(cursor + texCoordBytes, kStreamAlignment);
        m_colorOffset = (std::size_t)(cursor - storageBase);

        short* positionOut = reinterpret_cast<short*>(m_storage.data() + m_positionOffset);
        short* uvOut = reinterpret_cast<short*>(m_storage.data() + m_texCoordOffset);
        unsigned char* colorOut = m_storage.data() + m_colorOffset;

        const std::size_t requiredRuns = (std::size_t)vertexCount / 4u;
        if (requiredRuns > m_runs.capacity())
        {
            std::vector<Ps2TerrainTileRun>().swap(m_runs);
            m_runs.reserve(requiredRuns);
        }
        int currentTileX = -1;
        int currentTileY = -1;
        static const int kStripOrder[4] = { 1, 2, 0, 3 };

        for (int_t vertex = 0; vertex < vertexCount; ++vertex)
        {
            const int_t sourceVertex = (vertex & ~3) + kStripOrder[vertex & 3];
            const std::size_t base = (std::size_t)sourceVertex * slotsPerVertex;
            const float x = slotFloat(raw, base + 0u);
            const float y = slotFloat(raw, base + 1u);
            const float z = slotFloat(raw, base + 2u);
            const float u = slotFloat(raw, base + 3u);
            const float v = slotFloat(raw, base + 4u);

            positionOut[(std::size_t)vertex * 4u + 0u] = packSigned16((x - 8.0f) * kPositionScale);
            positionOut[(std::size_t)vertex * 4u + 1u] = packSigned16((y - 8.0f) * kPositionScale);
            positionOut[(std::size_t)vertex * 4u + 2u] = packSigned16((z - 8.0f) * kPositionScale);
            positionOut[(std::size_t)vertex * 4u + 3u] = 4096;
            uvOut[(std::size_t)vertex * 2u + 0u] = packSigned16(u * kUvScale);
            uvOut[(std::size_t)vertex * 2u + 1u] = packSigned16(v * kUvScale);

            if (hasColor)
            {
                const unsigned char* packed = reinterpret_cast<const unsigned char*>(raw + base + 5u);
                colorOut[(std::size_t)vertex * 4u + 0u] = gsColor(packed[0]);
                colorOut[(std::size_t)vertex * 4u + 1u] = gsColor(packed[1]);
                colorOut[(std::size_t)vertex * 4u + 2u] = gsColor(packed[2]);
                colorOut[(std::size_t)vertex * 4u + 3u] = (unsigned char)(packed[3] >> 1);
            }
            else
            {
                colorOut[(std::size_t)vertex * 4u + 0u] = 128;
                colorOut[(std::size_t)vertex * 4u + 1u] = 128;
                colorOut[(std::size_t)vertex * 4u + 2u] = 128;
                colorOut[(std::size_t)vertex * 4u + 3u] = 127;
            }

            if ((vertex & 3) == 0)
            {
                float minU = u;
                float minV = v;
                for (int corner = 0; corner < 4; ++corner)
                {
                    const std::size_t cornerBase = (std::size_t)(vertex + corner) * slotsPerVertex;
                    const float cornerU = slotFloat(raw, cornerBase + 3u);
                    const float cornerV = slotFloat(raw, cornerBase + 4u);
                    if (cornerU < minU) minU = cornerU;
                    if (cornerV < minV) minV = cornerV;
                }

                const int tileX = atlasTile(minU);
                const int tileY = atlasTile(minV);
                if (tileX != currentTileX || tileY != currentTileY)
                {
                    Ps2TerrainTileRun run;
                    run.firstVertex = vertex;
                    run.vertexCount = 4;
                    run.tileX = (unsigned char)tileX;
                    run.tileY = (unsigned char)tileY;
                    m_runs.push_back(run);
                    currentTileX = tileX;
                    currentTileY = tileY;
                }
                else
                {
                    m_runs.back().vertexCount += 4;
                }
            }
        }

        SyncDCache(positionOut, reinterpret_cast<unsigned char*>(positionOut) + positionBytes);
        SyncDCache(uvOut, reinterpret_cast<unsigned char*>(uvOut) + texCoordBytes);
        SyncDCache(colorOut, colorOut + colorBytes);

        m_vertexCount = vertexCount;
        m_valid = !m_runs.empty();
        if (!m_valid)
        {
            ++s_buildStats.emptyRuns;
            return false;
        }

        ++s_buildStats.successful;
        return true;
    }
    catch (const std::bad_alloc&)
    {
        ++s_buildStats.allocationFailed;
        release();
        return false;
    }
}

void Ps2TerrainMesh::clearKeepCapacity()
{
    m_storage.clear();
    m_runs.clear();
    m_positionOffset = 0;
    m_texCoordOffset = 0;
    m_colorOffset = 0;
    m_vertexCount = 0;
    m_valid = false;
}

void Ps2TerrainMesh::release()
{
    std::vector<unsigned char>().swap(m_storage);
    std::vector<Ps2TerrainTileRun>().swap(m_runs);
    m_positionOffset = 0;
    m_texCoordOffset = 0;
    m_colorOffset = 0;
    m_vertexCount = 0;
    m_valid = false;
}

#endif // PS2_PLATFORM
