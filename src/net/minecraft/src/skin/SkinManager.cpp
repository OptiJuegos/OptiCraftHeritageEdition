#include "SkinManager.h"
#include "Minecraft.h"
#include "GameSettings.h"
#include "EntityPlayerSP.h"
#include "java/File.h"
#include "platform/Storage.h"
#include "platform/Log.h"

#ifdef PS2_PLATFORM
#include "ps2/storage/assets/Ps2Assets.h"
#include "ps2/storage/save/McSavePS2.h"
#include "ps2/storage/save/Ps2MemoryCard.h"
#endif

#include "stb_image.h"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <cstring>
#include <cctype>

namespace
{
std::vector<SkinEntry> s_defaultSkins;
std::vector<SkinEntry> s_customSkins;
std::string s_selectedId = "LegacySteve";
std::string s_selectedIdP2 = "TennisSteve";
int s_selectedPackIndex = 0; // 0 = Default, 1 = Custom
int s_selectedIndex = 0;
bool s_initialized = false;

// Helper to copy a rectangle of pixels with optional horizontal mirroring and alpha blending
void copyPixelRect(const unsigned char *src, int srcW, int srcH,
                   int sx, int sy, int rw, int rh,
                   unsigned char *dst, int dstW, int dstH,
                   int dx, int dy, bool mirrorX, bool blendAlpha)
{
    for (int y = 0; y < rh; ++y)
    {
        int srcY = sy + y;
        int dstY = dy + y;
        if (srcY < 0 || srcY >= srcH || dstY < 0 || dstY >= dstH)
            continue;

        for (int x = 0; x < rw; ++x)
        {
            int srcX = mirrorX ? (sx + rw - 1 - x) : (sx + x);
            int dstX = dx + x;
            if (srcX < 0 || srcX >= srcW || dstX < 0 || dstX >= dstW)
                continue;

            const unsigned char *s = &src[(srcY * srcW + srcX) * 4];
            unsigned char *d = &dst[(dstY * dstW + dstX) * 4];

            if (blendAlpha)
            {
                float a = s[3] / 255.0f;
                if (a > 0.01f)
                {
                    d[0] = static_cast<unsigned char>(s[0] * a + d[0] * (1.0f - a));
                    d[1] = static_cast<unsigned char>(s[1] * a + d[1] * (1.0f - a));
                    d[2] = static_cast<unsigned char>(s[2] * a + d[2] * (1.0f - a));
                    d[3] = 255;
                }
            }
            else
            {
                d[0] = s[0];
                d[1] = s[1];
                d[2] = s[2];
                d[3] = s[3];
            }
        }
    }
}

// Assembles a 16x32 front preview image from a standard 64x32 or 64x64 Minecraft skin
bool assembleFrontPreview(const unsigned char *rgba, int w, int h, std::vector<unsigned char> &outFront)
{
    if (rgba == nullptr || w != 64 || (h != 32 && h != 64))
        return false;

    outFront.assign(16 * 32 * 4, 0);
    unsigned char *dst = outFront.data();

    // 1. Head front (8x8 at 8,8 -> dst 4,0)
    copyPixelRect(rgba, w, h, 8, 8, 8, 8, dst, 16, 32, 4, 0, false, false);

    // 2. Hat/Helmet overlay (8x8 at 40,8 -> dst 4,0 with alpha blend)
    copyPixelRect(rgba, w, h, 40, 8, 8, 8, dst, 16, 32, 4, 0, false, true);

    // 3. Torso front (8x12 at 20,20 -> dst 4,8)
    copyPixelRect(rgba, w, h, 20, 20, 8, 12, dst, 16, 32, 4, 8, false, false);

    // 4. Right Arm front (4x12 at 44,20 -> dst 0,8)
    copyPixelRect(rgba, w, h, 44, 20, 4, 12, dst, 16, 32, 0, 8, false, false);

    // 5. Left Arm front (4x12 at 12,8)
    if (h == 64)
    {
        copyPixelRect(rgba, w, h, 36, 52, 4, 12, dst, 16, 32, 12, 8, false, false);
    }
    else
    {
        // 64x32: Left arm is mirrored right arm
        copyPixelRect(rgba, w, h, 44, 20, 4, 12, dst, 16, 32, 12, 8, true, false);
    }

    // 6. Right Leg front (4x12 at 4,20 -> dst 4,20)
    copyPixelRect(rgba, w, h, 4, 20, 4, 12, dst, 16, 32, 4, 20, false, false);

    // 7. Left Leg front (4x12 at 8,20)
    if (h == 64)
    {
        copyPixelRect(rgba, w, h, 20, 52, 4, 12, dst, 16, 32, 8, 20, false, false);
    }
    else
    {
        // 64x32: Left leg is mirrored right leg
        copyPixelRect(rgba, w, h, 4, 20, 4, 12, dst, 16, 32, 8, 20, true, false);
    }

    return true;
}

// Converts a 64x64 skin to a 64x32 retro-compatible texture for MC 1.2.5 ModelBiped
bool makeRetro32(const unsigned char *rgba, int w, int h, std::vector<unsigned char> &outRetro)
{
    if (rgba == nullptr || w != 64 || (h != 32 && h != 64))
        return false;

    outRetro.assign(64 * 32 * 4, 0);
    std::memcpy(outRetro.data(), rgba, 64 * 32 * 4);

    if (h == 64)
    {
        // Composite modern 64x64 second-layer overlays onto base body parts:
        // Torso overlay: (16, 32, 24, 16) -> (16, 16, 24, 16)
        copyPixelRect(rgba, w, h, 16, 32, 24, 16, outRetro.data(), 64, 32, 16, 16, false, true);
        // Right Arm overlay: (40, 32, 16, 16) -> (40, 16, 16, 16)
        copyPixelRect(rgba, w, h, 40, 32, 16, 16, outRetro.data(), 64, 32, 40, 16, false, true);
        // Right Leg overlay: (0, 32, 16, 16) -> (0, 16, 16, 16)
        copyPixelRect(rgba, w, h, 0, 32, 16, 16, outRetro.data(), 64, 32, 0, 16, false, true);
    }

    // Set base skin regions opaque (Minecraft 1.2.5 spec)
    // Head base (0, 0, 32, 16)
    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 32; ++x)
            outRetro[(x + y * 64) * 4 + 3] = 255;
    // Torso, arms, legs (0, 16, 64, 16)
    for (int y = 16; y < 32; ++y)
        for (int x = 0; x < 64; ++x)
            outRetro[(x + y * 64) * 4 + 3] = 255;

