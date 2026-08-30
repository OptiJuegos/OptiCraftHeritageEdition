#include "StatBase.h"

#include <sstream>
#include <iomanip>
#include <locale>
#include "IStatType.h"
#include "StatList.h"
#include "AchievementMap.h"
#include "StatTypeSimple.h"
#include "StatTypeTime.h"
#include "StatTypeDistance.h"

StatBase::StatBase(int_t i, const std::string &s, IStatType *istattype) :
	statId(i),
	statName(s),
	independent(false),
	statType(istattype)
{
}

StatBase::StatBase(int_t i, const std::string &s) :
	StatBase(i, s, simpleStatType)
{
}

StatBase* StatBase::setIndependent()
{
	independent = true;
	return this;
}

StatBase* StatBase::registerStat()
{
	auto it = StatList::statMap.find(statId);
	if (it != StatList::statMap.end())
	{
		std::ostringstream oss;
		oss << "Duplicate stat id: "" << it->second->statName << "" and "" << statName << "" at id " << statId;
		throw std::runtime_error(oss.str());
	}

	StatList::allStats.push_back(this);
	StatList::statMap[statId] = this;
	statGuid = AchievementMap::getGuid(statId);
	return this;
}

bool StatBase::isAchievement()
{
	return false;
}

std::string StatBase::format(int_t i)
{
	return statType->format(i);
}

std::string StatBase::toString()
{
	return statName;
}

std::string StatBase::numberFormat(int_t i)
{
	std::ostringstream oss;
	oss.imbue(std::locale(""));
	oss << i;
	return oss.str();
}

std::string StatBase::decimalFormat(double d)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << d;
	return oss.str();
}

IStatType *StatBase::simpleStatType = new StatTypeSimple();
IStatType *StatBase::timeStatType = new StatTypeTime();
IStatType *StatBase::distanceStatType = new StatTypeDistance();
