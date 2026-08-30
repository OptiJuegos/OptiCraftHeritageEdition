#pragma once
#ifdef WII_PLATFORM

// Per-controller-family raw button assignments for the discrete actions that
// make sense to rebind on Wii (Jump/Sneak/Drop/Inventory/Attack/Use/
// ThirdPerson -- movement is analog-stick-driven on every family except a
// bare Wiimote, so it stays out of scope here). GameCube pad, the Wiimote's
// own buttons (plus Nunchuk C/Z when attached), and a Classic Controller each
// get an independent slot: pressing A on any one of them already triggers the
// same action today (see WiiGameCubePad.cpp / WiiRemote.cpp), and rebinding
// one family must not disturb the others.
//
// GameSettings owns the persisted values and pushes a snapshot here
// (mirroring PlatformKeyBindings for the PS2/keyboard-shaped side); the two
// pad pollers read it every frame instead of a hardcoded button constant.
namespace WiiButtonBindings
{

struct FamilySnapshot
{
	unsigned jump;
	unsigned sneak;
	unsigned drop;
	unsigned inventory;
	unsigned attack;      // break blocks
	unsigned use;         // place blocks / interact
	unsigned thirdPerson; // 0 where unbound by default (Wiimote, Classic)
};

struct Snapshot
{
	FamilySnapshot gameCube;
	FamilySnapshot wiimote;
	FamilySnapshot classic;
};

void set(const Snapshot &snapshot);
const Snapshot &get();

}

#endif // WII_PLATFORM
