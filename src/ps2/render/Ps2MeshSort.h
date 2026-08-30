#pragma once

#ifdef PS2_PLATFORM

#include <cstddef>
#include <vector>

#include "java/Type.h"
#include "ps2/render/Ps2Tuning.h"

// ---- Opaque section mesh reordering ----
//
// A finished pass-0 chunk mesh is a flat run of GL_QUADS in block-scan order.
// That order is the worst possible one for this renderer, for two independent
// reasons measured in the [PS2] stats build:
//
//   1. Consecutive quads almost never share an atlas cell, and ps2_select_clamp
//      puts every 3D atlas face in REGION_REPEAT keyed to its own 16x16 cell, so
//      each change flushes the strip batch. Measured: clampSet=2894 against
//      stripFlush=2900 -- every packet cut short -- and quadsPerFlush=12 out of
//      a possible 32.
//
//   2. Every face is axis-aligned, so roughly half of them are backfacing from
//      any given eye position, but nothing knows that until each one has been
//      through 4 VU0 transforms, 4 reciprocals and a signed-area test.
//
// ps2_mesh_sort_faces reorders whole quads (never vertices within a quad, never
// the geometry) into
//
//     atlas tile  ->  face direction bucket  ->  spatial cluster
//
// Tile-first order minimizes REGION_REPEAT changes, while face-before-cluster
// keeps all visible cells of one direction contiguous for direct VU1. Compact
// range metadata
// preserves the cluster and face identities, so the draw path can still reject
// whole backface groups and can classify 8x8x8 cluster AABBs before deciding
// between direct VU1 and the clipped VU0 path. Pass 0 is opaque and depth-tested,
// so the draw order it destroys was worth nothing. Pass 1 must NOT be reordered:
// blending depends on back-to-front order.

// Opposite face pairs remain adjacent within every atlas tile, allowing the draw
// path to merge neighbouring visible directions when no cluster decision splits
// them.
enum
{
	PS2_FACE_XP = 0, // outward normal +X
	PS2_FACE_XN,     // -X
	PS2_FACE_YP,     // +Y (block tops)
	PS2_FACE_YN,     // -Y (block bottoms)
	PS2_FACE_ZP,     // +Z
	PS2_FACE_ZN,     // -Z
	PS2_FACE_OTHER,  // not axis-aligned: crossed squares, torches, rails, ladders
	PS2_FACE_GROUP_COUNT
};

enum
{
	PS2_MESH_CLUSTER_COUNT =
		PS2_MESH_CLUSTER_AXIS * PS2_MESH_CLUSTER_AXIS * PS2_MESH_CLUSTER_AXIS
};

struct Ps2MeshCluster
{
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;

	bool valid() const
	{
		return minX <= maxX && minY <= maxY && minZ <= maxZ;
	}
};

// One non-empty tile/cluster/face run in the reordered vertex buffer. The
// temporary sorter uses tile+face+cluster, but the retained 16-bit sortKey stores
// only face+cluster: tile is already represented by physical ordering and is not
// needed by draw-time culling. This keeps each range at eight bytes even with
// 4x4x4 spatial clusters. PLATFORM_MESH_SORT_MAX_QUADS is 4096.
struct Ps2MeshRange
{
	int_t firstVertex;
	unsigned short quadCount;
	unsigned short sortKey;

	int_t vertexCount() const { return (int_t)quadCount * 4; }
	int faceGroup() const
	{
		return ((int)sortKey / PS2_MESH_CLUSTER_COUNT) % PS2_FACE_GROUP_COUNT;
	}
	int cluster() const { return (int)sortKey % PS2_MESH_CLUSTER_COUNT; }
};

struct Ps2FaceGroups
{
	// false = this mesh was not reordered (too large, wrong draw mode, feature
	// off). Callers must then draw the whole buffer as one range, exactly as
	// before.
	bool valid;

	// Ordered, non-overlapping ranges covering the entire reordered buffer.
	std::vector<Ps2MeshRange> ranges;

	// Exact union of the geometry assigned to each spatial cluster. These are
	// deliberately geometry bounds, not fixed cell bounds: a quad crossing an
	// 8-block boundary expands its cluster and remains safe to classify.
	Ps2MeshCluster clusters[PS2_MESH_CLUSTER_COUNT];

	// Section-local plane coordinate range of the faces in each axis-aligned
	// bucket, along that bucket's own axis. planeMin/planeMax are meaningless
	// for PS2_FACE_OTHER and for empty buckets.
	float planeMin[PS2_FACE_GROUP_COUNT];
	float planeMax[PS2_FACE_GROUP_COUNT];

	void reset();
	void release();
	std::size_t ramBytes() const
	{
		return ranges.capacity() * sizeof(Ps2MeshRange);
	}
};

// Reorder quadCount quads from src into dst.
//
// Both buffers hold Ps2CaptureLayout::Slots 32-bit slots per vertex and 4
// vertices per quad; they must not overlap, and dst must already have room for
// quadCount * 4 * Ps2CaptureLayout::Slots slots.
//
// Returns false and writes nothing to dst when the mesh cannot be reordered (0
// quads, or more than PLATFORM_MESH_SORT_MAX_QUADS); groups.valid mirrors the
// return value.
bool ps2_mesh_sort_faces(const int_t *src, int_t *dst, int_t quadCount, Ps2FaceGroups &groups);

#endif // PS2_PLATFORM
