#include "ScreenshotBackend.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <exception>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

#include "platform/RenderAPI.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace fs = std::filesystem;

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

    try
    {
        const std::size_t byteCount = checkedRgbByteCount(width, height);
        const int stride = checkedRgbStride(width);
        if (byteCount == 0 || stride == 0)
            return "Failed to save: framebuffer size overflow";

        fs::path screenshots = fs::path(basePath) / "screenshots";
        fs::create_directories(screenshots);

        std::vector<unsigned char> pixelData(byteCount);
        std::vector<unsigned char> imageData(byteCount);
        if (!renderReadPixelsRgb(0, 0, width, height, pixelData.data()))
            return "Failed to save: framebuffer readback unavailable";

        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tmValue{};
#ifdef _WIN32
        localtime_s(&tmValue, &now);
#else
        localtime_r(&now, &tmValue);
#endif

        std::ostringstream nameStream;
        nameStream << std::put_time(&tmValue, "%Y-%m-%d_%H.%M.%S");
        const std::string baseName = nameStream.str();

        fs::path output;
        for (int_t k = 1;; ++k)
        {
            const std::string suffix = k == 1 ? "" : "_" + std::to_string(k);
            output = screenshots / (baseName + suffix + ".png");
            if (!fs::exists(output))
                break;
        }

        for (int_t x = 0; x < width; ++x)
        {
            for (int_t y = 0; y < height; ++y)
            {
                const std::size_t source =
                    (static_cast<std::size_t>(x) +
                     static_cast<std::size_t>(height - y - 1) * static_cast<std::size_t>(width)) * 3u;
                const std::size_t dest =
                    (static_cast<std::size_t>(x) +
                     static_cast<std::size_t>(y) * static_cast<std::size_t>(width)) * 3u;
                imageData[dest + 0] = pixelData[source + 0];
                imageData[dest + 1] = pixelData[source + 1];
                imageData[dest + 2] = pixelData[source + 2];
            }
        }

        if (!stbi_write_png(output.string().c_str(), width, height, 3, imageData.data(), stride))
            return "Failed to save: could not write png";
        return "Saved screenshot as " + output.filename().string();
    }
    catch (const std::exception &exception)
    {
        return std::string("Failed to save: ") + exception.what();
    }
}
}
