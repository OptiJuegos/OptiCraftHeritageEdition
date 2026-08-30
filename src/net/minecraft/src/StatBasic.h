#pragma once

#include "StatBase.h"

// net.minecraft.src.StatBasic
class StatBasic : public StatBase
{
public:
	StatBasic(int_t i, const std::string &s, IStatType *istattype);
	StatBasic(int_t i, const std::string &s);

	StatBase* registerStat() override;
};
