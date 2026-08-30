// Resource_ps2.cpp — PS2 implementation of Resource::getResource().
//
// Normal game resources live on the removable-storage filesystem. Return a
// file stream directly instead of reading the entire file into an aligned
// buffer and copying it again into an istringstream. Buffered whole-file reads
// stay in Ps2Assets for consumers that actually need DMA-ready memory (IRX,
// ADPCM and the small pre-game PNG loaders).

#ifdef PS2_PLATFORM

#include "java/Resource.h"
#include "java/String.h"
#include "net/minecraft/src/GameResources.h"

#include <stdexcept>
#include <string>

namespace Resource
{

std::istream* getResource(const jstring& name)
{
    auto input = GameResources::open(static_cast<const std::string&>(name));
    if (input)
        return input.release();

    throw std::runtime_error("Failed to open resource " + static_cast<const std::string&>(name));
}

} // namespace Resource

#endif // PS2_PLATFORM
