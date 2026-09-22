#include "SkinManager.h"

#include <algorithm>

namespace
{
std::vector<SkinEntry> s_skins;
std::string s_selectedId = "LegacySteve";
bool s_initialized = false;
}

void SkinManager::init()
{
    if (s_initialized)
        return;

    s_skins.clear();

    // 1. Steve (default)
    s_skins.push_back({
        "LegacySteve",
        "Steve",
        "/skins/LegacySteve.png",
        "/skins/LegacySteve_32.png",
        "/skins/LegacySteve_Front.png"
    });

    // 2. Alex (immediately to the right of Steve)
    s_skins.push_back({
        "LegacyAlex",
        "Alex",
        "/skins/LegacyAlex.png",
        "/skins/LegacyAlex_32.png",
        "/skins/LegacyAlex_Front.png"
    });

    // Remaining skins
    s_skins.push_back({
        "TennisSteve",
        "Tennis Steve",
        "/skins/TennisSteve.png",
        "/skins/TennisSteve_32.png",
        "/skins/TennisSteve_Front.png"
    });

    s_skins.push_back({
        "TennisAlex",
        "Tennis Alex",
        "/skins/TennisAlex.png",
        "/skins/TennisAlex_32.png",
        "/skins/TennisAlex_Front.png"
    });

    s_skins.push_back({
        "TuxedoSteve",
        "Tuxedo Steve",
        "/skins/TuxedoSteve.png",
        "/skins/TuxedoSteve_32.png",
        "/skins/TuxedoSteve_Front.png"
    });

    s_skins.push_back({
        "TuxedoAlex",
        "Tuxedo Alex",
        "/skins/TuxedoAlex.png",
        "/skins/TuxedoAlex_32.png",
        "/skins/TuxedoAlex_Front.png"
    });

    s_skins.push_back({
        "AthleteSteve",
        "Athlete Steve",
        "/skins/AthleteSteve.png",
        "/skins/AthleteSteve_32.png",
        "/skins/AthleteSteve_Front.png"
    });

    s_skins.push_back({
        "CyclistSteve",
        "Cyclist Steve",
        "/skins/CyclistSteve.png",
        "/skins/CyclistSteve_32.png",
        "/skins/CyclistSteve_Front.png"
    });

    s_skins.push_back({
        "CyclistAlex",
        "Cyclist Alex",
        "/skins/CyclistAlex.png",
        "/skins/CyclistAlex_32.png",
        "/skins/CyclistAlex_Front.png"
    });

    s_skins.push_back({
        "BoxerSteve",
        "Boxer Steve",
        "/skins/BoxerSteve.png",
        "/skins/BoxerSteve_32.png",
        "/skins/BoxerSteve_Front.png"
    });

    s_skins.push_back({
        "BoxerAlex",
        "Boxer Alex",
        "/skins/BoxerAlex.png",
        "/skins/BoxerAlex_32.png",
        "/skins/BoxerAlex_Front.png"
    });

    s_skins.push_back({
        "PrisonerSteve",
        "Prisoner Steve",
        "/skins/PrisonerSteve.png",
        "/skins/PrisonerSteve_32.png",
        "/skins/PrisonerSteve_Front.png"
    });

    s_skins.push_back({
        "PrisonerAlex",
        "Prisoner Alex",
        "/skins/PrisonerAlex.png",
        "/skins/PrisonerAlex_32.png",
        "/skins/PrisonerAlex_Front.png"
    });

    s_skins.push_back({
        "Scottish_Steve",
        "Scottish Steve",
        "/skins/Scottish_Steve.png",
        "/skins/Scottish_Steve_32.png",
        "/skins/Scottish_Steve_Front.png"
    });

    s_skins.push_back({
        "SwedishAlex",
        "Swedish Alex",
        "/skins/SwedishAlex.png",
        "/skins/SwedishAlex_32.png",
        "/skins/SwedishAlex_Front.png"
    });

    s_skins.push_back({
        "MojangSteve",
        "Mojang Steve",
        "/skins/MojangSteve.png",
        "/skins/MojangSteve_32.png",
        "/skins/MojangSteve_Front.png"
    });

    s_skins.push_back({
        "MojangAlex",
        "Mojang Alex",
        "/skins/MojangAlex.png",
        "/skins/MojangAlex_32.png",
        "/skins/MojangAlex_Front.png"
    });

    s_skins.push_back({
        "Crocodile",
        "Crocodile",
        "/skins/Crocodile.png",
        "/skins/Crocodile_32.png",
        "/skins/Crocodile_Front.png"
    });

    s_skins.push_back({
        "Stampy",
        "Dust An Elysian Tail Fidget",
        "/skins/Stampy.png",
        "/skins/Stampy_32.png",
        "/skins/Stampy_Front.png"
    });

    s_skins.push_back({
        "LegacySquid",
        "Minecraft Squid",
        "/skins/LegacySquid.png",
        "/skins/LegacySquid_32.png",
        "/skins/LegacySquid_Front.png"
    });

    s_initialized = true;
}

const std::vector<SkinEntry>& SkinManager::getSkins()
{
    init();
    return s_skins;
}

int SkinManager::getSkinCount()
{
    init();
    return static_cast<int>(s_skins.size());
}

const SkinEntry* SkinManager::getSkin(int index)
{
    init();
    if (s_skins.empty())
        return nullptr;
    const int count = static_cast<int>(s_skins.size());
    const int wrapped = ((index % count) + count) % count;
    return &s_skins[wrapped];
}

const SkinEntry* SkinManager::getSkinById(const std::string& id)
{
    init();
    for (const auto& skin : s_skins)
    {
        if (skin.id == id)
            return &skin;
    }
    return nullptr;
}

int SkinManager::getIndexById(const std::string& id)
{
    init();
    for (std::size_t i = 0; i < s_skins.size(); ++i)
    {
        if (s_skins[i].id == id)
            return static_cast<int>(i);
    }
    return 0; // default to Steve
}

std::string SkinManager::getSelectedSkinId()
{
    return s_selectedId;
}

void SkinManager::setSelectedSkinId(const std::string& id)
{
    init();
    if (getSkinById(id) != nullptr)
        s_selectedId = id;
}

int SkinManager::getSelectedIndex()
{
    return getIndexById(s_selectedId);
}

void SkinManager::setSelectedIndex(int index)
{
    const SkinEntry* skin = getSkin(index);
    if (skin != nullptr)
        s_selectedId = skin->id;
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
