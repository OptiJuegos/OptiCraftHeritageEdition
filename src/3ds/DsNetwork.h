#pragma once

#include <string>

namespace DsNetwork
{

// Starts the 3DS soc:U socket service over the console's configured network
// (the same settings the HOME menu uses). Safe to call more than once; failed
// attempts may be retried.
bool initialize();
bool isReady();
const std::string &localAddress();
void shutdown();

// Pre-flight the wireless radio before a server connect is attempted.
// Returns an empty string when the radio is on (or unknowable -- a loader
// that will not answer ac:u never blocks the attempt), and a
// player-readable, actionable error when the console's Wi-Fi is off, which
// is what "servers never connect" on a New 3DS almost always is: the New
// models replaced the Old 3DS's physical wireless switch with a software
// toggle in the HOME menu, so the radio stays off after a reboot or a
// settings visit, and until now the game only ever answered with the generic
// "Connection refused". On kernel 2.55+ (system 11.4+) it first tries to
// switch the radio on itself (svcSetWifiEnabled): the player is explicitly
// asking to join a server, so enabling the radio matches their intent.
std::string wifiPreflightError();

}
