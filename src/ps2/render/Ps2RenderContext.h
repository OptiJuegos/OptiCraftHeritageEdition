#pragma once

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2RenderTypes.h"

#include <gsKit.h>

struct Ps2RenderContext
{
    const void* vp = nullptr;
    int vstride = 0;
    int vsize = 3;
    const void* tp = nullptr;
    int tstride = 0;
    const void* cp = nullptr;
    int cstride = 0;
    int csize = 4;
    bool cfloat = false;

    bool ven = false;
    bool ten = false;
    bool cen = false;

    bool tex2d = false;
    bool blend = false;
    bool terrainTranslucent = false;
    bool cullFace = false;
    unsigned int cullMode = ps2RenderValue(Ps2RenderFace::Back);
    unsigned int frontFace = 0x0901;

    unsigned char cr = 255;
    unsigned char cg = 255;
    unsigned char cb = 255;
    unsigned char ca = 255;

    bool alphaTest = false;
    unsigned int alphaFunc = ps2RenderValue(Ps2RenderCompare::Greater);
    u8 alphaRef = 25;

    bool fog = false;
    float fogStart = 0.0f;
    float fogEnd = 32.0f;
    float fogR = 0.7f;
    float fogG = 0.85f;
    float fogB = 1.0f;
    unsigned int fogMode = ps2RenderValue(Ps2RenderFogMode::Linear);
    float fogDensity = 1.0f;
};

Ps2RenderContext& ps2_render_context();

#endif // PS2_PLATFORM