    return true;
}

std::string sanitizeSkinName(const std::string &raw)
{
    std::string name;
    for (char c : raw)
    {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == ' ')
            name += c;
    }
    while (!name.empty() && (name.front() == ' ' || name.front() == '_'))
        name.erase(name.begin());
    while (!name.empty() && (name.back() == ' ' || name.back() == '_'))
        name.pop_back();
    return name.empty() ? "CustomSkin" : name;
}

} // namespace

void SkinManager::init()
{
    if (s_initialized)
        return;

    s_defaultSkins.clear();

    // 1. Steve (default)
    s_defaultSkins.push_back({
        "LegacySteve",
        "Steve",
        "/skins/LegacySteve.png",
        "/skins/LegacySteve_32.png",
        "/skins/LegacySteve_Front.png",
        false,
        ""
    });

    // 2. Alex (immediately to the right of Steve)
    s_defaultSkins.push_back({
        "LegacyAlex",
        "Alex",
        "/skins/LegacyAlex.png",
        "/skins/LegacyAlex_32.png",
        "/skins/LegacyAlex_Front.png",
        false,
        ""
    });

    // Remaining default legacy console skins from assets.pak
    s_defaultSkins.push_back({"TennisSteve", "Tennis Steve", "/skins/TennisSteve.png", "/skins/TennisSteve_32.png", "/skins/TennisSteve_Front.png", false, ""});
    s_defaultSkins.push_back({"TennisAlex", "Tennis Alex", "/skins/TennisAlex.png", "/skins/TennisAlex_32.png", "/skins/TennisAlex_Front.png", false, ""});
    s_defaultSkins.push_back({"TuxedoSteve", "Tuxedo Steve", "/skins/TuxedoSteve.png", "/skins/TuxedoSteve_32.png", "/skins/TuxedoSteve_Front.png", false, ""});
    s_defaultSkins.push_back({"TuxedoAlex", "Tuxedo Alex", "/skins/TuxedoAlex.png", "/skins/TuxedoAlex_32.png", "/skins/TuxedoAlex_Front.png", false, ""});
    s_defaultSkins.push_back({"AthleteSteve", "Athlete Steve", "/skins/AthleteSteve.png", "/skins/AthleteSteve_32.png", "/skins/AthleteSteve_Front.png", false, ""});
    s_defaultSkins.push_back({"SwedishAlex", "Swedish Alex", "/skins/SwedishAlex.png", "/skins/SwedishAlex_32.png", "/skins/SwedishAlex_Front.png", false, ""});
    s_defaultSkins.push_back({"CyclistSteve", "Cyclist Steve", "/skins/CyclistSteve.png", "/skins/CyclistSteve_32.png", "/skins/CyclistSteve_Front.png", false, ""});
    s_defaultSkins.push_back({"CyclistAlex", "Cyclist Alex", "/skins/CyclistAlex.png", "/skins/CyclistAlex_32.png", "/skins/CyclistAlex_Front.png", false, ""});
    s_defaultSkins.push_back({"BoxerSteve", "Boxer Steve", "/skins/BoxerSteve.png", "/skins/BoxerSteve_32.png", "/skins/BoxerSteve_Front.png", false, ""});
    s_defaultSkins.push_back({"BoxerAlex", "Boxer Alex", "/skins/BoxerAlex.png", "/skins/BoxerAlex_32.png", "/skins/BoxerAlex_Front.png", false, ""});
    s_defaultSkins.push_back({"PrisonerSteve", "Prisoner Steve", "/skins/PrisonerSteve.png", "/skins/PrisonerSteve_32.png", "/skins/PrisonerSteve_Front.png", false, ""});
    s_defaultSkins.push_back({"PrisonerAlex", "Prisoner Alex", "/skins/PrisonerAlex.png", "/skins/PrisonerAlex_32.png", "/skins/PrisonerAlex_Front.png", false, ""});
    s_defaultSkins.push_back({"ScottishSteve", "Scottish Steve", "/skins/Scottish_Steve.png", "/skins/Scottish_Steve_32.png", "/skins/Scottish_Steve_Front.png", false, ""});
    s_defaultSkins.push_back({"MojangSteve", "Mojang Steve", "/skins/MojangSteve.png", "/skins/MojangSteve_32.png", "/skins/MojangSteve_Front.png", false, ""});
    s_defaultSkins.push_back({"MojangAlex", "Mojang Alex", "/skins/MojangAlex.png", "/skins/MojangAlex_32.png", "/skins/MojangAlex_Front.png", false, ""});
    s_defaultSkins.push_back({"Stampy", "Stampy", "/skins/Stampy.png", "/skins/Stampy_32.png", "/skins/Stampy_Front.png", false, ""});
    s_defaultSkins.push_back({"Crocodile", "Crocodile", "/skins/Crocodile.png", "/skins/Crocodile_32.png", "/skins/Crocodile_Front.png", false, ""});
    s_defaultSkins.push_back({"LegacySquid", "Legacy Squid", "/skins/LegacySquid.png", "/skins/LegacySquid_32.png", "/skins/LegacySquid_Front.png", false, ""});

    s_initialized = true;

    // Scan custom skins installed in storage
    scanCustomSkins();
}

