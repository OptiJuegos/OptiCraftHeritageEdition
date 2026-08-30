#pragma once

#include <string>
#include "GuiSlot.h"
#include <vector>
#include <functional>

class GuiStats;
class StatCrafting;
class Tessellator;

// net.minecraft.src.GuiSlotStats
class GuiSlotStats : public GuiSlot
{
public:
	GuiSlotStats(GuiStats *guistats);
	int_t getSize() override;

protected:
	void elementClicked(int_t index, bool doubleClicked) override;
	bool isSelected(int_t index) override;
	void drawBackground() override;

	void drawHeader(int_t x, int_t y, Tessellator *tess) override;
	void clickedHeader(int_t x, int_t y) override;
	void renderHoverState(int_t mouseX, int_t mouseY) override;

	StatCrafting *getStatAt(int_t index);
	virtual std::string getColumnLabel(int_t column) = 0;

	void drawStatValue(StatCrafting *statcrafting, int_t x, int_t y, bool highlight);
	void drawItemTooltip(StatCrafting *statcrafting, int_t mouseX, int_t mouseY);
	void toggleSort(int_t column);

public:
	// Public so SorterStats* comparator classes can access them
	int_t hoveredColumn;
	std::vector<StatCrafting *> statEntries;
	std::function<int(StatCrafting *, StatCrafting *)> statComparator;
	int_t sortColumn;
	int_t sortDirection;
	GuiStats *parentGui;
};
