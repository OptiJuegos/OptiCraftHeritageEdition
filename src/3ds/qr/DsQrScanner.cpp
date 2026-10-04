// DsQrScanner.cpp -- camera capture + quirc decode, on the game thread.
#ifdef CTR_PLATFORM

#include "3ds/qr/DsQrScanner.h"

#include <3ds.h>

#include <cstring>

#include "platform/Log.h"
#include "quirc.h"

namespace
{
// QVGA, the camera's native cheap mode and more resolution than a QR code
// needs: quirc comfortably reads a version-3 code (29x29 cells) filling a
// third of this frame, which is what a phone or monitor shows comfortably.
constexpr int kCamWidth = 320;
constexpr int kCamHeight = 240;

// The GUI preview is a half-scale luminance image: 160x120 RGBA8, built in
// the same pass that extracts the gray plane. 76 KB, static, owned by the
// scanner because the GUI only reads it between scan() calls.
constexpr int kPreviewWidth = kCamWidth / 2;
constexpr int kPreviewHeight = kCamHeight / 2;

// One frame at 15fps lands every ~67 ms; the wait covers a missed frame plus
// the service's scheduling slack without freezing the menu on a stuck camera.
constexpr s64 kCaptureTimeoutNs = 150 * 1000 * 1000LL;

bool g_started = false;
// Sticky "at least one camera frame arrived": what separates "camera delivering
// but no QR yet" from the silent black preview of an unsupported emulation.
bool g_frameArrived = false;
struct quirc *g_quirc = nullptr;

// The cam service DMAs into this buffer, so it must be linear-heap memory
// that stays put (CAMU_SetReceiving hands the physical address to the
// service). YUV422: 2 bytes per pixel, Y in every even byte.
std::uint8_t *g_capture = nullptr;
// The transfer unit CAMU_GetMaxBytes reports for this resolution. It must be
// handed to BOTH CAMU_SetTransferBytes and every CAMU_SetReceiving -- the
// camera examples do exactly that -- because a mismatched unit makes the
// receive never complete (the invisible-only-on-hardware variant of a camera
// that simply never reaches the preview).
s16 g_transferUnit = 0;

// Hardware latches a buffer error and stalls the port the moment a camera
// frame lands while no receive is armed. Emulated cameras deliver on demand
// and never trip it, which is why this only shows up on hardware. The fix,
// taken from FBI's proven camera task (source/core/task/capturecam.c):
// arm the receive BEFORE starting capture, and keep one armed at all times
// via this persistent event handle.
Handle g_bufferErrorEvent = 0;
Handle g_receiveEvent = 0;

std::uint8_t g_preview[kPreviewWidth * kPreviewHeight * 4];

// quirc's own pixel buffer is the working grayscale image; remember its
// stride so the copy stays a plain memcpy per row.
int g_quircStride = 0;

// Decode every other captured frame: the identify pass is the fixed cost,
// and halving it keeps the Old 3DS menu responsive.
int g_scanParity = 0;
} // namespace

namespace DsQrScanner
{

bool start(std::string &outError)
{
	if (g_started)
		return true;

	Result rc = camInit();
	if (R_FAILED(rc))
	{
		outError = "The camera service is unavailable (cam:u)";
		MC_LOG_WARN("3ds", "qr: camInit failed %08lX\n", static_cast<unsigned long>(rc));
		return false;
	}

	// The back camera: QR codes sit on screens and paper the player points
	// the console's back at, the same camera every 3DS QR reader uses.
	rc = CAMU_Activate(SELECT_OUT1);
	if (R_FAILED(rc))
	{
		outError = "The camera could not be activated";
		MC_LOG_WARN("3ds", "qr: CAMU_Activate failed %08lX\n", static_cast<unsigned long>(rc));
		camExit();
		return false;
	}

	CAMU_SetSize(SELECT_OUT1, SIZE_QVGA, CONTEXT_A);
	CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_YUV_422, CONTEXT_A);
	CAMU_SetFrameRate(SELECT_OUT1, FRAME_RATE_15);
	CAMU_SetNoiseFilter(SELECT_OUT1, true);
	CAMU_SetAutoExposure(SELECT_OUT1, true);
	CAMU_SetAutoWhiteBalance(SELECT_OUT1, true);
	CAMU_SetTrimming(PORT_CAM1, false);

	std::uint32_t transferUnit = 0;
	rc = CAMU_GetMaxBytes(&transferUnit, kCamWidth, kCamHeight);
	if (R_FAILED(rc) || transferUnit == 0)
		transferUnit = 256; // the unit every published capture example uses
	g_transferUnit = static_cast<s16>(transferUnit);
	CAMU_SetTransferBytes(PORT_CAM1, transferUnit, kCamWidth, kCamHeight);

