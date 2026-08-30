#include "platform/Log.h"
#include "TexturePackList.h"

#include <algorithm>
#include <cctype>
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include <filesystem>
#endif
#include <iostream>
#include "TexturePackDefault.h"
#include "TexturePackCustom.h"
#include "Minecraft.h"
#include "GameSettings.h"
#include "java/String.h"

#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
namespace fs = std::filesystem;
#endif

TexturePackList::TexturePackList(Minecraft *minecraft, const std::string &file) :
	mc(minecraft),
	defaultTexturePack(new TexturePackDefault()),
	selectedTexturePack(nullptr)
{
	texturePackDir = file + "/texturepacks";
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
	if (!fs::exists(texturePackDir))
	{
		fs::create_directories(texturePackDir);
	}
#endif
	currentTexturePack = minecraft->gameSettings->skin;
	updateAvailableTexturePacks();
	selectedTexturePack->loadTexturePack();
}

TexturePackList::~TexturePackList()
{
	if (selectedTexturePack != nullptr)
		selectedTexturePack->closeTexturePackFile();
	for (auto &entry : soundPool)
		delete entry.second;
	soundPool.clear();
	availableTexturePacks.clear();
	delete defaultTexturePack;
	defaultTexturePack = nullptr;
	selectedTexturePack = nullptr;
}

bool TexturePackList::setTexturePack(TexturePackBase *texturepackbase)
{
	if (texturepackbase == selectedTexturePack)
	{
		return false;
	}
	else
	{
		selectedTexturePack->closeTexturePackFile();
		currentTexturePack = texturepackbase->texturePackFileName;
		selectedTexturePack = texturepackbase;
		mc->gameSettings->skin = currentTexturePack;
		mc->gameSettings->saveOptions();
		selectedTexturePack->loadTexturePack();
		return true;
	}
}

void TexturePackList::updateAvailableTexturePacks()
{
	std::vector<TexturePackBase*> arraylist;
	selectedTexturePack = nullptr;
	arraylist.push_back(defaultTexturePack);

#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
	if (fs::exists(texturePackDir) && fs::is_directory(texturePackDir))
	{
		for (const auto &entry : fs::directory_iterator(texturePackDir))
		{
			if (!entry.is_regular_file()) continue;
			std::string filename = entry.path().filename().string();
			const jstring lowerName = String::toLowerCaseJava(jstring(filename));
			if (lowerName.size() < 4 || lowerName.compare(lowerName.size() - 4, 4, ".zip") != 0) continue;

			auto file = entry.path();
			std::string s = filename + ":" + std::to_string(fs::file_size(file)) + ":" + std::to_string(fs::last_write_time(file).time_since_epoch().count());

			try
			{
				auto it = soundPool.find(s);
				if (it == soundPool.end())
				{
					TexturePackCustom *texturepackcustom = new TexturePackCustom(file.string());
					texturepackcustom->texturePackFolder = s;
					soundPool[s] = texturepackcustom;
					texturepackcustom->getTexturePackFolder(mc);
				}
				TexturePackBase *texturepackbase1 = soundPool[s];
				if (texturepackbase1->texturePackFileName == currentTexturePack)
				{
					selectedTexturePack = texturepackbase1;
				}
				arraylist.push_back(texturepackbase1);
			}
			catch (std::exception &ioexception)
			{
				MC_LOG_ERROR("game", "%s\n", ioexception.what());
			}
		}
	}
#endif

	if (selectedTexturePack == nullptr)
	{
		selectedTexturePack = defaultTexturePack;
	}

	for (auto *tp : availableTexturePacks)
	{
		bool found = false;
		for (auto *tp2 : arraylist)
		{
			if (tp == tp2) { found = true; break; }
		}
		if (!found)
		{
			soundPool.erase(tp->texturePackFolder);
			delete tp;
		}
	}

	availableTexturePacks = arraylist;
}

const std::vector<TexturePackBase*> &TexturePackList::getAvailableTexturePacks() const
{
	return availableTexturePacks;
}

TexturePackBase *TexturePackList::getSelectedTexturePack() const
{
	return selectedTexturePack;
}
