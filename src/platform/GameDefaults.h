#pragma once

struct PlatformGameDefaults
{
    bool useConsoleProfile = false;
    int renderDistance = 0;
    bool fancyGraphics = true;
    bool ambientOcclusion = true;
    int limitFramerate = 1;
    bool viewBobbing = true;
    bool fogOff = false;
    float brightness = 0.0f;
    float aoLevel = 0.0f;
    bool smoothFps = false;
    int autoSaveTicks = 4000;
    bool weather = true;
    bool stars = true;
    int chunkUpdates = 1;
    bool chunkUpdatesDynamic = true;
};

const PlatformGameDefaults& platformGameDefaults();
