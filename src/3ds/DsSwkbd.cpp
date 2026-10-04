// DsSwkbd.cpp -- launches the system software keyboard for one text field,
// on a helper thread so the game loop keeps running while the applet is up.
#ifdef CTR_PLATFORM

#include "3ds/DsSwkbd.h"

#include <3ds.h>

#include <cstddef>
#include <mutex>
#include <utility>
#include <vector>

#include "platform/Log.h"
#include "platform/Mutex.h"
#include "platform/Thread.h"

namespace
{
// swkbd counts its limit in UTF-16 code units while the result it writes is
// UTF-8: one BMP unit is at most 3 bytes and a surrogate pair (2 units) is 4,
// so 4 bytes per unit covers anything the applet can hand back. The slack is
// for the terminator even if the applet filled the buffer to the brim -- the
// text is read with strlen semantics, so a missing NUL would read past the
// end.
constexpr std::size_t kBytesPerCodeUnit = 4;
constexpr std::size_t kTerminatorSlack = 8;

// SwkbdState::max_text_len is a u16.
constexpr int kMaxTextLength = 0xFFFF;

// The helper runs swkbdInputText() -> aptLaunchLibraryApplet(), which blocks
// the calling thread until the dialog closes. 32 KiB is the same budget the
// network reader/writer workers get, and several times what the dialog's
// SwkbdState (~0.6 KiB) plus the APT call chain needs; priority 64 keeps the
// almost-always-blocked helper from ever preempting the game thread.
constexpr std::size_t kHelperStackBytes = 32 * 1024;
constexpr int kHelperPriority = 64;

struct SwkbdRequest
{
	std::string initial;
	int maxLength = 1;
};

struct SwkbdSession
{
	PlatformMutex mutex;
	bool active = false;     // applet launched and not yet finished
	bool resultReady = false;
	DsSwkbdResult result;
	PlatformThread thread;
};

SwkbdSession g_session;

// The old synchronous dialog, verbatim, on whatever thread calls it.
DsSwkbdResult runSwkbdDialog(const std::string& initial, int maxLength)
{
	DsSwkbdResult result;

	if (maxLength < 1)
		maxLength = 1;
	if (maxLength > kMaxTextLength)
		maxLength = kMaxTextLength;

	SwkbdState swkbd;
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, maxLength);
	// Accept everything: the field enforces its own rules anyway
	// (ChatAllowedCharacters plus the maxStringLength clamp, applied on the
	// result), and rejecting input inside the applet would only stop the
	// player from editing a value the game is going to trim itself.
	swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
	swkbdSetFeatures(&swkbd,
	                 SWKBD_DEFAULT_QWERTY | SWKBD_DARKEN_TOP_SCREEN | SWKBD_ALLOW_HOME);
	// Cancel on the left (its own button label, discarding the text), OK on
	// the right (submitting it) -- the dialog layout the player expects.
	swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
	swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "OK", true);
	if (!initial.empty())
		swkbdSetInitialText(&swkbd, initial.c_str());

	std::vector<char> buffer(
		static_cast<std::size_t>(maxLength) * kBytesPerCodeUnit + kTerminatorSlack, '\0');
	swkbdInputText(&swkbd, buffer.data(), buffer.size());

	// The button identifier comes back through swkbdGetResult as the
	// dialog's own result codes (a two-button dialog reports D1_CLICK0 for
	// the left button, D1_CLICK1 for the right one).
	switch (swkbdGetResult(&swkbd))
	{
	case SWKBD_D1_CLICK1:
		result.outcome = DsSwkbdOutcome::Confirmed;
		result.text.assign(buffer.data());
		return result;
	case SWKBD_D1_CLICK0:
		result.outcome = DsSwkbdOutcome::Cancelled;
		return result;
	case SWKBD_HOMEPRESSED:
	case SWKBD_RESETPRESSED:
	case SWKBD_POWERPRESSED:
		result.outcome = DsSwkbdOutcome::Cancelled;
		return result;
	default:
		// SWKBD_NONE / SWKBD_INVALID_INPUT / SWKBD_OUTOFMEM: no dialog. One
		// line here, then the caller pins the session to the fallback panel
		// instead of retrying the launch for every field it meets.
		MC_LOG_WARN("input",
			"3ds: software keyboard unavailable (swkbd result %d)\n",
			static_cast<int>(swkbdGetResult(&swkbd)));
		result.outcome = DsSwkbdOutcome::Unavailable;
		return result;
	}
}

void* swkbdHelperMain(void* argument)
{
	SwkbdRequest request = *static_cast<SwkbdRequest*>(argument);
	delete static_cast<SwkbdRequest*>(argument);

	DsSwkbdResult result = runSwkbdDialog(request.initial, request.maxLength);

	// The whole hand-over is one lock; once active goes false the result is
	// guaranteed stored, and nothing below locks again -- dsSwkbdTakeResult()
	// joins only after this point, so the join can never deadlock against it.
	{
		std::lock_guard<PlatformMutex> guard(g_session.mutex);
		g_session.result = std::move(result);
		g_session.resultReady = true;
		g_session.active = false;
	}
	return nullptr;
}
} // namespace

bool dsSwkbdOpenAsync(const std::string& initial, int maxLength)
{
	std::lock_guard<PlatformMutex> guard(g_session.mutex);
	if (g_session.active)
		return false; // a dialog is up; its result must be consumed first

	// A finished-but-unconsumed helper: join it now, drop its result, and
	// let this call start a fresh dialog. The thread is already past its
	// last lock (resultReady implies that), so joining here is safe.
	if (g_session.thread.joinable())
	{
		g_session.thread.join();
		g_session.resultReady = false;
		g_session.result = DsSwkbdResult();
	}

	auto* request = new SwkbdRequest{initial, maxLength};
	if (!g_session.thread.start(&swkbdHelperMain, request, kHelperStackBytes,
	                            kHelperPriority, 0))
	{
		delete request;
		MC_LOG_WARN("input", "3ds: swkbd helper thread could not start\n");
		return false;
	}

	g_session.resultReady = false;
	g_session.result = DsSwkbdResult();
	g_session.active = true;
	return true;
}

bool dsSwkbdActive()
{
	std::lock_guard<PlatformMutex> guard(g_session.mutex);
	return g_session.active;
}

bool dsSwkbdTakeResult(DsSwkbdResult& outResult)
{
	{
		std::lock_guard<PlatformMutex> guard(g_session.mutex);
		if (g_session.active || !g_session.resultReady)
			return false;
		outResult = std::move(g_session.result);
		g_session.result = DsSwkbdResult();
		g_session.resultReady = false;
	}
	// Outside the lock: the helper already stored its result, so it holds and
	// takes no more locks -- joining now only waits for its return.
	if (g_session.thread.joinable())
		g_session.thread.join();
	return true;
}

#endif // CTR_PLATFORM
