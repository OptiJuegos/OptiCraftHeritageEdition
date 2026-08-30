#pragma once

#include "GuiScreen.h"
#include <vector>
#include <string>
#include <ctime>

class GuiWorldSlot;
class SaveFormatComparator;
class ISaveFormat;

// net.minecraft.src.GuiSelectWorld
class GuiSelectWorld : public GuiScreen
{
	friend class GuiWorldSlot;

public:
	GuiSelectWorld(GuiScreen *parent);
	~GuiSelectWorld() override;

	void initGui() override;
	void initButtons();

private:
	void loadSaves();

protected:
	std::string getSaveFileName(int_t index);
	std::string getSaveName(int_t index);

public:
	void actionPerformed(GuiButton *button) override;
	void selectWorld(int_t index);
	void deleteWorld(bool confirmed, int_t index) override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	GuiScreen *parentScreen;
	std::string screenTitle;

private:
	std::string formatDate(long_t timestamp) const;

	bool selected;
	int_t selectedWorld;
	std::vector<SaveFormatComparator *> saveList;
	GuiWorldSlot *worldSlotContainer;
	std::string worldLabel;
	std::string conversionLabel;
	std::string gameModeLabels[2];
	bool deleting;
	GuiButton *buttonRename;
	GuiButton *buttonSelect;
	GuiButton *buttonDelete;
};
