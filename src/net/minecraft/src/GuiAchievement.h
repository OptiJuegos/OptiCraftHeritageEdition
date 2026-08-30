#pragma once

#include "Gui.h"
#include <string>

class Minecraft;
class Achievement;
class RenderItem;

// net.minecraft.src.GuiAchievement
class GuiAchievement : public Gui
{
public:
	GuiAchievement(Minecraft *minecraft);
	~GuiAchievement();

	void queueTakenAchievement(Achievement *achievement);
	void queueAchievementInformation(Achievement *achievement);
	void updateAchievementWindow();

private:
	void updateAchievementWindowScale();

	Minecraft *theGame;
	int_t achievementWindowWidth;
	int_t achievementWindowHeight;
	std::string field_25085_d;
	std::string field_25084_e;
	Achievement *theAchievement;
	long_t field_25083_f;
	RenderItem *itemRender;
	bool field_27103_i;
};
