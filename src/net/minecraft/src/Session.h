#pragma once

#include <string>
#include <vector>

class Block;

// net.minecraft.src.Session
class Session
{
public:
	Session(const std::string &s, const std::string &s1);

	static std::vector<Block *> registeredBlocksList;
	static void initialize();

	std::string username;
	std::string sessionId;
	std::string mpPassParameter;
};
