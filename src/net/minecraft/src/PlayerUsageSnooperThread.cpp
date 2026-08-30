#include "PlayerUsageSnooperThread.h"

#include <thread>

#include "PlayerUsageSnooper.h"
#include "PostHttp.h"

PlayerUsageSnooperThread::PlayerUsageSnooperThread(PlayerUsageSnooper *usageSnooper)
    : snooper(usageSnooper)
{
}

void PlayerUsageSnooperThread::run()
{
    if (snooper == nullptr)
        return;
    PostHttp::func_52018_a(snooper->getUrl(), snooper->snapshotParameters(), true);
}

void PlayerUsageSnooperThread::startDetached(PlayerUsageSnooper *snooper)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    (void)snooper;
#else
    // The original thread is daemon-like. Copy all data before detaching so the
    // worker never dereferences PlayerUsageSnooper after its owner is destroyed.
    if (snooper == nullptr)
        return;
    const std::string url = snooper->getUrl();
    const PostHttp::Parameters parameters = snooper->snapshotParameters();
    std::thread([url, parameters]()
    {
        PostHttp::func_52018_a(url, parameters, true);
    }).detach();
#endif
}
