#include "wii/render/WiiProjection.h"

#if defined(WII_PLATFORM)

#include <cmath>
#include <cstring>

namespace
{
constexpr float kPerspectiveRowEpsilon = 1.0e-6f;
constexpr float kPolygonOffsetScale = 1.0e-5f;
}

void wiiGetOpenGLProjectionInfo(const float source[4][4], u8& type,
                                float& nearPlane, float& farPlane)
{
    const float a = source[2][2];
    const float b = source[2][3];
    if (std::fabs(source[3][3]) <= kPerspectiveRowEpsilon)
    {
        type = GX_PERSPECTIVE;
        nearPlane = b / (a - 1.0f);
        farPlane = std::fabs(a + 1.0f) > kPerspectiveRowEpsilon
            ? b / (a + 1.0f)
            : nearPlane + 1.0f;
    }
    else
    {
        type = GX_ORTHOGRAPHIC;
        if (std::fabs(a) <= kPerspectiveRowEpsilon)
        {
            nearPlane = 0.1f;
            farPlane = 1.0f;
            return;
        }
        nearPlane = (b + 1.0f) / a;
        farPlane = (b - 1.0f) / a;
    }
}

void wiiLoadProjectionFromOpenGL(const float source[4][4], bool polygonOffsetFill,
                                 float polygonOffsetUnits)
{
    Mtx44 projection;
    std::memcpy(projection, source, sizeof(projection));

    u8 type;
    float nearPlane;
    float farPlane;
    wiiGetOpenGLProjectionInfo(source, type, nearPlane, farPlane);

    const float distance = farPlane - nearPlane;
    if (std::fabs(distance) > kPerspectiveRowEpsilon)
    {
        const float inverseDistance = 1.0f / distance;
        const float zOffset = polygonOffsetFill
            ? polygonOffsetUnits * kPolygonOffsetScale
            : 0.0f;

        if (type == GX_ORTHOGRAPHIC)
        {
            projection[2][2] = -inverseDistance;
            projection[2][3] = -farPlane * inverseDistance + zOffset;
        }
        else
        {
            projection[2][2] = -nearPlane * inverseDistance;
            projection[2][3] = -nearPlane * farPlane * inverseDistance + zOffset;
        }
    }

    GX_LoadProjectionMtx(projection, type);
}

#endif
