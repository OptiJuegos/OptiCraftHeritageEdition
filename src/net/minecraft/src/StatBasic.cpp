#include "StatBasic.h"

#include "StatList.h"

StatBasic::StatBasic(int_t i, const std::string &s, IStatType *istattype) :
	StatBase(i, s, istattype)
{
}

StatBasic::StatBasic(int_t i, const std::string &s) :
	StatBase(i, s)
{
}

StatBase* StatBasic::registerStat()
{
	StatBase::registerStat();
	StatList::generalStats.push_back(this);
	return this;
}
