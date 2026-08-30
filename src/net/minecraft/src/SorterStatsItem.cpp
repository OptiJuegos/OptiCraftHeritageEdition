#include "SorterStatsItem.h"
#include "GuiSlotStatsItem.h"
#include "GuiStats.h"
#include "StatCrafting.h"
#include "StatList.h"
#include "StatFileWriter.h"

SorterStatsItem::SorterStatsItem(GuiSlotStatsItem *guislotstatsitem, GuiStats *guistats)
	: field_27372_b(guislotstatsitem)
	, field_27373_a(guistats)
{
}

int_t SorterStatsItem::compare(StatCrafting *a, StatCrafting *b)
{
	int_t i = a->getItemID();
	int_t j = b->getItemID();
	StatBase *statbase  = nullptr;
	StatBase *statbase1 = nullptr;
	if (field_27372_b->sortColumn == 0)
	{
		statbase  = StatList::breakItemStatArray[i];
		statbase1 = StatList::breakItemStatArray[j];
	}
	else if (field_27372_b->sortColumn == 1)
	{
		statbase  = StatList::craftItemStatArray[i];
		statbase1 = StatList::craftItemStatArray[j];
	}
	else if (field_27372_b->sortColumn == 2)
	{
		statbase  = StatList::useItemStatArray[i];
		statbase1 = StatList::useItemStatArray[j];
	}
	if (statbase != nullptr || statbase1 != nullptr)
	{
		if (statbase  == nullptr) return  1;
		if (statbase1 == nullptr) return -1;
		int_t k = GuiStats::getStatFileWriter(field_27372_b->parentGui)->writeStat(statbase);
		int_t l = GuiStats::getStatFileWriter(field_27372_b->parentGui)->writeStat(statbase1);
		if (k != l)
			return (k - l) * field_27372_b->sortDirection;
	}
	return i - j;
}
