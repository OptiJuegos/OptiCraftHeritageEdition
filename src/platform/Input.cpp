#include "platform/Input.h"

namespace
{
bool g_textInputExclusive = false;
bool g_containerNavigationActive = false;
bool g_padRebindExclusive = false;
}

void platformSetTextInputExclusive(bool active)
{
    g_textInputExclusive = active;
}

bool platformTextInputExclusive()
{
    return g_textInputExclusive;
}

void platformSetPadRebindExclusive(bool active)
{
    g_padRebindExclusive = active;
}

bool platformPadRebindExclusive()
{
    return g_padRebindExclusive;
}

void platformSetContainerNavigationActive(bool active)
{
    g_containerNavigationActive = active;
}

bool platformContainerNavigationActive()
{
    return g_containerNavigationActive;
}
