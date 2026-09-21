#pragma once

#include "java/Type.h"
#include <string>

struct Waypoint
{
    std::string name;
    int_t x = 0;
    int_t y = 0;
    int_t z = 0;
    int_t color = 0x00FF00;
    bool enabled = true;
};
