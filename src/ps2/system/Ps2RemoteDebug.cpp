#ifdef PS2_PLATFORM

#include "ps2/system/Ps2RemoteDebug.h"

#include <ee_regs.h>

#include "platform/Log.h"
#include "ps2/render/Ps2GsQueue.h"

namespace Ps2RemoteDebug
{
void checkpoint(const char* site)
{
#ifdef PS2_REMOTE_DEBUG
    const long queueUsed = ps2_gs_queue_used_bytes();
    const long queueCapacity = ps2_gs_queue_capacity_bytes();
    const long queueOverflows = ps2_gs_queue_overflow_count();

    MC_LOG_DEBUG(
        "ps2.remote",
        "[PS2] HW %s: D1_CHCR=%08x D1_MADR=%08x D1_QWC=%08x D1_TADR=%08x "
        "VIF1_STAT=%08x VIF1_ERR=%08x VIF1_CODE=%08x VIF1_NUM=%08x "
        "GIF_STAT=%08x D2_CHCR=%08x D_STAT=%08x GSQ=%ld/%ld over=%ld\n",
        site != nullptr ? site : "?",
        (unsigned int)*R_EE_D1_CHCR,
        (unsigned int)*R_EE_D1_MADR,
        (unsigned int)*R_EE_D1_QWC,
        (unsigned int)*R_EE_D1_TADR,
        (unsigned int)*R_EE_VIF1_STAT,
        (unsigned int)*R_EE_VIF1_ERR,
        (unsigned int)*R_EE_VIF1_CODE,
        (unsigned int)*R_EE_VIF1_NUM,
        (unsigned int)*R_EE_GIF_STAT,
        (unsigned int)*R_EE_D2_CHCR,
        (unsigned int)*R_EE_D_STAT,
        queueUsed, queueCapacity, queueOverflows);
#else
    (void)site;
#endif
}
}

#endif // PS2_PLATFORM
