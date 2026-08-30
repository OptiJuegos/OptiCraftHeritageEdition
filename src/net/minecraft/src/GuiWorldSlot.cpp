#include "GuiWorldSlot.h"
#include "GuiSelectWorld.h"
#include "GuiButton.h"
#include "SaveFormatComparator.h"
#include "MathHelper.h"
#include "StatCollector.h"

GuiWorldSlot::GuiWorldSlot(GuiSelectWorld *parent)
	: GuiSlot(parent->mc, parent->width, parent->height, 32, parent->height - 64, 36)
	, parentWorldGui(parent)
{
}

int_t GuiWorldSlot::getSize()
{
	return (int_t)parentWorldGui->saveList.size();
}

void GuiWorldSlot::elementClicked(int_t index, bool doubleClicked)
{
	parentWorldGui->selectedWorld = index;
	bool valid = parentWorldGui->selectedWorld >= 0
	          && parentWorldGui->selectedWorld < getSize();
	parentWorldGui->buttonSelect->enabled = valid;
	parentWorldGui->buttonRename->enabled = valid;
	parentWorldGui->buttonDelete->enabled = valid;
	if (doubleClicked && valid)
		parentWorldGui->selectWorld(index);
}

bool GuiWorldSlot::isSelected(int_t index)
{
	return index == parentWorldGui->selectedWorld;
}

int_t GuiWorldSlot::getContentHeight()
{
	return getSize() * 36;
}

void GuiWorldSlot::drawBackground()
{
	parentWorldGui->drawDefaultBackground();
}

void GuiWorldSlot::drawSlot(int_t i, int_t x, int_t y, int_t h, Tessellator *tess)
{
	SaveFormatComparator *entry = parentWorldGui->saveList[i];
	std::string name = entry->getDisplayName();
	if (name.empty())
		name = parentWorldGui->worldLabel + " " + std::to_string(i + 1);

	std::string line2 = entry->getFileName() + " (" +
		parentWorldGui->formatDate(entry->getLastTimePlayed()) + ")";

	std::string line3;
	if (entry->requiresConversion())
	{
		line3 = parentWorldGui->conversionLabel;
	}
	else
	{
		const int_t gameType = entry->getGameType();
		if (gameType >= 0 && gameType < 2)
			line3 = parentWorldGui->gameModeLabels[gameType];
		if (entry->isHardcoreModeEnabled())
			line3 = StatCollector::translateToLocal("gameMode.hardcore");
	}

	parentWorldGui->drawString(parentWorldGui->fontRenderer, name,  x + 2, y + 1,  0xffffff);
	parentWorldGui->drawString(parentWorldGui->fontRenderer, line2, x + 2, y + 12, 0x808080);
	parentWorldGui->drawString(parentWorldGui->fontRenderer, line3, x + 2, y + 22, 0x808080);
}
