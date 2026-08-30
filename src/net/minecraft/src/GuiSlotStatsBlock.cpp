#include "GuiSlotStatsBlock.h"
#include "GuiStats.h"
#include "SorterStatsBlock.h"
#include "StatList.h"
#include "StatCrafting.h"
#include "StatFileWriter.h"
#include "Tessellator.h"

GuiSlotStatsBlock::GuiSlotStatsBlock(GuiStats *guistats)
	: GuiSlotStats(guistats)
{
	StatFileWriter *sfw = GuiStats::getStatFileWriter(guistats);
	for (StatBase *sb : StatList::objectCraftStats)
	{
		StatCrafting *statcrafting = dynamic_cast<StatCrafting *>(sb);
		if (statcrafting == nullptr) continue;
		int_t i = statcrafting->getItemID();
		bool flag = false;
		if (sfw->writeStat(statcrafting) > 0) flag = true;
		else if (StatList::useItemStatArray[i]   != nullptr && sfw->writeStat(StatList::useItemStatArray[i])   > 0) flag = true;
		else if (StatList::craftItemStatArray[i] != nullptr && sfw->writeStat(StatList::craftItemStatArray[i]) > 0) flag = true;
		if (flag) statEntries.push_back(statcrafting);
	}
	SorterStatsBlock *sorter = new SorterStatsBlock(this, guistats);
	statComparator = [sorter](StatCrafting *a, StatCrafting *b) { return sorter->compare(a, b); };
}

void GuiSlotStatsBlock::drawHeader(int_t x, int_t y, Tessellator *tess)
{
	GuiSlotStats::drawHeader(x, y, tess);
	if (hoveredColumn == 0)
		GuiStats::drawSlotIconAt(parentGui, (x + 115) - 18 + 1, y + 1 + 1, 18, 18);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 115) - 18,     y + 1,     18, 18);
	if (hoveredColumn == 1)
		GuiStats::drawSlotIconAt(parentGui, (x + 165) - 18 + 1, y + 1 + 1, 36, 18);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 165) - 18,     y + 1,     36, 18);
	if (hoveredColumn == 2)
		GuiStats::drawSlotIconAt(parentGui, (x + 215) - 18 + 1, y + 1 + 1, 54, 18);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 215) - 18,     y + 1,     54, 18);
}

void GuiSlotStatsBlock::drawSlot(int_t i, int_t x, int_t y, int_t h, Tessellator *tess)
{
	StatCrafting *statcrafting = getStatAt(i);
	int_t i1 = statcrafting->getItemID();
	GuiStats::drawItemIconAt(parentGui, x + 40, y, i1);
	drawStatValue((StatCrafting *)StatList::craftItemStatArray[i1], x + 115, y, i % 2 == 0);
	drawStatValue((StatCrafting *)StatList::useItemStatArray[i1],   x + 165, y, i % 2 == 0);
	drawStatValue(statcrafting,                                      x + 215, y, i % 2 == 0);
}

std::string GuiSlotStatsBlock::getColumnLabel(int_t i)
{
	if (i == 0) return "stat.crafted";
	if (i == 1) return "stat.used";
	return "stat.mined";
}
