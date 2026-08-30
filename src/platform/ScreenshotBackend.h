#pragma once

#include <string>
#include "java/Type.h"

namespace ScreenshotBackend
{
std::string save(const std::string &basePath, int_t width, int_t height);
}
