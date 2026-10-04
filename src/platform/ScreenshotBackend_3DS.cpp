#include "ScreenshotBackend.h"

namespace ScreenshotBackend
{
std::string save(const std::string &basePath, int_t width, int_t height)
{
    (void)basePath;
    (void)width;
    (void)height;
    return "Failed to save: 3DS framebuffer readback is not implemented";
}
}
