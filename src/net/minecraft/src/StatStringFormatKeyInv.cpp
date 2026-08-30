#include "StatStringFormatKeyInv.h"
#include "Minecraft.h"
#include "GameSettings.h"
#include "KeyBinding.h"
#include "lwjgl/Keyboard.h"
#include "StringTranslate.h"

StatStringFormatKeyInv::StatStringFormatKeyInv(Minecraft *minecraft) :
	mc(minecraft)
{
}

std::string StatStringFormatKeyInv::formatString(const std::string &s)
{
	std::string keyName = lwjgl::Keyboard::getKeyName(mc->gameSettings->keyBindInventory->keyCode);
	std::string result = s;
	std::string token = "%1$s";
	std::size_t pos = result.find(token);
	if (pos != std::string::npos)
		result.replace(pos, token.length(), keyName);
	return result;
}
