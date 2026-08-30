#pragma once

#include <string>
#include "java/String.h"

// net.minecraft.src.SoundPoolEntry
// Java URL replaced with filesystem path string
class SoundPoolEntry
{
public:
	SoundPoolEntry(const jstring &s, const std::string &url)
		: soundName(s), soundUrl(url) {}

	jstring soundName;
	std::string soundUrl;
};
