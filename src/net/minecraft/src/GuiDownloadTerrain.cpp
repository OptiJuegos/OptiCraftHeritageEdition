#include "GuiDownloadTerrain.h"
#include "NetClientHandler.h"
#include "Packet0KeepAlive.h"
#include "StringTranslate.h"
#include "GuiButton.h"
#include "FontRenderer.h"

GuiDownloadTerrain::GuiDownloadTerrain(NetClientHandler *netclienthandler)
	: updateCounter(0)
	, netHandler(netclienthandler)
{
}

void GuiDownloadTerrain::keyTyped(char_t c, int_t i)
{
}

void GuiDownloadTerrain::initGui()
{
	controlList.clear();
}

void GuiDownloadTerrain::updateScreen()
{
	updateCounter++;
	if (updateCounter % 20 == 0)
	{
		netHandler->addToSendQueue(new Packet0KeepAlive());
	}
	if (netHandler != nullptr)
	{
		netHandler->processReadPackets();
	}
}

void GuiDownloadTerrain::actionPerformed(GuiButton *guibutton)
{
}

void GuiDownloadTerrain::drawScreen(int_t i, int_t j, float_t f)
{
	drawBackground(0);
	StringTranslate *stringtranslate = StringTranslate::getInstance();
	drawCenteredString(fontRenderer, stringtranslate->translateKey("multiplayer.downloadingTerrain"), width / 2, height / 2 - 50, 0xffffff);
	GuiScreen::drawScreen(i, j, f);
}
