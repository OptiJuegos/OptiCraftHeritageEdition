// gx_wii.h — Wii VIDEO/GX display integration.
//
// Rendering state, textures and geometry live in src/wii/render and are
// submitted directly to libogc GX. This file owns only VIDEO/GX startup, the
// external framebuffers and the EFB->XFB flip.

#pragma once
#ifdef WII_PLATFORM

void wiigl_init(bool widescreen);
int  wiigl_width();
int  wiigl_height();

void wiigl_begin_frame();

// Close the frame on the GP side without waiting for it: queue the EFB->XFB
// copy and a draw-done token, then return. Whatever the caller does next runs
// while the GP is still draining the FIFO and copying.
//
// Idempotent within a frame -- a second call before wiigl_end_frame() does
// nothing -- so a caller that does not know whether the frame was already
// submitted can call it unconditionally.
void wiigl_submit_frame();

// Cancel a submitted-but-unpresented frame because a new one has started being
// drawn. Its queued copy holds the previous image, so presenting it would put
// the caller's frame one swap late; this retires the copy and hands its buffer
// back so the next wiigl_end_frame() submits what is in the EFB by then.
void wiigl_discard_pending_frame();

// Wait for the submitted frame, hand its XFB to the video interface and block
// until the retrace. Submits first if wiigl_submit_frame() was not called, so
// this remains a complete frame boundary on its own.
void wiigl_end_frame();
void wiigl_shutdown();

// Read the currently displayed XFB into packed RGB8. This is a native VIDEO/XFB
// readback, not an OpenGL-style framebuffer emulation.
bool wiigl_read_display_rgb(unsigned char *rgb, int width, int height);

// GX does not snoop the Broadway data cache. Flush CPU-written buffers before
// GX reads them as vertices, display lists or texture data.
void wiigl_flush_cache(const void* data, unsigned int bytes);

#endif // WII_PLATFORM
