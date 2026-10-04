#pragma once

// DsQrScanner.h -- the 3DS back camera as a QR-code reader, for the
// "Descarga QR" screen (src/3ds/qr/GuiQrDownload.h).
//
// The cam:u service fills a linear-heap buffer with YUV422 frames at
// QVGA/15fps; the decoder is the vendored quirc (external/quirc, ISC),
// which takes 8-bit grayscale and has no other dependency. quirc decodes
// planar codes at any in-plane rotation, so the player can hold the code
// any way up -- only mirroring would defeat it, and nothing in this
// pipeline mirrors.
//
// Everything runs on the GAME thread: a scan() call waits for the next
// camera frame (bounded, ~150 ms worst case), copies the luminance plane
// into quirc's buffer and runs one decode pass. At the 3DS menu this is
// exactly the right shape -- no worker threads to baby-sit, B cancels
// instantly, and the frame budget of a menu screen absorbs the work.
// Capture is the camera's own 15fps, so most scan() calls return with the
// SAME frame age quirc already saw; the decode itself runs on every other
// captured frame, which is what an Old 3DS's 268 MHz ARM11 sustains while
// keeping the menu at 60.

#include <cstdint>
#include <string>

namespace DsQrScanner
{

// Bring the camera + decoder up. False + a player-readable reason when the
// camera cannot be started (cam:u missing, another title holds the sensor,
// no memory). Idempotent.
bool start(std::string &outError);

// True once at least one camera frame arrived since the last start(): the
// scanner is delivering but no QR has decoded yet. Sticky-false after a
// timeout streak is the "black preview" state (emulator without a camera
// backend configured, or a service that never answers), and the GUI turns
// that into a prompt instead of silence.
bool hasReceivedFrame();

// Release the camera and the decoder. Safe when not started; the camera is
// a console-wide resource, so every start() owner MUST call this before the
// screen goes away (onGuiClosed).
void stop();
bool isActive();

// Grab the next camera frame, refresh the preview and run one decode pass.
// Returns true when a QR payload was read this frame; outUrl then holds the
// decoded text (a http(s):// URL, validated by the caller). A capture
// timeout or a frame with no readable code returns false -- keep calling.
bool scan(std::string &outUrl, std::string &outError);

// Camera preview for the GUI: a downscaled RGBA8 image (luminance
// replicated to RGB, opaque), refreshed by every scan() call. The GUI
// uploads it with renderTextureBeginUpload/renderTextureImageRgba on its
// own texture name.
std::uint32_t previewWidth();
std::uint32_t previewHeight();
const std::uint8_t *previewFrame();

} // namespace DsQrScanner
