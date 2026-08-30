#include "GuiSleepMP.h"
#include "java/String.h"
#include "GuiButton.h"
#include "StringTranslate.h"
#include "EntityClientPlayerMP.h"
#include "NetClientHandler.h"
#include "Packet19EntityAction.h"
#include "Minecraft.h"
#include "pc/lwjgl/Keyboard.h"

GuiSleepMP::GuiSleepMP()
{
}

void GuiSleepMP::initGui()
{
	GuiChat::initGui();
	StringTranslate *tr = StringTranslate::getInstance();
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	controlList.push_back(new GuiButton(1, width / 2 - 100, height - 154, tr->translateKey("multiplayer.stopSleeping")));
#else
	controlList.push_back(new GuiButton(1, width / 2 - 100, height - 40, tr->translateKey("multiplayer.stopSleeping")));
#endif
}

void GuiSleepMP::onGuiClosed()
{
	GuiChat::onGuiClosed();
}

void GuiSleepMP::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		stopSleeping();
	}
	else if (key == 28) // Enter
	{
		std::string s = String::trimJava(getMessage());
		if (!s.empty())
			mc->thePlayer->sendChatMessage(s);
		setMessage("");
	}
	else
	{
		GuiChat::keyTyped(c, key);
	}
}

void GuiSleepMP::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	GuiChat::drawScreen(mouseX, mouseY, partialTick);
}

void GuiSleepMP::actionPerformed(GuiButton *button)
{
	if (button->id == 1)
	{
		stopSleeping();
	}
	else
	{
		GuiChat::actionPerformed(button);
	}
}

void GuiSleepMP::stopSleeping()
{
	EntityClientPlayerMP *mp = dynamic_cast<EntityClientPlayerMP *>(mc->thePlayer);
	if (mp != nullptr)
	{
		mp->sendQueue->addToSendQueue(new Packet19EntityAction(mc->thePlayer, 3));
	}
}
