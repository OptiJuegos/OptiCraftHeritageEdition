#include "GuiSlotStats.h"
#include "java/String.h"
#include "GuiStats.h"
#include "StatCrafting.h"
#include "StatFileWriter.h"
#include "StringTranslate.h"
#include "FontRenderer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "Tessellator.h"
#include "SoundManager.h"
#include "pc/lwjgl/Mouse.h"
#include <algorithm>

GuiSlotStats::GuiSlotStats(GuiStats *guistats)
	: GuiSlot(GuiStats::getMc(guistats), guistats->width, guistats->height, 32, guistats->height - 64, 20)
	, parentGui(guistats)
	, hoveredColumn(-1)
	, sortColumn(-1)
	, sortDirection(0)
{
	setShowSelectionBox(false);
	setHasHeader(true, 20);
}

void GuiSlotStats::elementClicked(int_t i, bool flag)
{
}

bool GuiSlotStats::isSelected(int_t i)
{
	return false;
}

void GuiSlotStats::drawBackground()
{
	parentGui->drawDefaultBackground();
}

int_t GuiSlotStats::getSize()
{
	return (int_t)statEntries.size();
}

void GuiSlotStats::drawHeader(int_t x, int_t y, Tessellator *tess)
{
	if (!lwjgl::Mouse::isButtonDown(0))
	{
		hoveredColumn = -1;
	}
	if (hoveredColumn == 0)
		GuiStats::drawSlotIconAt(parentGui, (x + 115) - 18 + 1, y + 1 + 1, 0, 0);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 115) - 18,     y + 1,     0, 18);

	if (hoveredColumn == 1)
		GuiStats::drawSlotIconAt(parentGui, (x + 165) - 18 + 1, y + 1 + 1, 0, 0);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 165) - 18,     y + 1,     0, 18);

	if (hoveredColumn == 2)
		GuiStats::drawSlotIconAt(parentGui, (x + 215) - 18 + 1, y + 1 + 1, 0, 0);
	else
		GuiStats::drawSlotIconAt(parentGui, (x + 215) - 18,     y + 1,     0, 18);

	if (sortColumn != -1)
	{
		int_t iconX = 79;
		int_t texY  = 18;
		if (sortColumn == 1)       iconX = 129;
		else if (sortColumn == 2)  iconX = 179;
		if (sortDirection == 1)    texY  = 36;
		GuiStats::drawSlotIconAt(parentGui, x + iconX, y + 1, texY, 0);
	}
}

void GuiSlotStats::clickedHeader(int_t x, int_t y)
{
	hoveredColumn = -1;
	if (x >= 79  && x < 115) hoveredColumn = 0;
	else if (x >= 129 && x < 165) hoveredColumn = 1;
	else if (x >= 179 && x < 215) hoveredColumn = 2;
	if (hoveredColumn >= 0)
	{
		toggleSort(hoveredColumn);
		GuiStats::getMc(parentGui)->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
	}
}

void GuiSlotStats::renderHoverState(int_t mouseX, int_t mouseY)
{
	if (mouseY < top || mouseY > bottom) return;
	int_t idx = getElementAt(mouseX, mouseY);
	int_t l   = parentGui->width / 2 - 92 - 16;
	if (idx >= 0)
	{
		if (mouseX < l + 40 || mouseX > l + 40 + 20) return;
		StatCrafting *stat = getStatAt(idx);
		drawItemTooltip(stat, mouseX, mouseY);
	}
	else
	{
		std::string s = "";
		if      (mouseX >= (l + 115) - 18 && mouseX <= l + 115) s = getColumnLabel(0);
		else if (mouseX >= (l + 165) - 18 && mouseX <= l + 165) s = getColumnLabel(1);
		else if (mouseX >= (l + 215) - 18 && mouseX <= l + 215) s = getColumnLabel(2);
		else return;
		s = StringTranslate::getInstance()->translateKey(s);
		s = String::trimJava(s);
		if (s.empty()) return;
		int_t tx  = mouseX + 12;
		int_t ty  = mouseY - 12;
		int_t sw  = GuiStats::getFontRenderer(parentGui)->getStringWidth(s);
		GuiStats::drawGradientRectStatic(parentGui, tx - 3, ty - 3, tx + sw + 3, ty + 8 + 3, 0xc0000000, 0xc0000000);
		GuiStats::getFontRenderer(parentGui)->drawStringWithShadow(s, tx, ty, -1);
	}
}

StatCrafting *GuiSlotStats::getStatAt(int_t index)
{
	return index >= 0 && (std::size_t)index < statEntries.size() ? statEntries[(std::size_t)index] : nullptr;
}

void GuiSlotStats::drawStatValue(StatCrafting *statcrafting, int_t x, int_t y, bool highlight)
{
	int_t color = highlight ? 0xffffff : 0x909090;
	FontRenderer *fr = GuiStats::getFontRenderer(parentGui);
	if (statcrafting != nullptr)
	{
		StatFileWriter *sfw = GuiStats::getStatFileWriter(parentGui);
		std::string s = statcrafting->format(sfw->writeStat(statcrafting));
		parentGui->drawString(fr, s, x - fr->getStringWidth(s), y + 5, color);
	}
	else
	{
		std::string s1 = "-";
		parentGui->drawString(fr, s1, x - fr->getStringWidth(s1), y + 5, color);
	}
}

void GuiSlotStats::drawItemTooltip(StatCrafting *statcrafting, int_t mouseX, int_t mouseY)
{
	if (statcrafting == nullptr) return;
	int itemID = statcrafting->getItemID();
	if (!ItemStack::isValidItemID(itemID)) return;
	Item *item = Item::itemsList[itemID];
	std::string s = StringTranslate::getInstance()->translateNamedKey(item->getItemName());
	s = String::trimJava(s);
	if (s.empty()) return;
	int_t tx = mouseX + 12;
	int_t ty = mouseY - 12;
	int_t sw = GuiStats::getFontRenderer(parentGui)->getStringWidth(s);
	GuiStats::drawGradientRectStatic(parentGui, tx - 3, ty - 3, tx + sw + 3, ty + 8 + 3, 0xc0000000, 0xc0000000);
	GuiStats::getFontRenderer(parentGui)->drawStringWithShadow(s, tx, ty, -1);
}

void GuiSlotStats::toggleSort(int_t column)
{
	if (column != sortColumn)
	{
		sortColumn    = column;
		sortDirection = -1;
	}
	else if (sortDirection == -1)
	{
		sortDirection = 1;
	}
	else
	{
		sortColumn    = -1;
		sortDirection = 0;
	}
	std::stable_sort(statEntries.begin(), statEntries.end(), [this](StatCrafting *a, StatCrafting *b) {
		return statComparator(a, b) < 0;
	});
}
