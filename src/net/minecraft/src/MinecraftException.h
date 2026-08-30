#pragma once

#include <stdexcept>
#include <string>

// net.minecraft.src.MinecraftException
class MinecraftException : public std::runtime_error
{
public:
	MinecraftException(const std::string &s) : std::runtime_error(s) {}
};
