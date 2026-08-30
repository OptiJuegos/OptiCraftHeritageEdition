#ifdef PS2_PLATFORM

#include "ps2/render/Ps2MeshSort.h"

#include "ps2/render/Ps2CaptureLayout.h"
#include "platform/PlatformTuning.h"

#include <algorithm>
#include <math.h>
#include <string.h>

namespace
{
	// Capture layout: x y z u v rgba, one int_t each.
	const int kSlots = (int)Ps2CaptureLayout::Slots;
	const int kQuadInts = kSlots * 4;

	// Atlas cells along one axis of the 256x256 terrain atlas. This has to match
	// the granularity ps2_select_clamp keys REGION_REPEAT on (ufix = texel & ~15),
	// because the whole point of the grouping is that quads sharing a group need
	// no clamp change between them.
	const int kTilesPerAxis = 16;
	const int kTileCount = kTilesPerAxis * kTilesPerAxis; // 256
	const int kKeyCount =
		kTileCount * PS2_MESH_CLUSTER_COUNT * PS2_FACE_GROUP_COUNT;
	const float kSectionSize = 16.0f;

	static_assert(PS2_MESH_CLUSTER_AXIS > 0,
		"PS2 mesh clustering needs at least one cell per axis");
	static_assert((16 % PS2_MESH_CLUSTER_AXIS) == 0,
		"PS2 mesh clusters must divide a 16-block section evenly");
	// The full sort key includes the 256 atlas tiles and therefore exceeds
	// 16 bits with 4x4x4 clusters. Only the temporary sorter needs that full
	// key; retained Ps2MeshRange metadata stores face+cluster separately below.
	static_assert(PS2_FACE_GROUP_COUNT * PS2_MESH_CLUSTER_COUNT <= 65536,
		"PS2 retained face/cluster key must fit in unsigned short");
	static_assert(PLATFORM_MESH_SORT_MAX_QUADS <= 65535,
		"PS2 per-key quad counts must fit in unsigned short");
	static_assert(sizeof(Ps2MeshRange) == 8,
		"PS2 mesh range metadata must remain compact");

	static_assert(kKeyCount > 0, "PS2 sort key space must be non-empty");

	// Static, not stack: the EE stack is small and this runs from the render
	// path, single-threaded, once per completed section.
	//
	// Why these are keyed by quad and not by sort key
	// -----------------------------------------------
	// The key spans kKeyCount = 114688 values with 4x4x4 clustering. A
	// direct-indexed histogram over that range is 224KB of BSS, and every
	// completed section paid a 224KB memset plus a 114688-entry prefix-sum walk
	// -- for a mesh that measures 500-1500 quads. Over 99% of both was spent on
	// keys no quad in the section used.
	//
	// Only the keys that occur matter, and there are at most quadCount of them.
	// Sorting the per-quad keys makes the distinct set and its run lengths fall
	// out of a single linear scan, and ascending key order IS tile -> face ->
	// cluster order by construction (the key is
	// (tile * groups + face) * clusters + cluster, strictly monotonic in that
	// lexicographic order), so the emitted layout is identical to the one the
	// dense histogram produced -- this is a cost change, not a behaviour change.
	unsigned int s_key[PLATFORM_MESH_SORT_MAX_QUADS];
	// Sorted keys during the scan, then compacted in place to the distinct keys.
	unsigned int s_distinctKey[PLATFORM_MESH_SORT_MAX_QUADS];
	// Write cursor of each distinct key, parallel to s_distinctKey. Bounded by
	// quadCount, which the static_assert above pins under 65536.
	unsigned short s_distinctBase[PLATFORM_MESH_SORT_MAX_QUADS];

	inline float asFloat(int_t bits)
	{
		float f;
		__builtin_memcpy(&f, &bits, sizeof(f));
		return f;
	}

	// Which 16x16 atlas cell a quad samples, from the minimum corner of its UV
	// box -- the same corner ps2_select_clamp truncates to build ufix/vfix.
	inline int tileIndex(float minU, float minV)
	{
		int tu = (int)(minU * 256.0f) >> 4;
		int tv = (int)(minV * 256.0f) >> 4;
		if (tu < 0) tu = 0; else if (tu >= kTilesPerAxis) tu = kTilesPerAxis - 1;
		if (tv < 0) tv = 0; else if (tv >= kTilesPerAxis) tv = kTilesPerAxis - 1;
		return tv * kTilesPerAxis + tu;
	}

	inline int clusterCoord(float center)
	{
		int cell = (int)(center * ((float)PS2_MESH_CLUSTER_AXIS / kSectionSize));
		if (cell < 0) cell = 0;
		if (cell >= PS2_MESH_CLUSTER_AXIS) cell = PS2_MESH_CLUSTER_AXIS - 1;
		return cell;
	}

	inline int clusterIndex(float minX, float minY, float minZ,
	                       float maxX, float maxY, float maxZ)
	{
		const int x = clusterCoord((minX + maxX) * 0.5f);
		const int y = clusterCoord((minY + maxY) * 0.5f);
		const int z = clusterCoord((minZ + maxZ) * 0.5f);
		return (z * PS2_MESH_CLUSTER_AXIS + y) * PS2_MESH_CLUSTER_AXIS + x;
	}
}

