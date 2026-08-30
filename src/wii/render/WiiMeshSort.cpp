#ifdef WII_PLATFORM

#include "wii/render/WiiMeshSort.h"

#include <cmath>
#include <cstring>

namespace
{
    constexpr int kSlotsPerVertex = 8;
    constexpr int kVerticesPerQuad = 4;
    constexpr int kIntsPerQuad = kSlotsPerVertex * kVerticesPerQuad;
    constexpr int kFaceGroups = 7;

    enum FaceGroup
    {
        FaceXp = 0,
        FaceXn,
        FaceYp,
        FaceYn,
        FaceZp,
        FaceZn,
        FaceOther
    };

    float asFloat(int_t bits)
    {
        float value;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    int classifyQuad(const int_t* quad)
    {
        const float x0 = asFloat(quad[0]);
        const float y0 = asFloat(quad[1]);
        const float z0 = asFloat(quad[2]);
        const float x1 = asFloat(quad[kSlotsPerVertex + 0]);
        const float y1 = asFloat(quad[kSlotsPerVertex + 1]);
        const float z1 = asFloat(quad[kSlotsPerVertex + 2]);
        const float x2 = asFloat(quad[2 * kSlotsPerVertex + 0]);
        const float y2 = asFloat(quad[2 * kSlotsPerVertex + 1]);
        const float z2 = asFloat(quad[2 * kSlotsPerVertex + 2]);
        const float x3 = asFloat(quad[3 * kSlotsPerVertex + 0]);
        const float y3 = asFloat(quad[3 * kSlotsPerVertex + 1]);
        const float z3 = asFloat(quad[3 * kSlotsPerVertex + 2]);

        const float ax = x1 - x0;
        const float ay = y1 - y0;
        const float az = z1 - z0;
        const float bx = x2 - x0;
        const float by = y2 - y0;
        const float bz = z2 - z0;
        const float nx = ay * bz - az * by;
        const float ny = az * bx - ax * bz;
        const float nz = ax * by - ay * bx;
        const float anx = std::fabs(nx);
        const float any = std::fabs(ny);
        const float anz = std::fabs(nz);

        if (anx >= any && anx >= anz)
        {
            if (anx > 1.0e-6f && any <= 1.0e-3f * anx && anz <= 1.0e-3f * anx &&
                std::fabs(x1 - x0) <= 1.0e-4f && std::fabs(x2 - x0) <= 1.0e-4f &&
                std::fabs(x3 - x0) <= 1.0e-4f)
                return nx > 0.0f ? FaceXp : FaceXn;
        }
        else if (any >= anz)
        {
            if (any > 1.0e-6f && anx <= 1.0e-3f * any && anz <= 1.0e-3f * any &&
                std::fabs(y1 - y0) <= 1.0e-4f && std::fabs(y2 - y0) <= 1.0e-4f &&
                std::fabs(y3 - y0) <= 1.0e-4f)
                return ny > 0.0f ? FaceYp : FaceYn;
        }
        else
        {
            if (anz > 1.0e-6f && anx <= 1.0e-3f * anz && any <= 1.0e-3f * anz &&
                std::fabs(z1 - z0) <= 1.0e-4f && std::fabs(z2 - z0) <= 1.0e-4f &&
                std::fabs(z3 - z0) <= 1.0e-4f)
                return nz > 0.0f ? FaceZp : FaceZn;
        }

        return FaceOther;
    }
}

bool wii_sort_opaque_faces(const std::vector<int_t>& src, std::vector<int_t>& dst,
                           int_t quadCount, int_t maxQuads,
                           WiiFaceGroupRanges* outRanges)
{
    if (quadCount <= 0 || maxQuads <= 0 || quadCount > maxQuads)
        return false;

    const size_t required = (size_t)quadCount * (size_t)kIntsPerQuad;
    if (src.size() != required)
        return false;

    static unsigned char groups[4096];
    if (quadCount > (int_t)(sizeof(groups) / sizeof(groups[0])))
        return false;

    int_t counts[kFaceGroups] = {};
    float planeMin[kFaceGroups];
    float planeMax[kFaceGroups];
    for (int g = 0; g < kFaceGroups; ++g)
    {
        planeMin[g] = 0.0f;
        planeMax[g] = 0.0f;
    }

    for (int_t q = 0; q < quadCount; ++q)
    {
        const int_t* quad = src.data() + (size_t)q * kIntsPerQuad;
        const int group = classifyQuad(quad);
        groups[q] = (unsigned char)group;

        if (group != FaceOther)
        {
            // classifyQuad() only returns an axis group after checking that all
            // four vertices share that coordinate, so the first vertex is the
            // plane. Track the extent while the quad is already in cache
            // instead of walking the buffer a second time.
            const float plane = asFloat(quad[group >> 1]);
            if (counts[group] == 0)
            {
                planeMin[group] = plane;
                planeMax[group] = plane;
            }
            else if (plane < planeMin[group])
                planeMin[group] = plane;
            else if (plane > planeMax[group])
                planeMax[group] = plane;
        }

        ++counts[group];
    }

    int_t cursors[kFaceGroups];
    int_t running = 0;
    for (int g = 0; g < kFaceGroups; ++g)
    {
        cursors[g] = running;
        running += counts[g];
    }

    dst.resize(required);
    for (int_t q = 0; q < quadCount; ++q)
    {
        const int group = groups[q];
        const int_t dstQuad = cursors[group]++;
        std::memcpy(dst.data() + (size_t)dstQuad * kIntsPerQuad,
                    src.data() + (size_t)q * kIntsPerQuad,
                    (size_t)kIntsPerQuad * sizeof(int_t));
    }

    if (outRanges != nullptr)
    {
        static_assert(WiiFaceGroupRanges::kGroupCount == kFaceGroups,
                      "face group count must match the classifier");
        for (int g = 0; g < kFaceGroups; ++g)
        {
            outRanges->quadCount[g] = counts[g];
            outRanges->planeMin[g] = planeMin[g];
            outRanges->planeMax[g] = planeMax[g];
        }
    }

    return true;
}

#endif
