#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/system/Ps2IrxLoader.h"
#include "ps2/storage/assets/Ps2Assets.h"
#include "ps2/storage/Ps2Storage.h"
#include "platform/storage/PathUtils.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

#include <loadfile.h>
#include <sbv_patches.h>

namespace Ps2IrxLoader
{

int load(const char* assetKey, const char* fallbackPath)
{
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    unsigned int size = 0;
    unsigned char* data = nullptr;
    std::string usedPath;

    // 1. Try primary assetKey via Ps2Assets
    if (assetKey && assetKey[0] != '\0')
    {
        data = Ps2Assets::loadAsset(assetKey, &size);
        if (data && size > 0)
            usedPath = assetKey;
        else
        {
            std::free(data);
            data = nullptr;
            // 2. Try prefixing with "data/" if not already present
            std::string dataKey = std::string("data/") + assetKey;
            data = Ps2Assets::loadAsset(dataKey, &size);
            if (data && size > 0)
                usedPath = dataKey;
            else
            {
                std::free(data);
                data = nullptr;
            }
        }
    }

    // 3. Try direct file read from well-known paths
    if (!data && assetKey && assetKey[0] != '\0')
    {
        std::string filename = assetKey;
        const size_t slash = filename.find_last_of("/\\");
        if (slash != std::string::npos)
            filename = filename.substr(slash + 1);

        std::string upperFilename = filename;
        std::transform(upperFilename.begin(), upperFilename.end(), upperFilename.begin(), ::toupper);

        std::vector<std::string> fileCandidates = {
            std::string("host:data/irx/") + filename,
            std::string("host:/data/irx/") + filename,
            std::string("host:irx/") + filename,
            std::string("host:/irx/") + filename,
            std::string("host:") + filename,
            std::string("cdrom0:/DATA/IRX/") + upperFilename,
            std::string("cdrom0:/DATA/IRX/") + upperFilename + ";1",
            std::string("cdrom0:/data/irx/") + filename,
            std::string("mass:/OptiCraftHeritage/data/irx/") + filename,
            std::string("mass:/data/irx/") + filename,
            std::string("data/irx/") + filename,
            std::string("irx/") + filename
        };

        if (fallbackPath && fallbackPath[0] != '\0')
            fileCandidates.push_back(fallbackPath);

        for (const auto& cand : fileCandidates)
        {
            data = Ps2Storage::readWholeFile(cand, &size, 64);
            if (data && size > 0)
            {
                usedPath = cand;
                break;
            }
            std::free(data);
            data = nullptr;
        }
    }

    if (data && size > 0)
    {
        int mod_res = 0;
        const int result = SifExecModuleBuffer(data, static_cast<int>(size), 0, nullptr, &mod_res);
        std::free(data);
        MC_LOG_INFO("platform", "[PS2][irx] load %s -> result=%d, mod_res=%d\n", usedPath.c_str(), result, mod_res);
        if (result < 0 || mod_res == 1)
            return -1;
        return result;
    }
    std::free(data);

    // 4. SifLoadStartModule fallback if file read buffer failed
    if (fallbackPath && fallbackPath[0] != '\0')
    {
        int mod_res = 0;
        const int result = SifLoadStartModule(fallbackPath, 0, nullptr, &mod_res);
        MC_LOG_INFO("platform", "[PS2][irx] SifLoadStartModule %s -> result=%d, mod_res=%d\n", fallbackPath, result, mod_res);
        if (result < 0 || mod_res == 1)
            return -1;
        return result;
    }

    MC_LOG_INFO("platform", "[PS2][irx] module unavailable: %s\n", assetKey ? assetKey : "(null)");
    return -1;
}

} // namespace Ps2IrxLoader

#endif // PS2_PLATFORM
