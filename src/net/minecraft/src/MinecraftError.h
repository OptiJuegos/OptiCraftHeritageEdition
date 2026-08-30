#pragma once

#include <stdexcept>

// net.minecraft.src.MinecraftError
class MinecraftError : public std::exception
{
public:
	MinecraftError() = default;
};
