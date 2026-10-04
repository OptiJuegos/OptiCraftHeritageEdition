#pragma once

// DsSwkbd.h -- the system software keyboard (the swkbd applet) as the 3DS's
// primary text-entry path. Only 3DS code includes this; VirtualKeyboard.cpp
// reaches it through its CTR_PLATFORM branch and falls back to the on-screen
// keyboard panel on the bottom screen when it reports Unavailable (see
// VirtualKeyboard::tick()).
//
// The applet only needs APT (swkbd.o references no other service), so
// resources/3ds_cia.rsf grants nothing new: the APT:U it already has covers
// aptLaunchLibraryApplet().
//
// The dialog is ASYNCHRONOUS. The 3DS does not suspend the application while
// a library applet runs -- the official Programming Manual puts the app into
// an "Inactive" state where "threads other than those used by the
// applications to call functions to display them or wait for them to complete
// can continue to run" -- so swkbdInputText() only blocks the ONE thread that
// calls it. That used to be the game thread, which froze the whole client
// while the player typed: no ticks ran, keepalives went unanswered, and the
// server timed the session out ("the keyboard opens and the server kicks
// me"). The dialog now runs on a helper thread (32 KiB stack, the same
// PlatformThread shape the network backend uses) and the game keeps ticking
// through it: rendering, aptMainLoop() and HID polling stand down while
// dsSwkbdActive() (the applet owns both LCDs and the APT session), while the
// network read/write/dispatch threads and the world tick continue -- the
// "don't pause the game while typing" behaviour players asked for.
//
// One dialog at a time: dsSwkbdOpenAsync() refuses to launch while a result
// from a previous dialog is still unconsumed (join it, discard, then retry).

#include <string>

// What the player did when the applet disappeared.
enum class DsSwkbdOutcome
{
	Confirmed,   // the right button ("OK") -- result.text holds the input
	Cancelled,   // the left button ("Cancel"), or HOME/RESET/POWER
	Unavailable, // the applet never produced a dialog (bad launch, no memory);
	             // the caller falls back to the bottom-screen panel, and
	             // stops trying for the rest of the session
};

struct DsSwkbdResult
{
	DsSwkbdOutcome outcome = DsSwkbdOutcome::Unavailable;
	std::string text; // UTF-8; set only when outcome == Confirmed
};

// Ask the system keyboard to open with `initial` pre-filled. Returns
// immediately: the dialog runs on a helper thread. maxLength is the field's
// maxStringLength in UTF-16 code units, the unit swkbd counts its limit in
// (the game's jstring is UTF-8; the result buffer sizes for that).
// Returns false when a previous dialog's result has not been consumed yet or
// the helper thread could not be started -- the caller falls back to the
// bottom-screen panel.
bool dsSwkbdOpenAsync(const std::string& initial, int maxLength);

// True from a successful dsSwkbdOpenAsync() until the applet is closed and
// its result is stored. While true, the caller must not touch APT
// (aptMainLoop()) nor the HID state: the applet owns the foreground, the
// screens and the APT session.
bool dsSwkbdActive();

// Consume the finished dialog's outcome. Returns false while the applet is
// still up or no dialog ever ran; true hands the result out exactly once
// (and joins the finished helper thread before returning).
bool dsSwkbdTakeResult(DsSwkbdResult& outResult);
