#pragma once

#include "platform/RenderAPI.h"

void gluPerspective(float fovy, float aspect, float zNear, float zFar);
void gluFrustum(float left, float right, float bottom, float top, float zNear, float zFar);