void Ps2FaceGroups::reset()
{
	valid = false;
	ranges.clear();
	for (int g = 0; g < PS2_FACE_GROUP_COUNT; g++)
	{
		planeMin[g] = 0.0f;
		planeMax[g] = 0.0f;
	}
	for (int c = 0; c < PS2_MESH_CLUSTER_COUNT; c++)
	{
		clusters[c].minX = clusters[c].minY = clusters[c].minZ = 1e30f;
		clusters[c].maxX = clusters[c].maxY = clusters[c].maxZ = -1e30f;
	}
}

void Ps2FaceGroups::release()
{
	std::vector<Ps2MeshRange>().swap(ranges);
	reset();
}

bool ps2_mesh_sort_faces(const int_t *src, int_t *dst, int_t quadCount, Ps2FaceGroups &groups)
{
	groups.reset();

	if (src == nullptr || dst == nullptr || quadCount <= 0)
		return false;
	if (quadCount > (int_t)PLATFORM_MESH_SORT_MAX_QUADS)
		return false;

	float planeMin[PS2_FACE_GROUP_COUNT];
	float planeMax[PS2_FACE_GROUP_COUNT];
	for (int g = 0; g < PS2_FACE_GROUP_COUNT; g++)
	{
		planeMin[g] = 1e30f;
		planeMax[g] = -1e30f;
	}

	// ---- Pass 1: classify every quad, build the histogram ----
	for (int_t q = 0; q < quadCount; q++)
	{
		const int_t *v = src + (size_t)q * kQuadInts;

		const float x0 = asFloat(v[0]),               y0 = asFloat(v[1]),               z0 = asFloat(v[2]);
		const float x1 = asFloat(v[kSlots + 0]),      y1 = asFloat(v[kSlots + 1]),      z1 = asFloat(v[kSlots + 2]);
		const float x2 = asFloat(v[2 * kSlots + 0]),  y2 = asFloat(v[2 * kSlots + 1]),  z2 = asFloat(v[2 * kSlots + 2]);
		const float x3 = asFloat(v[3 * kSlots + 0]),  y3 = asFloat(v[3 * kSlots + 1]),  z3 = asFloat(v[3 * kSlots + 2]);
		float minX = x0, minY = y0, minZ = z0;
		float maxX = x0, maxY = y0, maxZ = z0;
		const float quadX[3] = { x1, x2, x3 };
		const float quadY[3] = { y1, y2, y3 };
		const float quadZ[3] = { z1, z2, z3 };
		for (int corner = 0; corner < 3; corner++)
		{
			if (quadX[corner] < minX) minX = quadX[corner];
			if (quadX[corner] > maxX) maxX = quadX[corner];
			if (quadY[corner] < minY) minY = quadY[corner];
			if (quadY[corner] > maxY) maxY = quadY[corner];
			if (quadZ[corner] < minZ) minZ = quadZ[corner];
			if (quadZ[corner] > maxZ) maxZ = quadZ[corner];
		}

		// Outward normal. Minecraft winds a face so that (p1-p0)x(p2-p0) points
		// away from the block -- verified against RenderBlocks::renderTopFace,
		// whose vertex order (x1,y1,z1)(x1,y1,z0)(x0,y1,z0)(x0,y1,z1) yields
		// +Y, and renderBottomFace, which yields -Y. The GL winding convention
		// is not involved: this is the geometric normal, and the visibility test
		// below is pure geometry.
		const float ax = x1 - x0, ay = y1 - y0, az = z1 - z0;
		const float bx = x2 - x0, by = y2 - y0, bz = z2 - z0;
		const float nx = ay * bz - az * by;
		const float ny = az * bx - ax * bz;
		const float nz = ax * by - ay * bx;

		const float anx = fabsf(nx), any = fabsf(ny), anz = fabsf(nz);

		int bucket = PS2_FACE_OTHER;
		float plane = 0.0f;

		// Dominant axis, then confirm the face really is axis-aligned: the other
		// two normal components negligible AND all four corners on one plane.
		// Anything else (crossed squares, torches, rails, ladders, the fluid
		// surface slope) goes to PS2_FACE_OTHER and is always drawn.
		if (anx >= any && anx >= anz)
		{
			if (anx > 1e-6f && any <= 1e-3f * anx && anz <= 1e-3f * anx &&
			    fabsf(x1 - x0) <= 1e-4f && fabsf(x2 - x0) <= 1e-4f && fabsf(x3 - x0) <= 1e-4f)
			{
				bucket = (nx > 0.0f) ? PS2_FACE_XP : PS2_FACE_XN;
				plane = x0;
			}
		}
		else if (any >= anz)
		{
			if (any > 1e-6f && anx <= 1e-3f * any && anz <= 1e-3f * any &&
			    fabsf(y1 - y0) <= 1e-4f && fabsf(y2 - y0) <= 1e-4f && fabsf(y3 - y0) <= 1e-4f)
			{
				bucket = (ny > 0.0f) ? PS2_FACE_YP : PS2_FACE_YN;
				plane = y0;
			}
		}
		else
		{
			if (anz > 1e-6f && anx <= 1e-3f * anz && any <= 1e-3f * anz &&
			    fabsf(z1 - z0) <= 1e-4f && fabsf(z2 - z0) <= 1e-4f && fabsf(z3 - z0) <= 1e-4f)
			{
				bucket = (nz > 0.0f) ? PS2_FACE_ZP : PS2_FACE_ZN;
				plane = z0;
			}
		}

		if (bucket != PS2_FACE_OTHER)
		{
			if (plane < planeMin[bucket]) planeMin[bucket] = plane;
			if (plane > planeMax[bucket]) planeMax[bucket] = plane;
		}

		// UV box minimum over the four corners, matching ensureClamp.
		float minU = asFloat(v[3]), minV = asFloat(v[4]);
		for (int c = 1; c < 4; c++)
		{
			const float u = asFloat(v[c * kSlots + 3]);
			const float w = asFloat(v[c * kSlots + 4]);
			if (u < minU) minU = u;
			if (w < minV) minV = w;
		}

		const int cluster = clusterIndex(minX, minY, minZ, maxX, maxY, maxZ);
		Ps2MeshCluster &clusterBounds = groups.clusters[cluster];
		if (minX < clusterBounds.minX) clusterBounds.minX = minX;
		if (minY < clusterBounds.minY) clusterBounds.minY = minY;
		if (minZ < clusterBounds.minZ) clusterBounds.minZ = minZ;
		if (maxX > clusterBounds.maxX) clusterBounds.maxX = maxX;
		if (maxY > clusterBounds.maxY) clusterBounds.maxY = maxY;
		if (maxZ > clusterBounds.maxZ) clusterBounds.maxZ = maxZ;

		const int tile = tileIndex(minU, minV);
		const unsigned int key = (unsigned int)(
			(tile * PS2_FACE_GROUP_COUNT + bucket) * PS2_MESH_CLUSTER_COUNT + cluster);
		s_key[q] = key;
		s_distinctKey[q] = key;
	}

	for (int g = 0; g < PS2_FACE_GROUP_COUNT; g++)
	{
		if (planeMin[g] <= planeMax[g] && g != PS2_FACE_OTHER)
		{
			groups.planeMin[g] = planeMin[g];
			groups.planeMax[g] = planeMax[g];
		}
	}

	// ---- Prefix sum over the keys that actually occur ----
	// Sorting collects equal keys into runs, so one scan yields both the
	// distinct key set and each run's length. Ascending key order is tile ->
	// face -> cluster order, which is the order the ranges must be emitted in
	// and the order the destination buffer must be laid out in.
	std::sort(s_distinctKey, s_distinctKey + quadCount);

	int distinctCount = 0;
	int_t running = 0;
	for (int_t q = 0; q < quadCount; )
	{
		const unsigned int key = s_distinctKey[q];
		int_t end = q + 1;
		while (end < quadCount && s_distinctKey[end] == key)
			++end;
		const int_t n = end - q;

		Ps2MeshRange range;
		range.firstVertex = running * 4;
		range.quadCount = (unsigned short)n;
		// Tile is needed only for ordering. Draw-time classification needs
		// face+cluster, which remains compact even with 64 clusters, and the
		// key was built as (tile * groups + face) * clusters + cluster -- so
		// dropping the tile is one modulo.
		range.sortKey = (unsigned short)(
			key % (unsigned int)(PS2_FACE_GROUP_COUNT * PS2_MESH_CLUSTER_COUNT));
		groups.ranges.push_back(range);

		// Compact in place: distinctCount never overtakes q.
		s_distinctKey[distinctCount] = key;
		s_distinctBase[distinctCount] = (unsigned short)running;
		++distinctCount;

		running += n;
		q = end;
	}

	// ---- Pass 2: scatter whole quads into their slot ----
	// s_distinctKey is sorted and deduplicated, so the key a quad carries is
	// found by binary search instead of the direct index the dense histogram
	// allowed. At 500-1500 quads that is ten compares against a table that
	// stays in cache, versus the 224KB the direct index cost to clear.
	for (int_t q = 0; q < quadCount; q++)
	{
		const unsigned int key = s_key[q];
		int lo = 0;
		int hi = distinctCount - 1;
		while (lo < hi)
		{
			const int mid = (lo + hi) >> 1;
			if (s_distinctKey[mid] < key)
				lo = mid + 1;
			else
				hi = mid;
		}
		const int_t d = (int_t)s_distinctBase[lo]++;
		memcpy(dst + (size_t)d * kQuadInts,
		       src + (size_t)q * kQuadInts,
		       (size_t)kQuadInts * sizeof(int_t));
	}

	groups.valid = running == quadCount && !groups.ranges.empty();
	return groups.valid;
}

#endif // PS2_PLATFORM
