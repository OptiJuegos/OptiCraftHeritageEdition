#pragma once
#ifdef PS2_PLATFORM

namespace Ps2Pointer {
void enterMenu();
void leaveMenu();
float beginMenuFrame();
void move(float dx, float dy);
void publish();
void setPosition(int x, int y);
int x();
int y();
}

#endif
