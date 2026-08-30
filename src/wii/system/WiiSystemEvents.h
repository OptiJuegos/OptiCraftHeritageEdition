#pragma once
#ifdef WII_PLATFORM
namespace WiiSystemEvents
{
void install();
bool exitRequested();
void clearExitRequest();
}
#endif
