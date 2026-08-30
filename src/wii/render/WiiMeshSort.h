#pragma once

#ifdef WII_PLATFORM

#include <vector>

#include "java/Type.h"

// Where each face direction landed in the reordered buffer, and how far its
// quads reach along the axis they face. Both are section-local, in the same
// space as the vertex positions the sort read.
//
// The plane extents are what makes the reorder worth its rebuild cost: they let
// the draw decide, per section and per direction, that every quad in a group
// faces away from the eye. GX culls those triangles anyway, but only after the
// vertices have gone through the write-gather pipe and the transform unit,
// which is the part being paid for here.
struct WiiFaceGroupRanges
{
    static const int kGroupCount = 7;

    // Quads per group, in the order the sort emits them: +X, -X, +Y, -Y, +Z,
    // -Z, then everything whose geometry is not an axis-aligned plane (stairs,
    // fences, crossed plants). The last group can never be culled.
    int_t quadCount[kGroupCount];

    // Extent of the group along axis (group >> 1). Meaningless for kGroupCount-1
    // and for an empty group; both are left at zero and their quadCount says so.
    float planeMin[kGroupCount];
    float planeMax[kGroupCount];
};

// Reorders opaque terrain quads by their geometric face direction. The Wii
// terrain atlas is bound once for the whole pass, so unlike the PS2 path there
// is no benefit in sorting again by atlas tile.
//
// outRanges is optional; pass null when only the reorder is wanted. It is
// written only on success, and its quadCount entries always sum to quadCount.
bool wii_sort_opaque_faces(const std::vector<int_t>& src, std::vector<int_t>& dst,
                           int_t quadCount, int_t maxQuads,
                           WiiFaceGroupRanges* outRanges = nullptr);

#endif
