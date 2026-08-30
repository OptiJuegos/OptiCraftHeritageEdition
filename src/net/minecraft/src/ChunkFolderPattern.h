#pragma once

#include <string>

// net.minecraft.src.ChunkFolderPattern
// Matches 1- or 2-char base-36 folder names used by the chunk save layout.
// Hand-written matcher instead of std::regex (see ChunkFilePattern.h).
class ChunkFolderPattern
{
private:
	ChunkFolderPattern() = delete;

	static bool isBase36(char c)
	{
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
	}

public:
	// [0-9a-z]|([0-9a-z][0-9a-z])
	static bool accept(const std::string &dirName)
	{
		if (dirName.size() != 1 && dirName.size() != 2)
			return false;
		for (char c : dirName)
			if (!isBase36(c))
				return false;
		return true;
	}
};
