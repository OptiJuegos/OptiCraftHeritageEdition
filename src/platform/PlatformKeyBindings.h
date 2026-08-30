#pragma once

// Shared key-binding snapshot so console input mappers (PS2's gamepad-to-key
// synthesis, Wii's pad-to-key virtual key table) drive whatever key the
// player actually bound in GameSettings, instead of hardcoding the vanilla
// defaults. GameSettings pushes a new snapshot whenever bindings load or a
// rebind happens; the console pollers read it once per frame.
namespace PlatformKeyBindings
{

struct Snapshot
{
	int forward;
	int back;
	int left;
	int right;
	int jump;
	int sneak;
	int drop;
	int inventory;
};

void set(const Snapshot &snapshot);
const Snapshot &get();

// Bumped on every set() call so a poller can tell a rebind just happened and
// release any synthetic key still latched under the outgoing mapping before
// adopting the new one.
unsigned version();

}
