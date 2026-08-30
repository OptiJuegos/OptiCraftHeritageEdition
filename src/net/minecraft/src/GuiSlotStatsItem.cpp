#include "GuiSlotStatsItem.h"
#include "GuiStats.h"
#include "SorterStatsItem.h"
#include "StatList.h"
#include "StatCrafting.h"
#include "StatFileWriter.h"
#include "Block.h"
#include "Tessellator.h"

GuiSlotStatsItem::GuiSlotStatsItem(GuiStats *guistats)
	: GuiSlotStats(guistats)
{
	StatFileWriter *sfw = GuiStats::getStatFileWriter(guistats);
	for (StatBase *sb : StatList::itemStats)
	{
		StatCrafting *statcrafting = dynamic_cast<StatCrafting *>(sb);
		if (statcrafting == nullptr) continue;
		int_t i = statcrafting->getItemID();
		bool flag = false;
		if (sfw->writeStat(statcrafting) > 0) flag = true;
		else if (StatList::breakItemStatArray[i] != nullptr && sfw->writeStat(StatList::breakItemStatArray[i]) > 0) flag = true;
		else if (StatList::craftItemStatArray[i] != nullptr && sfw->writeStat(StatList::craftItemStatArray[i]) > 0) flag = true;
		if (flag) statEntries.push_back(statcrafting);
	}
	SorterStatsItem *sorter = new SorterStatsItem(this, guistats);
	statComparator = [sorter](StatCrafting *a, StatCrafting *b) { return sorter->compare(a, b); };
}

void GuiSlotStatsItem::drawHeader(int_t x, int_t y, Tessellator *tess)
{
	GuiSlotStats::drawHeader(x, y, tess);
	if (hoveredColumn == 0)
		GuiStats::drawSlotIconAt(parentGui, (x + 115) - 18 + 1, y + 1 + 1, 72, 18);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 115) - 18,     y + 1,     72, 18);
	if (hoveredColumn == 1)
		GuiStats::drawSlotIconAt(parentGui, (x + 165) - 18 + 1, y + 1 + 1, 18, 18);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 165) - 18,     y + 1,     18, 18);
	if (hoveredColumn == 2)
		GuiStats::drawSlotIconAt(parentGui, (x + 215) - 18 + 1, y + 1 + 1, 36, 18);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 215) - 18,     y + 1,     36, 18);
}

void GuiSlotStatsItem::drawSlot(int_t i, int_t x, int_t y, int_t h, Tessellator *tess)
{
	StatCrafting *statcrafting = getStatAt(i);
	int_t i1 = statcrafting->getItemID();
	GuiStats::drawItemIconAt(parentGui, x + 40, y, i1);
	drawStatValue((StatCrafting *)StatList::breakItemStatArray[i1], x + 115, y, i % 2 == 0);
	drawStatValue((StatCrafting *)StatList::craftItemStatArray[i1], x + 165, y, i % 2 == 0);
	drawStatValue(statcrafting,                                      x + 215, y, i % 2 == 0);
}

std::string GuiSlotStatsItem::getColumnLabel(int_t i)
{
	if (i == 1) return "stat.crafted";
	if (i == 2) return "stat.used";
	return "stat.depleted";
}
