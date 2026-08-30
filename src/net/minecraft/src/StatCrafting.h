#pragma once

#include "StatBase.h"

// net.minecraft.src.StatCrafting
class StatCrafting : public StatBase
{
public:
	StatCrafting(int_t i, const std::string &s, int_t j);

	int_t getItemID();

private:
	int_t itemID;
};
