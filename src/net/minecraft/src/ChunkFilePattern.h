#pragma once

#include <string>

// net.minecraft.src.ChunkFilePattern
// Matches chunk save-file names: c.<x36>.<z36>.dat
// Hand-written matcher instead of std::regex: instantiating the regex engine
// costs several hundred KB of .text on the PS2 ELF for these two patterns.
class ChunkFilePattern
{
private:
	ChunkFilePattern() = delete;

	// Length of a -?[0-9a-z]+ token at pos (0 if none).
	static size_t tokenLen(const std::string &s, size_t pos)
	{
		size_t i = pos;
		if (i < s.size() && s[i] == '-') ++i;
		size_t digits = 0;
		while (i < s.size() && ((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z')))
		{
			++i;
			++digits;
		}
		return digits ? (i - pos) : 0;
	}

public:
	// Full match of c\.(-?[0-9a-z]+)\.(-?[0-9a-z]+)\.dat
	// On success outX/outZ (if given) receive the raw base-36 coordinate tokens.
	static bool parse(const std::string &name, std::string *outX = nullptr, std::string *outZ = nullptr)
	{
		if (name.size() < 2 || name[0] != 'c' || name[1] != '.')
			return false;
		const size_t x0 = 2;
		const size_t xl = tokenLen(name, x0);
		if (xl == 0 || x0 + xl >= name.size() || name[x0 + xl] != '.')
			return false;
		const size_t z0 = x0 + xl + 1;
		const size_t zl = tokenLen(name, z0);
		if (zl == 0)
			return false;
		if (name.compare(z0 + zl, std::string::npos, ".dat") != 0)
			return false;
		if (outX) *outX = name.substr(x0, xl);
		if (outZ) *outZ = name.substr(z0, zl);
		return true;
	}

	static bool accept(const std::string &filename)
	{
		return parse(filename);
	}
};