	g_capture = static_cast<std::uint8_t *>(linearAlloc(kCamWidth * kCamHeight * 2));
	if (g_capture == nullptr)
	{
		outError = "Not enough memory for the camera frame";
		camExit();
		return false;
	}

	g_quirc = quirc_new();
	if (g_quirc == nullptr || quirc_resize(g_quirc, kCamWidth, kCamHeight) != 0)
	{
		outError = "Not enough memory for the QR decoder";
		if (g_quirc != nullptr)
		{
			quirc_destroy(g_quirc);
			g_quirc = nullptr;
		}
		linearFree(g_capture);
		g_capture = nullptr;
		camExit();
		return false;
	}
	int quircWidth = 0;
	int quircHeight = 0;
	quirc_begin(g_quirc, &quircWidth, &quircHeight);
	g_quircStride = quircWidth;

	rc = CAMU_ClearBuffer(PORT_CAM1);
	if (R_FAILED(rc))
		MC_LOG_WARN("3ds", "qr: CAMU_ClearBuffer failed %08lX\n", static_cast<unsigned long>(rc));
	rc = CAMU_GetBufferErrorInterruptEvent(&g_bufferErrorEvent, PORT_CAM1);
	if (R_FAILED(rc))
		MC_LOG_WARN("3ds", "qr: CAMU_GetBufferErrorInterruptEvent failed %08lX\n", static_cast<unsigned long>(rc));

	g_started = true; // early enough for stop() to tear the camera down on any failure below

	// FBI's proven order: the receive is armed FIRST and capture started
	// after. Starting capture with no armed receive drops the first real
	// frames and latches a buffer error that stalls the port silently.
	rc = CAMU_SetReceiving(&g_receiveEvent, g_capture, PORT_CAM1,
	                       kCamWidth * kCamHeight * 2, g_transferUnit);
	if (R_FAILED(rc))
	{
		outError = "The camera could not receive frames";
		MC_LOG_WARN("3ds", "qr: CAMU_SetReceiving failed %08lX\n", static_cast<unsigned long>(rc));
		stop();
		return false;
	}
	rc = CAMU_StartCapture(PORT_CAM1);
	if (R_FAILED(rc))
	{
		outError = "The camera could not start capturing";
		MC_LOG_WARN("3ds", "qr: CAMU_StartCapture failed %08lX\n", static_cast<unsigned long>(rc));
		stop();
		return false;
	}

	g_scanParity = 0;
	MC_LOG_INFO("3ds", "qr: scanner ready (%dx%d YUV422 @ 15 fps)\n", kCamWidth, kCamHeight);
	return true;
}

void stop()
{
	if (!g_started && g_capture == nullptr && g_quirc == nullptr)
		return;

	if (g_started)
	{
		// The official capture example's teardown order: stop the port
		// first, then let the service settle before handing the sensor
		// back. A pending receive would keep the port busy, and scan()
		// closes its event on every call, so there is none outstanding.
		CAMU_StopCapture(PORT_CAM1);
		bool busy = true;
		for (int attempt = 0; attempt < 30 && busy; ++attempt)
		{
			if (R_FAILED(CAMU_IsBusy(&busy, PORT_CAM1)))
				break;
			svcSleepThread(10 * 1000 * 1000LL);
		}
		CAMU_ClearBuffer(PORT_CAM1); // FBI's teardown also clears, so no latched error survives a restart
		CAMU_Activate(SELECT_NONE);
	}

	if (g_quirc != nullptr)
	{
		quirc_destroy(g_quirc);
		g_quirc = nullptr;
	}
	if (g_receiveEvent != 0)
	{
		svcCloseHandle(g_receiveEvent);
		g_receiveEvent = 0;
	}
	if (g_bufferErrorEvent != 0)
	{
		svcCloseHandle(g_bufferErrorEvent);
		g_bufferErrorEvent = 0;
	}
	g_frameArrived = false;
	if (g_capture != nullptr)
	{
		linearFree(g_capture);
		g_capture = nullptr;
	}
	g_quircStride = 0;
	camExit();
	g_started = false;
	MC_LOG_INFO("3ds", "qr: scanner stopped\n");
}

bool isActive()
{
	return g_started;
}

bool hasReceivedFrame()
{
	return g_frameArrived;
}

