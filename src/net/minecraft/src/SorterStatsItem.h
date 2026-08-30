#pragma once

#include "java/Type.h"

class StatCrafting;
class GuiSlotStatsItem;
class GuiStats;

// net.minecraft.src.SorterStatsItem
class SorterStatsItem
{
public:
	SorterStatsItem(GuiSlotStatsItem *guislotstatsitem, GuiStats *guistats);

	int_t compare(StatCrafting *a, StatCrafting *b);

	GuiStats         *field_27373_a;
	GuiSlotStatsItem *field_27372_b;
};
