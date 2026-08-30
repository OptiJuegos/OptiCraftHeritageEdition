#pragma once

#include "java/Type.h"

// net.minecraft.src.MouseHelper
class MouseHelper
{
public:
	MouseHelper(void *component);

	void grabMouseCursor();
	void ungrabMouseCursor();
	void mouseXYChange();

public:
	int_t deltaX;
	int_t deltaY;

private:
	void *field_1117_c;
	void *cursor;
	int_t field_1115_e;
};
