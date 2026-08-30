#include "platform/ClientProfilerBackend.h"

namespace ClientProfilerBackend
{
void frameBegin() {}
void ticks(long long, int) {}
void lighting(long long) {}
void displayUpdate(long long) {}
void render(long long) {}
void frameEnd(long long, long long, long long, int, int, World*, RenderGlobal*) {}
}
