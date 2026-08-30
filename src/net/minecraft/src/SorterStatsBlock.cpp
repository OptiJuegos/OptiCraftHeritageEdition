#include "SorterStatsBlock.h"
#include "GuiSlotStatsBlock.h"
#include "GuiStats.h"
#include "StatCrafting.h"
#include "StatList.h"
#include "StatFileWriter.h"

SorterStatsBlock::SorterStatsBlock(GuiSlotStatsBlock *guislotstatsblock, GuiStats *guistats)
	: field_27298_b(guislotstatsblock)
	, field_27299_a(guistats)
{
}

int_t SorterStatsBlock::compare(StatCrafting *a, StatCrafting *b)
{
	int_t i = a->getItemID();
	int_t j = b->getItemID();
	StatBase *statbase  = nullptr;
	StatBase *statbase1 = nullptr;
	if (field_27298_b->sortColumn == 2)
	{
		statbase  = StatList::mineBlockStatArray[i];
		statbase1 = StatList::mineBlockStatArray[j];
	}
	else if (field_27298_b->sortColumn == 0)
	{
		statbase  = StatList::craftItemStatArray[i];
		statbase1 = StatList::craftItemStatArray[j];
	}
	else if (field_27298_b->sortColumn == 1)
	{
		statbase  = StatList::useItemStatArray[i];
		statbase1 = StatList::useItemStatArray[j];
	}
	if (statbase != nullptr || statbase1 != nullptr)
	{
		if (statbase  == nullptr) return  1;
		if (statbase1 == nullptr) return -1;
		int_t k = GuiStats::getStatFileWriter(field_27298_b->parentGui)->writeStat(statbase);
		int_t l = GuiStats::getStatFileWriter(field_27298_b->parentGui)->writeStat(statbase1);
		if (k != l)
			return (k - l) * field_27298_b->sortDirection;
	}
	return i - j;
}
