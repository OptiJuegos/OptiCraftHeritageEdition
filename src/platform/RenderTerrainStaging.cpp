#include "platform/RenderTerrainStaging.h"
#include "platform/PlatformConfig.h"

#if PLATFORM_PS2
#include "ps2/render/Ps2MeshStagingPool.h"
#endif

int renderTerrainStagingAcquire()
{
#if PLATFORM_PS2
    return ps2_mesh_staging_acquire();
#else
    return RENDER_TERRAIN_STAGING_INVALID_SLOT;
#endif
}

void renderTerrainStagingRelease(int slot)
{
#if PLATFORM_PS2
    ps2_mesh_staging_release(slot);
#else
    (void)slot;
#endif
}

std::vector<int_t>* renderTerrainStagingBuffers(int slot)
{
#if PLATFORM_PS2
    return ps2_mesh_staging_buffers(slot);
#else
    (void)slot;
    return nullptr;
#endif
}

int renderTerrainStagingSlotsInUse()
{
#if PLATFORM_PS2
    return ps2_mesh_staging_slots_in_use();
#else
    return 0;
#endif
}
