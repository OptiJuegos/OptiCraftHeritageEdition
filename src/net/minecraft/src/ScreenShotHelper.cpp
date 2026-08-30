#include "ScreenShotHelper.h"
#include "platform/ScreenshotBackend.h"

std::string ScreenShotHelper::saveScreenshot(const std::string &file, int_t width, int_t height)
{
    return ScreenshotBackend::save(file, width, height);
}
