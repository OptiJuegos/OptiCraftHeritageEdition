#pragma once

#include "java/Type.h"

namespace ConsoleAspectRatio
{
bool getDefaultWidescreen();
int_t getLogicalWidth(int_t physicalWidth, int_t physicalHeight, bool widescreen);
int_t getLogicalHeight(int_t physicalHeight);
double getProjectionAspect(int_t physicalWidth, int_t physicalHeight, bool widescreen);
}
