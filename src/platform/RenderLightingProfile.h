#pragma once

struct RenderLightingProfile
{
    float ambient;
    float diffuse;
};

RenderLightingProfile renderGetStandardItemLightingProfile();
