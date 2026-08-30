#include "GuiSelectWorld.h"
#include "GuiWorldSlot.h"
#include "GuiButton.h"
#include "GuiYesNo.h"
#include "GuiCreateWorld.h"
#include "GuiRenameWorld.h"
#include "StringTranslate.h"
#include "ISaveFormat.h"
#include "SaveFormatComparator.h"
#include "MathHelper.h"
#include "PlayerControllerSP.h"
#include "PlayerControllerCreative.h"
#include "WorldSettings.h"
#include "Minecraft.h"
#include "java/String.h"
#include <algorithm>
#include <ctime>

GuiSelectWorld::GuiSelectWorld(GuiScreen *parent)
	: screenTitle("Select world")
	, selected(false)
	, selectedWorld(-1)
	, parentScreen(parent)
	, worldSlotContainer(nullptr)
	, deleting(false)
	, buttonRename(nullptr)
	, buttonSelect(nullptr)
	, buttonDelete(nullptr)
{
}

GuiSelectWorld::~GuiSelectWorld()
{
	delete worldSlotContainer;
	for (SaveFormatComparator *entry : saveList)
		delete entry;
}

void GuiSelectWorld::initGui()
{
	StringTranslate *tr = StringTranslate::getInstance();
	screenTitle  = tr->translateKey("selectWorld.title");
	worldLabel       = tr->translateKey("selectWorld.world");
	conversionLabel  = tr->translateKey("selectWorld.conversion");
	gameModeLabels[0] = tr->translateKey("gameMode.survival");
	gameModeLabels[1] = tr->translateKey("gameMode.creative");
	loadSaves();
	delete worldSlotContainer;
	worldSlotContainer = new GuiWorldSlot(this);
	worldSlotContainer->registerScrollButtons(controlList, 4, 5);
	initButtons();
}

void GuiSelectWorld::loadSaves()
{
	for (SaveFormatComparator *entry : saveList)
		delete entry;
	saveList.clear();
	ISaveFormat *fmt = mc->getSaveLoader();
	saveList = fmt->getSaveList();
	std::stable_sort(saveList.begin(), saveList.end(),
		[](const SaveFormatComparator *a, const SaveFormatComparator *b) { return *a < *b; });
	selectedWorld = -1;
}

std::string GuiSelectWorld::getSaveFileName(int_t i)
{
	return i >= 0 && (std::size_t)i < saveList.size() && saveList[(std::size_t)i] != nullptr
		? saveList[(std::size_t)i]->getFileName() : std::string();
}

std::string GuiSelectWorld::getSaveName(int_t i)
{
	if (i < 0 || (std::size_t)i >= saveList.size() || saveList[(std::size_t)i] == nullptr)
		return std::string();
	std::string s = saveList[(std::size_t)i]->getDisplayName();
	if (s.empty())
	{
		StringTranslate *tr = StringTranslate::getInstance();
		s = tr->translateKey("selectWorld.world") + " " + std::to_string(i + 1);
	}
	return s;
}

void GuiSelectWorld::initButtons()
{
	StringTranslate *tr = StringTranslate::getInstance();
	controlList.push_back(buttonSelect = new GuiButton(1, width / 2 - 154, height - 52, 150, 20, tr->translateKey("selectWorld.select")));
	controlList.push_back(buttonRename = new GuiButton(6, width / 2 - 154, height - 28,  70, 20, tr->translateKey("selectWorld.rename")));
	controlList.push_back(buttonDelete = new GuiButton(2, width / 2 -  74, height - 28,  70, 20, tr->translateKey("selectWorld.delete")));
	controlList.push_back(new GuiButton(3, width / 2 + 4, height - 52, 150, 20, tr->translateKey("selectWorld.create")));
	controlList.push_back(new GuiButton(0, width / 2 + 4, height - 28, 150, 20, tr->translateKey("gui.cancel")));
	buttonSelect->enabled = false;
	buttonRename->enabled = false;
	buttonDelete->enabled = false;
}

void GuiSelectWorld::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;

	if (button->id == 2)
	{
		std::string name = getSaveName(selectedWorld);
		if (!name.empty())
		{
			deleting = true;
			StringTranslate *tr = StringTranslate::getInstance();
			std::string q   = tr->translateKey("selectWorld.deleteQuestion");
			std::string w   = "'" + name + "' " + tr->translateKey("selectWorld.deleteWarning");
			std::string yes = tr->translateKey("selectWorld.deleteButton");
			std::string no  = tr->translateKey("gui.cancel");
			mc->displayGuiScreen(new GuiYesNo(this, q, w, yes, no, selectedWorld));
		}
	}
	else if (button->id == 1) selectWorld(selectedWorld);
	else if (button->id == 3) mc->displayGuiScreen(new GuiCreateWorld(this));
	else if (button->id == 6) mc->displayGuiScreen(new GuiRenameWorld(this, getSaveFileName(selectedWorld)));
	else if (button->id == 0) mc->displayGuiScreen(parentScreen);
	else worldSlotContainer->actionPerformed(button);
}

void GuiSelectWorld::selectWorld(int_t i)
{
	mc->displayGuiScreen(nullptr);
	if (selected) return;
	selected = true;
	delete mc->playerController;
	const int_t gameType = saveList[i]->getGameType();
	if (gameType == 0)
		mc->playerController = new PlayerControllerSP(mc);
	else
		mc->playerController = new PlayerControllerCreative(mc);
	std::string fname = getSaveFileName(i);
	if (fname.empty()) fname = "World" + std::to_string(i);
	mc->startWorld(fname, getSaveName(i), static_cast<WorldSettings *>(nullptr));
	mc->displayGuiScreen(nullptr);
}

void GuiSelectWorld::deleteWorld(bool confirmed, int_t i)
{
	if (deleting)
	{
		deleting = false;
		if (confirmed)
		{
			ISaveFormat *fmt = mc->getSaveLoader();
			fmt->flushCache();
			fmt->deleteWorldDirectory(getSaveFileName(i));
			loadSaves();
		}
		mc->displayGuiScreen(this);
	}
}

std::string GuiSelectWorld::formatDate(long_t timestamp) const
{
	time_t t = (time_t)(timestamp / 1000LL);
	char buf[64];
	struct tm localTime{};
#ifdef _WIN32
	localtime_s(&localTime, &t);
#else
	localtime_r(&t, &localTime);
#endif
	strftime(buf, sizeof(buf), "%x %X", &localTime);
	return std::string(buf);
}

void GuiSelectWorld::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	worldSlotContainer->drawScreen(mouseX, mouseY, partialTick);
	drawCenteredString(this->fontRenderer, screenTitle, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