std::string SkinManager::getSkinsDir()
{
#ifdef PS2_PLATFORM
    if (Ps2MemoryCard::isFormatted())
    {
        return "mc0:OPTCRAFT/skins";
    }
#endif
    File *dataDir = Minecraft::getMinecraftDir();
    if (dataDir != nullptr)
    {
        return PlatformStorage::join(dataDir->toString(), "skins");
    }
    return "skins";
}

void SkinManager::scanCustomSkins()
{
    s_customSkins.clear();

    std::vector<std::string> searchDirs;
    std::string primaryDir = getSkinsDir();
    searchDirs.push_back(primaryDir);

#ifdef PS2_PLATFORM
    if (primaryDir != "mc0:/OPTCRAFT/skins")
        searchDirs.push_back("mc0:/OPTCRAFT/skins");
#endif
    if (primaryDir != "skins" && primaryDir != "./skins")
        searchDirs.push_back("skins");

    std::vector<std::string> seenIds;

    for (const auto &dir : searchDirs)
    {
        if (dir.empty())
            continue;

        std::vector<std::string> entries;
        if (!PlatformStorage::listPathEntries(dir, entries))
            continue;

        for (const auto &entry : entries)
        {
            if (entry.size() < 5)
                continue;

            std::string lower = entry;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.compare(lower.size() - 4, 4, ".png") != 0)
                continue;

            // Skip companion files generated automatically
            if (lower.find("_32.png") != std::string::npos || lower.find("_front.png") != std::string::npos)
                continue;

            std::string fullPath = PlatformStorage::join(dir, entry);
            std::vector<unsigned char> rawBytes;
            if (!PlatformStorage::readFile(fullPath, rawBytes) || rawBytes.empty())
                continue;

            int w = 0, h = 0, comp = 0;
            if (!stbi_info_from_memory(rawBytes.data(), static_cast<int>(rawBytes.size()), &w, &h, &comp))
                continue;

            if (w != 64 || (h != 32 && h != 64))
                continue;

            std::string baseName = entry.substr(0, entry.size() - 4);
            std::string id = "custom_" + baseName;

            if (std::find(seenIds.begin(), seenIds.end(), id) != seenIds.end())
                continue;
            seenIds.push_back(id);

            std::string model32 = PlatformStorage::join(dir, baseName + "_32.png");
            std::string front32 = PlatformStorage::join(dir, baseName + "_Front.png");

            // Auto-generate retro 64x32 and front preview if missing
            if (h == 64 && !PlatformStorage::exists(model32))
            {
                unsigned char *rgba = stbi_load_from_memory(rawBytes.data(), static_cast<int>(rawBytes.size()), &w, &h, &comp, 4);
                if (rgba != nullptr)
                {
                    std::vector<unsigned char> retro32;
                    if (makeRetro32(rgba, w, h, retro32))
                    {
                        int len = 0;
                        unsigned char *png = stbi_write_png_to_mem(retro32.data(), 64 * 4, 64, 32, 4, &len);
                        if (png != nullptr)
                        {
                            PlatformStorage::writeFile(model32, png, len);
                            STBIW_FREE(png);
                        }
                    }
                    if (!PlatformStorage::exists(front32))
                    {
                        std::vector<unsigned char> frontRgba;
                        if (assembleFrontPreview(rgba, w, h, frontRgba))
                        {
                            int len = 0;
                            unsigned char *png = stbi_write_png_to_mem(frontRgba.data(), 16 * 4, 16, 32, 4, &len);
                            if (png != nullptr)
                            {
                                PlatformStorage::writeFile(front32, png, len);
                                STBIW_FREE(png);
                            }
                        }
                    }
                    stbi_image_free(rgba);
                }
            }
            else if (h == 32 && !PlatformStorage::exists(front32))
            {
                unsigned char *rgba = stbi_load_from_memory(rawBytes.data(), static_cast<int>(rawBytes.size()), &w, &h, &comp, 4);
                if (rgba != nullptr)
                {
                    std::vector<unsigned char> frontRgba;
                    if (assembleFrontPreview(rgba, w, h, frontRgba))
                    {
                        int len = 0;
                        unsigned char *png = stbi_write_png_to_mem(frontRgba.data(), 16 * 4, 16, 32, 4, &len);
                        if (png != nullptr)
                        {
                            PlatformStorage::writeFile(front32, png, len);
                            STBIW_FREE(png);
                        }
                    }
                    stbi_image_free(rgba);
                }
            }

            SkinEntry custom;
            custom.id = id;
            custom.name = sanitizeSkinName(baseName);
            custom.skinPath = fullPath;
            custom.modelPath = (h == 64 && PlatformStorage::exists(model32)) ? model32 : (h == 32 ? fullPath : model32);
            custom.frontPath = PlatformStorage::exists(front32) ? front32 : "";
            custom.isCustom = true;
            custom.filePath = fullPath;

            s_customSkins.push_back(custom);
        }
    }

    // If active pack is Custom but custom list became empty, revert to Default pack
    if (s_customSkins.empty() && s_selectedPackIndex == 1)
    {
        s_selectedPackIndex = 0;
        s_selectedId = "LegacySteve";
        s_selectedIndex = 0;
    }
}

