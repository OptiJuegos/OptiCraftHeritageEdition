#ifdef PS2_PLATFORM

#include "ps2/input/Ps2Pointer.h"
#include "ps2/render/Ps2Graphics.h"
#include "platform/time.h"
#include "lwjgl/Mouse.h"

namespace {
float s_x = 320.0f;
float s_y = 240.0f;
float s_lastTime = 0.0f;

void clamp() {
    const float maxX = (float)(Ps2Graphics::width() - 1);
    const float maxY = (float)(Ps2Graphics::height() - 1);
    if (s_x < 0.0f) s_x = 0.0f;
    if (s_y < 0.0f) s_y = 0.0f;
    if (s_x > maxX) s_x = maxX;
    if (s_y > maxY) s_y = maxY;
}
}

namespace Ps2Pointer {
void enterMenu() {
    s_x = (float)Ps2Graphics::width() * 0.5f;
    s_y = (float)Ps2Graphics::height() * 0.5f;
    s_lastTime = getTimeS();
}
void leaveMenu() { s_lastTime = 0.0f; }
float beginMenuFrame() {
    const float now = getTimeS();
    float dt = (s_lastTime > 0.0f) ? now - s_lastTime : 0.0f;
    s_lastTime = now;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.10f) dt = 0.10f;
    return dt;
}
void move(float dx, float dy) { s_x += dx; s_y += dy; clamp(); }
void publish() { lwjgl::Mouse::detail::pushMotion((int)s_x, (int)s_y, 0, 0); }
void setPosition(int x, int y) { s_x = (float)x; s_y = (float)y; clamp(); publish(); }
int x() { return (int)s_x; }
int y() { return (int)s_y; }
}

#endif
