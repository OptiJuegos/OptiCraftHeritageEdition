#include "KeyBinding.h"

#include <algorithm>

std::vector<KeyBinding *> KeyBinding::keybindArray;
IntHashMap KeyBinding::hash;

KeyBinding::KeyBinding(const std::string &s, int_t i)
	: keyDescription(s)
	, keyCode(i)
	, pressed(false)
	, pressTime(0)
{
	keybindArray.push_back(this);
	hash.addKey(keyCode, this);
}

KeyBinding::~KeyBinding()
{
	auto it = std::find(keybindArray.begin(), keybindArray.end(), this);
	if (it != keybindArray.end())
		keybindArray.erase(it);
	resetKeyBindingArrayAndHash();
}

void KeyBinding::onTick(int_t keyCode)
{
	KeyBinding *binding = static_cast<KeyBinding *>(hash.lookup(keyCode));
	if (binding != nullptr)
		++binding->pressTime;
}

void KeyBinding::setKeyBindState(int_t keyCode, bool state)
{
	KeyBinding *binding = static_cast<KeyBinding *>(hash.lookup(keyCode));
	if (binding != nullptr)
		binding->pressed = state;
}

void KeyBinding::unPressAllKeys()
{
	for (KeyBinding *binding : keybindArray)
	{
		if (binding != nullptr)
			binding->unpressKey();
	}
}

void KeyBinding::resetKeyBindingArrayAndHash()
{
	hash.clearMap();
	for (KeyBinding *binding : keybindArray)
	{
		if (binding != nullptr)
			hash.addKey(binding->keyCode, binding);
	}
}

bool KeyBinding::isPressed()
{
	if (pressTime == 0)
		return false;
	--pressTime;
	return true;
}

void KeyBinding::unpressKey()
{
	pressTime = 0;
	pressed = false;
}
