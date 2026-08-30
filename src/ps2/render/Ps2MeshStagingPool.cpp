#include "ps2/render/Ps2MeshStagingPool.h"

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2Tuning.h"

namespace
{
	struct Ps2MeshStagingSlot
	{
		std::vector<int_t> buffer[2];
		bool leased;
	};

	// File-scope rather than a heap allocation: the pool exists for the whole
	// session and its size is a compile-time constant, so there is nothing for
	// an arena this fragmentation-sensitive to gain from putting the array of
	// vector headers on the heap. The vectors' own storage is still heap.
	Ps2MeshStagingSlot s_slots[PS2_MESH_STAGING_SLOTS];
	int s_leasedCount = 0;
}

int ps2_mesh_staging_acquire()
{
	for (int slot = 0; slot < PS2_MESH_STAGING_SLOTS; ++slot)
	{
		if (s_slots[slot].leased)
			continue;

		// Normal capacities are retained between leases. Oversized high-water
		// buffers are reclaimed on release, so an acquired slot is already within
		// the configured retention bound here.
		s_slots[slot].buffer[0].clear();
		s_slots[slot].buffer[1].clear();
		s_slots[slot].leased = true;
		++s_leasedCount;
		return slot;
	}
	return PS2_MESH_STAGING_INVALID_SLOT;
}

void ps2_mesh_staging_release(int slot)
{
	if (slot < 0 || slot >= PS2_MESH_STAGING_SLOTS || !s_slots[slot].leased)
		return;

	for (int pass = 0; pass < 2; ++pass)
	{
		const std::size_t retainedBytes = s_slots[slot].buffer[pass].capacity() * sizeof(int_t);
		if (retainedBytes > PS2_MAX_RETAINED_RAW_MESH_BYTES)
			std::vector<int_t>().swap(s_slots[slot].buffer[pass]);
		else
			s_slots[slot].buffer[pass].clear();
	}
	s_slots[slot].leased = false;
	if (s_leasedCount > 0)
		--s_leasedCount;
}

std::vector<int_t> *ps2_mesh_staging_buffers(int slot)
{
	if (slot < 0 || slot >= PS2_MESH_STAGING_SLOTS || !s_slots[slot].leased)
		return nullptr;
	return s_slots[slot].buffer;
}

std::size_t ps2_mesh_staging_ram_bytes()
{
	std::size_t bytes = 0;
	for (int slot = 0; slot < PS2_MESH_STAGING_SLOTS; ++slot)
	{
		bytes += s_slots[slot].buffer[0].capacity() * sizeof(int_t);
		bytes += s_slots[slot].buffer[1].capacity() * sizeof(int_t);
	}
	return bytes;
}

int ps2_mesh_staging_slots_in_use()
{
	return s_leasedCount;
}

#endif // PS2_PLATFORM
