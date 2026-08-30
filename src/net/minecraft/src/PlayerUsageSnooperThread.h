#pragma once

class PlayerUsageSnooper;

// net.minecraft.src.PlayerUsageSnooperThread
class PlayerUsageSnooperThread
{
public:
    explicit PlayerUsageSnooperThread(PlayerUsageSnooper *snooper);
    void run();

    static void startDetached(PlayerUsageSnooper *snooper);

private:
    PlayerUsageSnooper *snooper;
};
