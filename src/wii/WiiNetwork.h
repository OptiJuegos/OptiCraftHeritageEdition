#pragma once

#include <string>

namespace WiiNetwork
{

// Starts the Wii network stack using the connection configured in the System
// Menu. Safe to call more than once; failed attempts may be retried.
bool initialize();
bool isReady();
const std::string &localAddress();
void shutdown();

}
