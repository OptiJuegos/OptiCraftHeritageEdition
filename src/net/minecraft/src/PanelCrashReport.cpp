#include "PanelCrashReport.h"

#include <ctime>
#include <sstream>
#include <stdexcept>
#include <string>

#include "UnexpectedThrowable.h"
#include "platform/RenderAPI.h"

PanelCrashReport::PanelCrashReport(UnexpectedThrowable *unexpectedthrowable)
{
    // Build the stack-trace string from the stored exception_ptr
    std::string stackTrace;
    if (unexpectedthrowable->exception)
    {
        try
        {
            std::rethrow_exception(unexpectedthrowable->exception);
        }
        catch (std::exception &e)
        {
            stackTrace = e.what();
        }
        catch (...)
        {
            stackTrace = "(unknown exception)";
        }
    }

    std::ostringstream s2;

    // Generated date
    {
        std::time_t t = std::time(nullptr);
        char timeBuf[64];
        std::strftime(timeBuf, sizeof(timeBuf), "%c", std::localtime(&t));
        s2 << "Generated " << timeBuf << "\n\n";
    }

    s2 << "Minecraft: Minecraft 1.2.5\n";

#if defined(_WIN32)
    s2 << "OS: Windows\n";
#elif defined(__linux__)
    s2 << "OS: Linux\n";
#elif defined(__APPLE__)
    s2 << "OS: macOS\n";
#else
    s2 << "OS: Unknown\n";
#endif

    s2 << "VM: C++ (BetaPlusPlus)\n";
    s2 << "LWJGL: SDL2 (N/A)\n";

    std::string glVendor;
    try
    {
        const char *vendor   = (const char *)renderGetString(RenderStringQuery::Vendor);
        const char *renderer = (const char *)renderGetString(RenderStringQuery::Renderer);
        const char *version  = (const char *)renderGetString(RenderStringQuery::Version);
        if (vendor)   glVendor = vendor;
        if (renderer && version && vendor)
            s2 << "OpenGL: " << renderer << " version " << version << ", " << vendor << "\n";
    }
    catch (...) {}

    s2 << "\n" << stackTrace;

    std::string s3;
    s3 += "\n\n";

    if (stackTrace.find("Pixel format not accelerated") != std::string::npos)
    {
        s3 += "      Bad video card drivers!      \n";
        s3 += "      -----------------------      \n\n";
        s3 += "Minecraft was unable to start because it failed to find an accelerated OpenGL mode.\n";
        s3 += "This can usually be fixed by updating the video card drivers.\n";

        std::string lowerVendor = glVendor;
        for (auto &c : lowerVendor) c = (char)std::tolower((unsigned char)c);

        if (lowerVendor.find("nvidia") != std::string::npos)
        {
            s3 += "\nYou might be able to find drivers for your video card here:\n";
            s3 += "  http://www.nvidia.com/\n";
        }
        else if (lowerVendor.find("ati") != std::string::npos)
        {
            s3 += "\nYou might be able to find drivers for your video card here:\n";
            s3 += "  http://www.amd.com/\n";
        }
    }
    else
    {
        s3 += "      Minecraft has crashed!      \n";
        s3 += "      ----------------------      \n\n";
        s3 += "Minecraft has stopped running because it encountered a problem.\n\n";
        s3 += "If you wish to report this, please copy this entire text and email it to support@mojang.com.\n";
        s3 += "Please include a description of what you did when the error occured.\n";
    }

    s3 += "\n\n\n";

    // Hash of s3 as hex (mirroring Java Integer.toHexString(s3.hashCode()))
    auto javaHashCode = [](const std::string &s) -> int {
        int h = 0;
        for (unsigned char c : s)
            h = 31 * h + c;
        return h;
    };

    char hexBuf[16];
    std::snprintf(hexBuf, sizeof(hexBuf), "%x", (unsigned)javaHashCode(s3));
    s3 += "--- BEGIN ERROR REPORT ";
    s3 += hexBuf;
    s3 += " --------\n";
    s3 += s2.str();

    std::snprintf(hexBuf, sizeof(hexBuf), "%x", (unsigned)javaHashCode(s3));
    s3 += "--- END ERROR REPORT ";
    s3 += hexBuf;
    s3 += " ----------\n\n\n";

    reportText = s3;
}

const std::string &PanelCrashReport::getText() const
{
    return reportText;
}
