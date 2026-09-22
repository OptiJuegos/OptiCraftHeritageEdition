#include "ModManager.h"
#include "Minecraft.h"
#include "java/File.h"
#include "platform/Storage.h"
#include "platform/Log.h"
#include "DynamicMod.h"

// Built-in mods
#include "toomanyitems/TooManyItemsMod.h"
#include "reiminimap/ReiMinimapMod.h"

#include <cstdio>

ModManager::ModManager()
    : mc(nullptr)
    , initialized(false)
{
}

ModManager::~ModManager()
{
    shutdown();
}

ModManager &ModManager::getInstance()
{
    static ModManager instance;
    return instance;
}

void ModManager::init(Minecraft *mcInstance)
{
    if (initialized)
        return;

    mc = mcInstance;
    initialized = true;

    // Scan and load installed .ochpack packages from the persistent game mods directory
    scanAndLoadPacks();

    // Load persisted enabled/disabled state from storage
    load();

    // Initialize all registered mods
    for (auto &mod : mods)
    {
        try
        {
            mod->onInit(mc);
        }
        catch (...)
        {
            MC_LOG_WARN("mods", "Failed to initialize mod: %s\n", mod->getId().c_str());
        }
    }

    MC_LOG_INFO("mods", "ModManager initialized with %u mods.\n", static_cast<unsigned>(mods.size()));
}

void ModManager::shutdown()
{
    if (!initialized)
        return;

    save();
    mods.clear();
    mc = nullptr;
    initialized = false;
}

void ModManager::registerMod(std::unique_ptr<IMod> mod)
{
    if (!mod)
        return;
    mods.push_back(std::move(mod));
}

IMod *ModManager::getMod(const std::string &id)
{
    for (auto &mod : mods)
    {
        if (mod->getId() == id)
            return mod.get();
    }
    return nullptr;
}

bool ModManager::isModEnabled(const std::string &id)
{
    IMod *mod = getMod(id);
    return mod != nullptr && mod->isEnabled();
}

void ModManager::setModEnabled(const std::string &id, bool enabled)
{
    IMod *mod = getMod(id);
    if (mod != nullptr)
    {
        mod->setEnabled(enabled);
        save();
    }
}

std::string ModManager::getGameModsDir() const
{
    File *dataDir = Minecraft::getMinecraftDir();
    if (dataDir != nullptr)
    {
        return PlatformStorage::join(dataDir->toString(), "mods");
    }
    return "mods";
}

std::string ModManager::getInstalledModVersion(const std::string &id) const
{
    for (const auto &mod : mods)
    {
        if (mod->getId() == id)
            return mod->getVersion();
    }
    return "";
}

void ModManager::scanAndLoadPacks()
{
    std::string modsDir = getGameModsDir();
    PlatformStorage::mkdirs(modsDir);

    auto packs = OchPackReader::scanDirectory(modsDir);
    for (const auto &pack : packs)
    {
        IMod *existing = getMod(pack.id);
        if (existing != nullptr)
        {
            existing->setPackPath(pack.filePath);
            continue;
        }

        std::unique_ptr<IMod> newMod;
        if (pack.id == "toomanyitems")
        {
            newMod = std::make_unique<TooManyItemsMod>();
        }
        else if (pack.id == "reiminimap")
        {
            newMod = std::make_unique<ReiMinimapMod>();
        }
        else
        {
            newMod = std::make_unique<DynamicMod>(pack);
        }

        if (newMod)
        {
            newMod->setPackPath(pack.filePath);
            if (initialized && mc != nullptr)
            {
                try
                {
                    newMod->onInit(mc);
                }
                catch (...)
                {
                }
            }
            registerMod(std::move(newMod));
        }
    }
}

