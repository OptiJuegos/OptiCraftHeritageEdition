#pragma once

#include <string>
#include "java/Type.h"

// net.minecraft.src.ChatLine
class ChatLine
{
public:
	ChatLine(const std::string &s) : message(s), updateCounter(0) {}

	std::string message;
	int_t updateCounter;
};