int SkinManager::getPackCount()
{
    init();
    return s_customSkins.empty() ? 1 : 2;
}

std::string SkinManager::getPackName(int packIndex)
{
    if (packIndex == 1 && !s_customSkins.empty())
        return "Custom Skins";
    return "Default Skins";
}

const std::vector<SkinEntry>& SkinManager::getSkinsForPack(int packIndex)
{
    init();
    if (packIndex == 1 && !s_customSkins.empty())
        return s_customSkins;
    return s_defaultSkins;
}

int SkinManager::getSkinCountForPack(int packIndex)
{
    return static_cast<int>(getSkinsForPack(packIndex).size());
}

const SkinEntry* SkinManager::getSkin(int packIndex, int skinIndex)
{
    const auto &skins = getSkinsForPack(packIndex);
    if (skins.empty())
        return nullptr;
    const int count = static_cast<int>(skins.size());
    const int wrapped = ((skinIndex % count) + count) % count;
    return &skins[wrapped];
}

int SkinManager::getSelectedPackIndex()
{
    return s_selectedPackIndex;
}

void SkinManager::setSelectedPackIndex(int packIndex)
{
    if (packIndex == 1 && s_customSkins.empty())
        packIndex = 0;
    s_selectedPackIndex = packIndex;
}

const std::vector<SkinEntry>& SkinManager::getCustomSkins()
{
    init();
    return s_customSkins;
}

