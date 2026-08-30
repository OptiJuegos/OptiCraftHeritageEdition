#pragma once

#if defined(WII_PLATFORM)

#include <gccore.h>

void wiiGetOpenGLProjectionInfo(const float source[4][4], u8& type,
                                float& nearPlane, float& farPlane);
void wiiLoadProjectionFromOpenGL(const float source[4][4], bool polygonOffsetFill,
                                 float polygonOffsetUnits);

#endif
