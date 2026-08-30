#ifdef WII_PLATFORM

#include "wii/input/WiiPointer.h"

#include <cmath>

#include <ogc/lwp_watchdog.h>

#include "lwjgl/Mouse.h"
#include "wii/gx_wii.h"

namespace
{
constexpr float kDegreesPerMousePixel = 0.15f;
constexpr float kStickLookDegPerSec = 160.0f;
constexpr float kIrLookDegPerSec = 180.0f;
constexpr float kCursorPixelsPerSec = 750.0f;

constexpr float kIrDeadzone = 0.16f;
constexpr float kIrSaturation = 0.85f;
constexpr float kIrCurveLinear = 0.35f;
constexpr bool kIrRateCamera = true;

constexpr float kMinLookDt = 1.0f / 240.0f;
constexpr float kMaxLookDt = 1.0f / 15.0f;

float g_lookX = 0.0f;
float g_lookY = 0.0f;
float g_irLookX = 0.0f;
float g_irLookY = 0.0f;
float g_lookDt = 1.0f / 60.0f;
u64 g_lookLastTime = 0;
float g_residualX = 0.0f;
float g_residualY = 0.0f;

bool g_haveIrPrevious = false;
int g_irPreviousX = 0;
int g_irPreviousY = 0;
int g_cursorX = 0;
int g_cursorY = 0;

int irMagnitudePercent(int x, int y, int width, int height)
{
	if (height <= 0)
		return 0;
	const float half = (float)height * 0.5f;
	const float ux = ((float)x - (float)width * 0.5f) / half;
	const float uy = ((float)y - (float)height * 0.5f) / half;
	return (int)(std::sqrt(ux * ux + uy * uy) * 100.0f);
}

void addIrRateLook(int x, int y, int width, int height)
{
	if (height <= 0)
		return;

	// Normalize both axes by half the screen height so diagonal pointing keeps
	// the same direction after conversion into camera rate.
	const float half = (float)height * 0.5f;
	const float ux = ((float)x - (float)width * 0.5f) / half;
	const float uy = ((float)y - (float)height * 0.5f) / half;
	const float magnitude = std::sqrt(ux * ux + uy * uy);
	if (magnitude <= kIrDeadzone)
		return;

	float t = (magnitude - kIrDeadzone) / (kIrSaturation - kIrDeadzone);
	if (t > 1.0f)
		t = 1.0f;
	const float curve = t * (kIrCurveLinear + (1.0f - kIrCurveLinear) * t);
	const float pixels = (kIrLookDegPerSec / kDegreesPerMousePixel) * curve * g_lookDt;
	g_irLookX += (ux / magnitude) * pixels;
	g_irLookY += (uy / magnitude) * pixels;
}
}

namespace WiiPointer
{

void beginFrame()
{
	const u64 now = gettime();
	if (g_lookLastTime != 0)
	{
		const float dt = (float)diff_usec(g_lookLastTime, now) * 1e-6f;
		g_lookDt = (dt < kMinLookDt) ? kMinLookDt : ((dt > kMaxLookDt) ? kMaxLookDt : dt);
	}
	g_lookLastTime = now;
	g_lookX = 0.0f;
	g_lookY = 0.0f;
	g_irLookX = 0.0f;
	g_irLookY = 0.0f;
}

void addStickLook(float x, float y)
{
	g_lookX += x;
	g_lookY += y;
}

int handleIr(int x, int y, bool allowGameplayLook)
{
	const int width = wiigl_width();
	const int height = wiigl_height();
	const int magnitude = irMagnitudePercent(x, y, width, height);

	if (kIrRateCamera && lwjgl::Mouse::isGrabbed())
	{
		if (allowGameplayLook)
			addIrRateLook(x, y, width, height);
		g_haveIrPrevious = false;
		return magnitude;
	}

	if (g_haveIrPrevious)
	{
		const int dx = x - g_irPreviousX;
		const int dy = y - g_irPreviousY;
		if (dx != 0 || dy != 0)
		{
			g_cursorX = x;
			g_cursorY = y;
			lwjgl::Mouse::detail::pushMotion(g_cursorX, g_cursorY, dx, dy);
		}
	}
	g_irPreviousX = x;
	g_irPreviousY = y;
	g_haveIrPrevious = true;
	return magnitude;
}

void loseIr()
{
	g_haveIrPrevious = false;
}

void flush()
{
	const bool grabbed = lwjgl::Mouse::isGrabbed();

	float x = g_lookX;
	float y = g_lookY;
	if (x > 1.0f) x = 1.0f; else if (x < -1.0f) x = -1.0f;
	if (y > 1.0f) y = 1.0f; else if (y < -1.0f) y = -1.0f;

	const float stickScale = grabbed
		? (kStickLookDegPerSec / kDegreesPerMousePixel) * g_lookDt
		: kCursorPixelsPerSec * g_lookDt;
	const float px = x * stickScale + g_irLookX;
	const float py = y * stickScale + g_irLookY;

	if (px == 0.0f && py == 0.0f)
	{
		g_residualX = 0.0f;
		g_residualY = 0.0f;
		return;
	}

	g_residualX += px;
	g_residualY += py;
	const int ix = (int)g_residualX;
	const int iy = (int)g_residualY;
	if (ix == 0 && iy == 0)
		return;
	g_residualX -= (float)ix;
	g_residualY -= (float)iy;

	if (!grabbed)
	{
		const int width = wiigl_width();
		const int height = wiigl_height();
		g_cursorX += ix;
		g_cursorY += iy;
		if (g_cursorX < 0) g_cursorX = 0; else if (g_cursorX >= width) g_cursorX = width - 1;
		if (g_cursorY < 0) g_cursorY = 0; else if (g_cursorY >= height) g_cursorY = height - 1;
	}

	lwjgl::Mouse::detail::pushMotion(g_cursorX, g_cursorY, ix, iy);
}

void setCursorPosition(int x, int y)
{
	const int width = wiigl_width();
	const int height = wiigl_height();
	if (x < 0) x = 0; else if (x >= width) x = width - 1;
	if (y < 0) y = 0; else if (y >= height) y = height - 1;
	g_cursorX = x;
	g_cursorY = y;
	lwjgl::Mouse::detail::pushMotion(g_cursorX, g_cursorY, 0, 0);
}

int cursorX()
{
	return g_cursorX;
}

int cursorY()
{
	return g_cursorY;
}

}

#endif
