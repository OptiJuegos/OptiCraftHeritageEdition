#pragma once

#include <string>
#include <vector>

#include "IntHashMap.h"
#include "java/Type.h"

// net.minecraft.src.KeyBinding
class KeyBinding
{
public:
	KeyBinding(const std::string &s, int_t i);
	~KeyBinding();

	static void onTick(int_t keyCode);
	static void setKeyBindState(int_t keyCode, bool pressed);
	static void unPressAllKeys();
	static void resetKeyBindingArrayAndHash();

	bool isPressed();

	static std::vector<KeyBinding *> keybindArray;
	static IntHashMap hash;

	std::string keyDescription;
	int_t keyCode;
	bool pressed;
	int_t pressTime;

private:
	void unpressKey();
};
