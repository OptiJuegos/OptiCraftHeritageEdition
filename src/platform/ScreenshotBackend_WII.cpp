#include "ScreenshotBackend.h"

#include <cstdio>
#include <ctime>
#include <limits>
#include <sys/stat.h>
#include <vector>

#include "platform/RenderAPI.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace
{
std::size_t checkedRgbByteCount(int_t width, int_t height)
{
    if (width <= 0 || height <= 0)
        return 0;
    const std::size_t w = static_cast<std::size_t>(width);
    const std::size_t h = static_cast<std::size_t>(height);
    if (w > std::numeric_limits<std::size_t>::max() / h)
        return 0;
    const std::size_t pixels = w * h;
    if (pixels > std::numeric_limits<std::size_t>::max() / 3u)
        return 0;
    return pixels * 3u;
}

int checkedRgbStride(int_t width)
{
    if (width <= 0 || width > std::numeric_limits<int>::max() / 3)
        return 0;
    return width * 3;
}
}

namespace ScreenshotBackend
{
std::string save(const std::string &basePath, int_t width, int_t height)
{
    if (width <= 0 || height <= 0)
        return "Failed to save: invalid framebuffer size";

    const std::size_t byteCount = checkedRgbByteCount(width, height);
    const int stride = checkedRgbStride(width);
    if (byteCount == 0 || stride == 0)
        return "Failed to save: framebuffer size overflow";

    std::vector<unsigned char> imageData(byteCount);
    if (!renderReadPixelsRgb(0, 0, width, height, imageData.data()))
        return "Failed to save: Wii framebuffer unavailable";

    std::string screenshots = basePath;
    if (!screenshots.empty() && screenshots.back() != '/' && screenshots.back() != '\\')
        screenshots += '/';
    screenshots += "screenshots";
    mkdir(screenshots.c_str(), 0777);

    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);
    char stamp[32];
    if (tm != nullptr)
        std::strftime(stamp, sizeof(stamp), "%Y-%m-%d_%H.%M.%S", tm);
    else
        std::snprintf(stamp, sizeof(stamp), "screenshot");

    for (int suffix = 0; suffix < 1000; ++suffix)
    {
        std::string output = screenshots + "/" + stamp;
        if (suffix != 0)
            output += "_" + std::to_string(suffix + 1);
        output += ".png";

        FILE *probe = std::fopen(output.c_str(), "rb");
        if (probe != nullptr)
        {
            std::fclose(probe);
            continue;
        }

        if (!stbi_write_png(output.c_str(), width, height, 3, imageData.data(), stride))
            return "Failed to save: could not write png";
        return "Saved screenshot as " + output;
    }
    return "Failed to save: too many screenshots with the same timestamp";
}
}