bool SkinManager::installCustomSkin(const std::string &sourcePath, const std::string &skinName, std::string &outError)
{
    std::vector<unsigned char> rawBytes;
    if (!PlatformStorage::readFile(sourcePath, rawBytes) || rawBytes.empty())
    {
        outError = "Could not read skin file from source.";
        return false;
    }

    int w = 0, h = 0, comp = 0;
    unsigned char *rgba = stbi_load_from_memory(rawBytes.data(), static_cast<int>(rawBytes.size()), &w, &h, &comp, 4);
    if (rgba == nullptr)
    {
        outError = "Invalid image file (must be standard PNG).";
        return false;
    }

    if (w != 64 || (h != 32 && h != 64))
    {
        stbi_image_free(rgba);
        outError = "Skin dimensions must be 64x32 or 64x64 pixels.";
        return false;
    }

    std::string destDir = getSkinsDir();
    if (!PlatformStorage::mkdirs(destDir))
    {
        stbi_image_free(rgba);
        outError = "Failed to access/create skins storage directory.";
        return false;
    }

    std::string base = sanitizeSkinName(skinName);
    std::string mainPath = PlatformStorage::join(destDir, base + ".png");
    std::string model32Path = PlatformStorage::join(destDir, base + "_32.png");
    std::string frontPath = PlatformStorage::join(destDir, base + "_Front.png");

    // 1. Write the original raw file
    if (!PlatformStorage::writeFile(mainPath, rawBytes.data(), rawBytes.size()))
    {
        stbi_image_free(rgba);
        outError = "Failed to save skin to storage.";
        return false;
    }

    // 2. Generate 64x32 retro texture if source is 64x64
    if (h == 64)
    {
        std::vector<unsigned char> retro32;
        if (makeRetro32(rgba, w, h, retro32))
        {
            int len = 0;
            unsigned char *png = stbi_write_png_to_mem(retro32.data(), 64 * 4, 64, 32, 4, &len);
            if (png != nullptr)
            {
                PlatformStorage::writeFile(model32Path, png, len);
                STBIW_FREE(png);
            }
        }
    }

    // 3. Assemble and save the 16x32 Front Preview
    std::vector<unsigned char> frontRgba;
    if (assembleFrontPreview(rgba, w, h, frontRgba))
    {
        int len = 0;
        unsigned char *png = stbi_write_png_to_mem(frontRgba.data(), 16 * 4, 16, 32, 4, &len);
        if (png != nullptr)
        {
            PlatformStorage::writeFile(frontPath, png, len);
            STBIW_FREE(png);
        }
    }

    stbi_image_free(rgba);

    // Refresh custom skins and select the newly installed one
    scanCustomSkins();
    s_selectedPackIndex = 1;
    std::string skinId = "custom_" + base;
    setSelectedSkinId(skinId);

    Minecraft *mc = Minecraft::getMinecraft();
    if (mc != nullptr)
    {
        if (mc->gameSettings != nullptr)
        {
            mc->gameSettings->selectedSkin = skinId;
            mc->gameSettings->saveOptions();
        }
        if (mc->thePlayer != nullptr)
        {
            std::string activeModelPath = (h == 64 && PlatformStorage::exists(model32Path)) ? model32Path : mainPath;
            mc->thePlayer->setEntityTexture(activeModelPath);
            mc->thePlayer->skinUrl = "";
        }
    }

    MC_LOG_INFO("skins", "Custom skin installed successfully: %s\n", base.c_str());
    return true;
}