bool ModManager::installModPack(const std::string &sourcePath, std::string &outError)
{
    OchPackInfo info;
    if (!OchPackReader::readInfo(sourcePath, info))
    {
        outError = "Failed to read .ochpack archive or missing mod.info.";
        return false;
    }

    std::string modsDir = getGameModsDir();
    if (!PlatformStorage::mkdirs(modsDir))
    {
        outError = "Failed to access/create game mods directory.";
        return false;
    }

    std::string destPath = PlatformStorage::join(modsDir, info.fileName);

    // Read source file data using OchPackReader for full optical disc & candidate support
    std::vector<unsigned char> data;
    if (!OchPackReader::readFileBytes(sourcePath, data))
    {
        if (!info.filePath.empty() && info.filePath != sourcePath)
        {
            OchPackReader::readFileBytes(info.filePath, data);
        }
    }

    if (data.empty())
    {
        outError = "Failed to read package file.";
        return false;
    }

    // Write to destination
    if (!PlatformStorage::writeFile(destPath, data.data(), data.size()))
    {
        outError = "Failed to write package to destination.";
        return false;
    }

    // Update or register mod
    info.filePath = destPath;
    IMod *existing = getMod(info.id);
    if (existing != nullptr)
    {
        existing->setPackPath(destPath);
        existing->setEnabled(true);
    }
    else
    {
        std::unique_ptr<IMod> newMod;
        if (info.id == "toomanyitems")
            newMod = std::make_unique<TooManyItemsMod>();
        else if (info.id == "reiminimap")
            newMod = std::make_unique<ReiMinimapMod>();
        else
            newMod = std::make_unique<DynamicMod>(info);

        if (newMod)
        {
            newMod->setPackPath(destPath);
            newMod->setEnabled(true);
            if (initialized && mc != nullptr)
            {
                try
                {
                    newMod->onInit(mc);
                }
                catch (...)
                {
                }
            }
            registerMod(std::move(newMod));
        }
    }

    save();
    return true;
}

bool ModManager::deleteMod(const std::string &id)
{
    for (auto it = mods.begin(); it != mods.end(); ++it)
    {
        if ((*it)->getId() == id)
        {
            std::string pack = (*it)->getPackPath();
            if (!pack.empty())
            {
                PlatformStorage::removeFile(pack);
                std::remove(pack.c_str());
            }

            mods.erase(it);
            save();
            return true;
        }
    }
    return false;
}

std::string ModManager::getConfigPath() const
{
    File *dataDir = Minecraft::getMinecraftDir();
    if (dataDir != nullptr)
    {
        return PlatformStorage::join(dataDir->toString(), "mods.txt");
    }
    return "mods.txt";
}

void ModManager::load()
{
    std::string path = getConfigPath();
    std::vector<unsigned char> bytes;
    if (!PlatformStorage::readFile(path, bytes) || bytes.empty())
        return;

    std::string content(bytes.begin(), bytes.end());
    size_t start = 0;
    while (start < content.size())
    {
        size_t end = content.find('\n', start);
        std::string line;
        if (end == std::string::npos)
        {
            line = content.substr(start);
            start = content.size();
        }
        else
        {
            line = content.substr(start, end - start);
            start = end + 1;
        }

        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
            line.pop_back();

        if (line.empty() || line[0] == '#')
            continue;

        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
            std::string id = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            bool enabled = (val == "true" || val == "1" || val == "ON" || val == "on");

            for (auto &mod : mods)
            {
                if (mod->getId() == id)
                {
                    mod->setEnabled(enabled);
                    break;
                }
            }
        }
    }
}

void ModManager::save()
{
    std::string path = getConfigPath();
    std::string content;
    content.reserve(256);
    content += "# OptiCraft Mods Configuration\n";
    for (const auto &mod : mods)
    {
        content += mod->getId();
        content += ":";
        content += (mod->isEnabled() ? "true" : "false");
        content += "\n";
    }

    PlatformStorage::writeFile(path, content.data(), content.size());
}

void ModManager::onTick()
{
    for (auto &mod : mods)
    {
        if (mod->isEnabled())
            mod->onTick();
    }
}

void ModManager::onRenderGameOverlay(GuiIngame *gui, int_t screenWidth, int_t screenHeight, float_t partialTick)
{
    for (auto &mod : mods)
    {
        if (mod->isEnabled())
            mod->onRenderGameOverlay(gui, screenWidth, screenHeight, partialTick);
    }
}

void ModManager::onDrawContainer(GuiContainer *container, int_t mouseX, int_t mouseY)
{
    for (auto &mod : mods)
    {
        if (mod->isEnabled())
            mod->onDrawContainer(container, mouseX, mouseY);
    }
}

bool ModManager::onContainerMouseClicked(GuiContainer *container, int_t x, int_t y, int_t button)
{
    for (auto &mod : mods)
    {
        if (mod->isEnabled())
        {
            if (mod->onContainerMouseClicked(container, x, y, button))
                return true;
        }
    }
    return false;
}

bool ModManager::onContainerKeyTyped(char_t c, int_t key)
{
    for (auto &mod : mods)
    {
        if (mod->isEnabled())
        {
            if (mod->onContainerKeyTyped(c, key))
                return true;
        }
    }
    return false;
}
