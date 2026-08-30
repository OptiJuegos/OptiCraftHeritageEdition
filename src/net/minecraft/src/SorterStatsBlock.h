#pragma once

#include "java/Type.h"

class StatCrafting;
class GuiSlotStatsBlock;
class GuiStats;

// net.minecraft.src.SorterStatsBlock
class SorterStatsBlock
{
public:
	SorterStatsBlock(GuiSlotStatsBlock *guislotstatsblock, GuiStats *guistats);

	int_t compare(StatCrafting *a, StatCrafting *b);

	GuiStats          *field_27299_a;
	GuiSlotStatsBlock *field_27298_b;
};
