#pragma once
#include <string>
#include <fstream>
#include <memory>

// Centralized resource path resolution.
// All resource lookups should go through here instead of each file having
// its own getExeDir() copy.
namespace GameResources
{
    // Legacy platform base directory. New resource code should prefer the
    // explicit asset/audio roots below instead of assuming an install layout.
    std::string getExeDir();

    // Returns the root directories used by local game resources. Console ports
    // can keep their data/ install layout without leaking it into callers.
    std::string getAssetsDir();
    std::string getAudioResourcesDir();

    // Resolves a Minecraft-style path (e.g. "/terrain.png" or "terrain.png")
    // inside the platform asset root. Audio uses getAudioResourcesDir().
    // Returns the path if it exists, or an empty string otherwise.
    std::string resolve(const std::string &mcPath);

    // Opens a resource stream. Returns nullptr if not found.
    std::unique_ptr<std::ifstream> open(const std::string &mcPath);
}
