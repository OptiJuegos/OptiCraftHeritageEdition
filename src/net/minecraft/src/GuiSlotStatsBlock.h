#pragma once

#include "GuiSlotStats.h"

class GuiStats;
class Tessellator;

// net.minecraft.src.GuiSlotStatsBlock
class GuiSlotStatsBlock : public GuiSlotStats
{
public:
	GuiSlotStatsBlock(GuiStats *guistats);

protected:
	void drawHeader(int_t x, int_t y, Tessellator *tess) override;
	void drawSlot(int_t index, int_t x, int_t y, int_t height, Tessellator *tess) override;
	std::string getColumnLabel(int_t column) override;
};
