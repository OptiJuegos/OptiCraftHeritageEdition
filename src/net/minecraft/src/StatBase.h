#pragma once

#include <string>
#include <memory>
#include "java/Type.h"

class IStatType;

// net.minecraft.src.StatBase
class StatBase
{
public:
	StatBase(int_t i, const std::string &s, IStatType *istattype);
	StatBase(int_t i, const std::string &s);
	virtual ~StatBase() = default;

	virtual StatBase* setIndependent();
	virtual StatBase* registerStat();
	virtual bool isAchievement();
	virtual std::string format(int_t i);
	virtual std::string toString();

	static std::string numberFormat(int_t i);
	static std::string decimalFormat(double d);

	int_t statId;
	std::string statName;
	bool independent;
	std::string statGuid;

	static IStatType *simpleStatType;
	static IStatType *timeStatType;
	static IStatType *distanceStatType;

private:
	IStatType *statType;
};