bool SkinManager::deleteCustomSkin(const std::string &id)
{
    init();
    for (auto it = s_customSkins.begin(); it != s_customSkins.end(); ++it)
    {
        if (it->id == id)
        {
            if (!it->filePath.empty())
            {
                PlatformStorage::removeFile(it->filePath);

                // Also remove companion files
                std::string path = it->filePath;
                if (path.size() > 4 && path.substr(path.size() - 4) == ".png")
                {
                    std::string stem = path.substr(0, path.size() - 4);
                    PlatformStorage::removeFile(stem + "_32.png");
                    PlatformStorage::removeFile(stem + "_Front.png");
                }
            }

            s_customSkins.erase(it);

            if (s_customSkins.empty())
            {
                s_selectedPackIndex = 0;
                s_selectedId = "LegacySteve";
                s_selectedIndex = 0;
            }
            return true;
        }
    }
    return false;
}

const std::vector<SkinEntry>& SkinManager::getSkins()
{
    return getSkinsForPack(s_selectedPackIndex);
}

int SkinManager::getSkinCount()
{
    return getSkinCountForPack(s_selectedPackIndex);
}

const SkinEntry* SkinManager::getSkin(int index)
{
    return getSkin(s_selectedPackIndex, index);
}

const SkinEntry* SkinManager::getSkinById(const std::string& id)
{
    init();
    for (const auto& skin : s_defaultSkins)
    {
        if (skin.id == id)
            return &skin;
    }
    for (const auto& skin : s_customSkins)
    {
        if (skin.id == id)
            return &skin;
    }
    return nullptr;
}

int SkinManager::getIndexById(const std::string& id)
{
    init();
    const auto &skins = getSkins();
    for (std::size_t i = 0; i < skins.size(); ++i)
    {
        if (skins[i].id == id)
            return static_cast<int>(i);
    }
    return 0;
}

std::string SkinManager::getSelectedSkinId()
{
    return s_selectedId;
}

void SkinManager::setSelectedSkinId(const std::string& id)
{
    init();
    for (std::size_t i = 0; i < s_defaultSkins.size(); ++i)
    {
        if (s_defaultSkins[i].id == id)
        {
            s_selectedPackIndex = 0;
            s_selectedId = id;
            s_selectedIndex = static_cast<int>(i);
            return;
        }
    }
    for (std::size_t i = 0; i < s_customSkins.size(); ++i)
    {
        if (s_customSkins[i].id == id)
        {
            s_selectedPackIndex = 1;
            s_selectedId = id;
            s_selectedIndex = static_cast<int>(i);
            return;
        }
    }
}

int SkinManager::getSelectedIndex()
{
    return s_selectedIndex;
}

void SkinManager::setSelectedIndex(int index)
{
    const SkinEntry* skin = getSkin(index);
    if (skin != nullptr)
    {
        s_selectedId = skin->id;
        s_selectedIndex = index;
    }
}

std::string SkinManager::getSelectedSkinIdP2()
{
    return s_selectedIdP2;
}

void SkinManager::setSelectedSkinIdP2(const std::string& id)
{
    init();
    s_selectedIdP2 = id;
}

std::string SkinManager::getPlayer2SkinTexture()
{
    init();
    const SkinEntry* skin = getSkinById(s_selectedIdP2);
    if (skin != nullptr)
        return skin->modelPath;
    return getDefaultSkinTexture();
}

std::string SkinManager::getActiveSkinTexture()
{
    init();
    const SkinEntry* skin = getSkinById(s_selectedId);
    if (skin != nullptr)
        return skin->modelPath;
    return getDefaultSkinTexture();
}

std::string SkinManager::getDefaultSkinTexture()
{
    return "/mob/char.png";
}
