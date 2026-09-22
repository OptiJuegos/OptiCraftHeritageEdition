#pragma once

#include <string>
#include <vector>

struct SkinEntry
{
    std::string id;          // Internal ID, e.g. "LegacySteve"
    std::string name;        // Display name, e.g. "Steve"
    std::string skinPath;    // 64x64 skin path, e.g. "/skins/LegacySteve.png"
    std::string modelPath;   // 64x32 model texture path, e.g. "/skins/LegacySteve_32.png"
    std::string frontPath;   // 16x32 2D front preview path, e.g. "/skins/LegacySteve_Front.png"
};

class SkinManager
{
public:
    static void init();
    static const std::vector<SkinEntry>& getSkins();
    static int getSkinCount();
    static const SkinEntry* getSkin(int index);
    static const SkinEntry* getSkinById(const std::string& id);
    static int getIndexById(const std::string& id);

    static std::string getSelectedSkinId();
    static void setSelectedSkinId(const std::string& id);
    static int getSelectedIndex();
    static void setSelectedIndex(int index);

    // Returns the texture path for the player model (e.g. "/skins/LegacySteve_32.png" or "/mob/char.png")
    static std::string getActiveSkinTexture();
    static std::string getDefaultSkinTexture();
};
