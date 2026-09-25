#pragma once

#include <string>
#include <vector>

struct SkinEntry
{
    std::string id;          // Internal ID, e.g. "LegacySteve", "custom_Goku"
    std::string name;        // Display name, e.g. "Steve", "Goku"
    std::string skinPath;    // 64x64 or full skin path
    std::string modelPath;   // 64x32 model texture path
    std::string frontPath;   // 16x32 2D front preview path (or empty if custom)
    bool isCustom = false;
    std::string filePath;    // Source file path on disk
};

class SkinManager
{
public:
    static void init();
    static void scanCustomSkins();

    // Pack / Tab support
    static int getPackCount();
    static std::string getPackName(int packIndex);
    static const std::vector<SkinEntry>& getSkinsForPack(int packIndex);
    static int getSkinCountForPack(int packIndex);
    static const SkinEntry* getSkin(int packIndex, int skinIndex);

    static int getSelectedPackIndex();
    static void setSelectedPackIndex(int packIndex);

    // Custom skin management
    static std::string getSkinsDir();
    static bool installCustomSkin(const std::string &sourcePath, const std::string &skinName, std::string &outError);
    static bool deleteCustomSkin(const std::string &id);
    static const std::vector<SkinEntry>& getCustomSkins();

    // Backward-compatible methods operating on current pack
    static const std::vector<SkinEntry>& getSkins();
    static int getSkinCount();
    static const SkinEntry* getSkin(int index);
    static const SkinEntry* getSkinById(const std::string& id);
    static int getIndexById(const std::string& id);

    static std::string getSelectedSkinId();
    static void setSelectedSkinId(const std::string& id);
    static int getSelectedIndex();
    static void setSelectedIndex(int index);

    // Player 2 skin support
    static std::string getSelectedSkinIdP2();
    static void setSelectedSkinIdP2(const std::string& id);
    static std::string getPlayer2SkinTexture();

    // Returns the texture path for the player model (e.g. "/skins/LegacySteve_32.png" or custom path)
    static std::string getActiveSkinTexture();
    static std::string getDefaultSkinTexture();
};