bool scan(std::string &outUrl, std::string &outError)
{
	outError.clear();
	if (!g_started || g_capture == nullptr || g_quirc == nullptr)
	{
		outError = "The camera is not running";
		return false;
	}

	// Wait on the already-armed receive (kept perpetual since start()) plus
	// the buffer-error event, FBI's task_capture_cam shape:
	// svcWaitSynchronizationN over both handles.
	Handle events[2] = { g_bufferErrorEvent, g_receiveEvent };
	s32 signaled = -1;
	Result rc = svcWaitSynchronizationN(&signaled, events, 2, false, kCaptureTimeoutNs);
	if (R_FAILED(rc))
		return false; // no frame in time; the caller keeps scanning

	if (signaled == 0)
	{
		// A frame landed with no receive armed and latched the buffer error;
		// the port stalls until the FBI recovery order: clear the error
		// flags, re-arm the receive, restart capture.
		svcCloseHandle(g_receiveEvent);
		g_receiveEvent = 0;
		MC_LOG_WARN("3ds", "qr: camera buffer error, recovering\n");
		CAMU_ClearBuffer(PORT_CAM1);
		if (R_SUCCEEDED(CAMU_SetReceiving(&g_receiveEvent, g_capture, PORT_CAM1,
		                                  kCamWidth * kCamHeight * 2, g_transferUnit)))
			CAMU_StartCapture(PORT_CAM1);
		return false;
	}

	// The receive completed: consume the event and re-arm IMMEDIATELY,
	// before any decode work. The sensor keeps pushing frames at 15 fps and
	// any frame landing during a disarmed window latches the error above.
	svcCloseHandle(g_receiveEvent);
	g_receiveEvent = 0;
	CAMU_SetReceiving(&g_receiveEvent, g_capture, PORT_CAM1,
	                  kCamWidth * kCamHeight * 2, g_transferUnit);

	g_frameArrived = true;

	// The cam service wrote through DMA into memory the CPU may still hold
	// cached lines for -- invalidate before reading, or a stale line turns
	// a decodable frame into noise. GSPGPU_InvalidateDataCache is the
	// documented handle-free way to do it for exactly this case.
	GSPGPU_InvalidateDataCache(g_capture, kCamWidth * kCamHeight * 2);

	// One pass over the YUV422 frame: the preview (Y replicated to RGBA,
	// half scale) and quirc's grayscale image, whose row stride is the
	// buffer width quirc handed out at start().
	int quircW = 0;
	int quircH = 0;
	std::uint8_t *pixels = quirc_begin(g_quirc, &quircW, &quircH);
	if (pixels == nullptr)
		return false;

	for (int y = 0; y < kCamHeight; ++y)
	{
		const std::uint8_t *yuvRow = g_capture + (y * kCamWidth) * 2;
		std::uint8_t *grayRow = pixels + y * g_quircStride;
		for (int x = 0; x < kCamWidth; ++x)
			grayRow[x] = yuvRow[x * 2];
		if ((y & 1) == 0)
		{
			// Reversed row order for the GUI: this console's sampler
			// displays "file order" rows flipped against Java art (the
			// whole reason the shipped assets and the flip scripts store
			// display PNGs upside down -- DsAssetConvert, the pak recipe),
			// so a right-side-up preview hands it the camera frame with
			// its bottom row first. quirc's grayscale plane keeps the
			// natural order: decode is rotation-agnostic.
			const int py = (kCamHeight - 1 - y) / 2;
			std::uint8_t *previewRow = g_preview + py * kPreviewWidth * 4;
			for (int x = 0; x < kPreviewWidth; ++x)
			{
				const std::uint8_t luma = yuvRow[(x * 2) * 2];
				previewRow[x * 4 + 0] = luma;
				previewRow[x * 4 + 1] = luma;
				previewRow[x * 4 + 2] = luma;
				previewRow[x * 4 + 3] = 0xFF;
			}
		}
	}
	quirc_end(g_quirc);

	// Decode every other frame; a no-code frame is the normal case and the
	// caller keeps scanning.
	if ((++g_scanParity & 1) != 0)
		return false;

	const int count = quirc_count(g_quirc);
	for (int index = 0; index < count; ++index)
	{
		struct quirc_code code;
		struct quirc_data data;
		quirc_extract(g_quirc, index, &code);
		const quirc_decode_error_t err = quirc_decode(&code, &data);
		if (err == QUIRC_SUCCESS && data.payload_len > 0)
		{
			outUrl.assign(reinterpret_cast<const char *>(data.payload),
			              static_cast<std::size_t>(data.payload_len));
			return true;
		}
		// A rejected candidate is ordinary (half-scanned frame, reflection
		// glare); the next frame usually lands it.
	}
	return false;
}

std::uint32_t previewWidth()
{
	return kPreviewWidth;
}

std::uint32_t previewHeight()
{
	return kPreviewHeight;
}

const std::uint8_t *previewFrame()
{
	return g_preview;
}

} // namespace DsQrScanner

#endif // CTR_PLATFORM
