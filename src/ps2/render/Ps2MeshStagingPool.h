#pragma once

#ifdef PS2_PLATFORM

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "java/Type.h"

// Shared staging buffers for in-progress chunk-section meshes.
//
// A WorldRenderer builds its replacement mesh over many frames, so the staging
// buffer has to survive between steps -- but only for a renderer that is
// actually mid-build. The grid holds 75 renderers and the measured completion
// rate is around 30 per 120 frames, so giving every renderer its own pair of
// staging vectors kept roughly 2 MB of a 32 MB budget in buffers belonging to
// builds that were not running. Measured 2026-08-21 in the [PS2][FRAME] line:
// mesh= tracked mallocUsed to within 8-18% over four consecutive periods, i.e.
// section meshes were essentially the whole of the arena's growth.
//
// The buffers are leased instead: a renderer takes a slot when its build
// begins and returns it when the build completes or is abandoned. A build that
// finds no free slot simply defers to a later frame, which costs streaming
// latency and nothing else.
//
// Why leasing does not reintroduce the allocator churn the per-renderer buffers
// were protecting against (see WorldRenderer::ps2BuildRendererStep): normal
// capacities survive release, while only allocations above the PS2 retention
// cap are returned. What stays constant is the slot count instead of one pair
// per renderer.


struct Ps2MeshSectionCache
{
	enum
	{
		BlockCount = 4096,
		RowCount = 16 * 16,
		FaceCount = 6
	};

	std::array<std::uint16_t, BlockCount> blockIds;
	// Per-block tint for greedy cubes whose color multiplier is world-dependent.
	// Fast leaves are the hot case: compute their biome tint once per section
	// build instead of once for every exposed face across six sliced passes.
	std::array<std::uint32_t, BlockCount> colorMultipliers;
	std::array<std::uint16_t, RowCount> greedyRows;
	std::array<std::uint16_t, RowCount> occluderRows;
	std::array<std::array<std::uint16_t, RowCount>, FaceCount> visibleRows;
	int_t originX;
	int_t originY;
	int_t originZ;
	bool valid;
};

// Returned by ps2_mesh_staging_acquire() when every slot is leased.
const int PS2_MESH_STAGING_INVALID_SLOT = -1;

// Takes a free slot, or PS2_MESH_STAGING_INVALID_SLOT when the pool is full.
// The slot's two buffers are emptied (capacity retained) before it is handed
// over, so a caller never sees a previous build's vertices.
int ps2_mesh_staging_acquire();

// Returns a slot to the pool. Safe to call with an invalid slot, so the
// abandon paths do not each need their own guard.
void ps2_mesh_staging_release(int slot);

// The leased buffers for a slot: index 0 is the opaque pass, 1 the translucent
// one. Null for an invalid or unleased slot, which is what makes a missing
// lease a null dereference at the use site rather than silent corruption of
// another renderer's staging mesh.
std::vector<int_t> *ps2_mesh_staging_buffers(int slot);

// Section-local block and face-mask cache owned by the same staging lease.
// The cache is invalidated on acquire/release and survives incremental build
// steps without adding permanent storage to every WorldRenderer.
Ps2MeshSectionCache *ps2_mesh_staging_section_cache(int slot);

// Total bytes held by the pool, so RenderGlobal::terrainMeshRamBytes() keeps
// reporting the same quantity it did when the buffers were per-renderer.
std::size_t ps2_mesh_staging_ram_bytes();

// Slots currently leased. Diagnostic only: a value pinned at the slot count
// means builds are being deferred for want of a buffer.
int ps2_mesh_staging_slots_in_use();

// Whether ps2_mesh_staging_acquire() can currently hand out a slot. This is a
// scheduling predicate, not a diagnostic: a renderer that is not already mid
// build cannot do any work while the pool is full, so the caller can stop
// attempting such renderers instead of paying a full source scan per attempt
// only to be turned away at ps2BeginBuildState().
bool ps2_mesh_staging_has_free_slot();

#endif // PS2_PLATFORM
