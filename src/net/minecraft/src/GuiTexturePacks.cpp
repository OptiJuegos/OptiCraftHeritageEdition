#include "GuiTexturePacks.h"
#include "GuiTexturePackSlot.h"
#include "GuiSmallButton.h"
#include "GuiButton.h"
#include "StringTranslate.h"
#include "TexturePackList.h"
#include "RenderEngine.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "java/File.h"
#include "java/System.h"
#include "java/String.h"
#include <memory>

GuiTexturePacks::GuiTexturePacks(GuiScreen *guiscreen)
	: field_6454_o(-1)
	, fileLocation("")
	, guiScreen(guiscreen)
	, guiTexturePackSlot(nullptr)
{
}

GuiTexturePacks::~GuiTexturePacks()
{
	delete guiTexturePackSlot;
}

void GuiTexturePacks::initGui()
{
	StringTranslate *stringtranslate = StringTranslate::getInstance();
#if defined(CTR_PLATFORM)
	// The "Open Folder" button shells out to the host OS's file browser
	// (System::openURL("file://...")), which does not exist on the 3DS -- the
	// pack list plus the QR download is this console's whole install story.
	// Only "Done" remains, centered in the GuiSmallButton's 150 px width.
	controlList.push_back(new GuiSmallButton(6, width / 2 - 75, height - 48, stringtranslate->translateKey("gui.done")));
#else
	controlList.push_back(new GuiSmallButton(5, width / 2 - 154, height - 48, stringtranslate->translateKey("texturePack.openFolder")));
	controlList.push_back(new GuiSmallButton(6, width / 2 + 4,   height - 48, stringtranslate->translateKey("gui.done")));
#endif
	mc->texturePackList->updateAvailableTexturePacks();
	std::unique_ptr<File> texturePackDirectory(File::open(*Minecraft::getMinecraftDir(), "texturepacks"));
	fileLocation = texturePackDirectory->toString();
	delete guiTexturePackSlot;
	guiTexturePackSlot = new GuiTexturePackSlot(this);
	guiTexturePackSlot->registerScrollButtons(controlList, 7, 8);
}

void GuiTexturePacks::actionPerformed(GuiButton *guibutton)
{
	if (!guibutton->enabled) return;
	if (guibutton->id == 5)
	{
		System::openURL("file://" + fileLocation);
	}
	else if (guibutton->id == 6)
	{
		mc->refreshResources();
		mc->displayGuiScreen(guiScreen);
	}
	else
	{
		guiTexturePackSlot->actionPerformed(guibutton);
	}
}

void GuiTexturePacks::mouseClicked(int_t i, int_t j, int_t k)
{
	GuiScreen::mouseClicked(i, j, k);
}

void GuiTexturePacks::mouseMovedOrUp(int_t i, int_t j, int_t k)
{
	GuiScreen::mouseMovedOrUp(i, j, k);
}

void GuiTexturePacks::drawScreen(int_t i, int_t j, float_t f)
{
	guiTexturePackSlot->drawScreen(i, j, f);
	if (field_6454_o <= 0)
	{
		mc->texturePackList->updateAvailableTexturePacks();
		field_6454_o += 20;
	}
	StringTranslate *stringtranslate = StringTranslate::getInstance();
	drawCenteredString(fontRenderer, stringtranslate->translateKey("texturePack.title"),      width / 2,      16,            0xffffff);
	drawCenteredString(fontRenderer, stringtranslate->translateKey("texturePack.folderInfo"), width / 2 - 77, height - 26,   0x808080);
	GuiScreen::drawScreen(i, j, f);
}

void GuiTexturePacks::updateScreen()
{
	GuiScreen::updateScreen();
	field_6454_o--;
}
