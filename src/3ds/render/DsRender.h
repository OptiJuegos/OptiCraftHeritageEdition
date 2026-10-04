#pragma once

// DsRender.h -- citro3d primitives behind the 3DS RenderAPI surface.
//
// RenderAPI_CTR_3DS.cpp owns the GL-shaped state the shared game code drives
// (capabilities, blend factors, matrix mode, bound texture name) and pushes it
// here at draw time; this layer owns everything that is genuinely PICA200:
// the render target, the frame lifecycle, the vertex staging arenas, the
// shader program and the GPU state translation.
//
// The split mirrors the Wii backend (RenderAPI_GX_WII.cpp over WiiNative*),
// and exists for the same reason: the platform surface stays pure C++ with no
// console headers in it, so the shared code can be reasoned about without
// knowing anything about the hardware below.

#include "platform/RenderAPI.h"

namespace ds
{

// GL-shaped fixed-function state, mutated by RenderAPI_CTR_3DS.cpp and pushed
// to the PICA immediately before every draw. Defaults are GL's defaults.
struct GpuState
{
	bool blend = false;
	RenderBlendFactor blendSrc = RenderBlendFactor::One;
	RenderBlendFactor blendDst = RenderBlendFactor::Zero;

	bool depthTest = false;
	bool depthWrite = true;
	RenderCompare depthFunc = RenderCompare::Less;

	bool alphaTest = false;
	RenderCompare alphaFunc = RenderCompare::Always;
	float alphaRef = 0.0f;

	bool cullFace = false;
	RenderFace cullFaceMode = RenderFace::Back;

	bool texture2d = false;
	int boundTexture = 0;

	bool colorWriteR = true;
	bool colorWriteG = true;
	bool colorWriteB = true;
	bool colorWriteA = true;

	// GL's current-colour register (glColor4f/glColor3f): the colour taken by
	// every vertex that arrives without one baked into the mesh -- the sky
	// dome, the horizon band, the sun/moon quads, GUI overlays. renderColor4f
	// stores it here and ds::draw writes it into the staged colour words of a
	// hasColor == false mesh, which is exactly the semantic the PS2/Wii
	// backends implement (ps2_render_color4f, WiiNativeState's current_color)
	// and GL spells with the fixed-function current colour.
	float currentColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

// Lifecycle. init() brings up C3D, the shader and the render target; it is
// idempotent and returns false on failure, in which case every other call
// here degrades to a no-op and the frame loop simply presents VBlanks (the
// phase-1 heartbeat behaviour, but with the reason logged).
bool init();
void fini();

// Close the open citro3d frame if there is one (renderSubmitFrame: start the
// asynchronous present as early as the last draw, without waiting for it).
void submitFrame();

// Mirror "a system applet owns the foreground" (the swkbd dialog, see
// 3ds/DsSwkbd.h) into the renderer: while set, no citro3d frame is opened, so
// every draw, clear and bottom-panel pass no-ops and the screens stay the
// applet's. The game loop keeps running -- only the GPU side stands down.
void setAppletForeground(bool active);

// Close the open frame if there is one, then report whether a citro3d frame
// has ended since the last call that returned true. swapBuffers uses the
// return value to decide its own pacing: a submitted frame is paced by the
// next frame's C3D_FrameBegin(C3D_FRAME_SYNCDRAW), so only an iteration that
// submitted nothing should wait for VBlank itself -- the long startGame()
// stretches with no LoadingScreenRenderer frame, for instance.
bool present();

// Clear values, in GL terms.
void setClearColor(float r, float g, float b, float a);
void setClearDepth(double depth);

// Clear the current frame's target. Opens a frame first when there is not
// one: in this port a colour clear is where a frame starts (see the frame
// lifecycle note in DsRender.cpp).
void clear(unsigned mask);

// The bottom LCD as a second render target: the dual-screen GUI lays its
// menus and gameplay widgets out there (every GuiScreen on this port) and
// VirtualKeyboard's fallback panel draws on the same surface when the
// system keyboard is unavailable (see the bottom-panel section in
// DsRender.cpp). keyboardBottomBegin() creates the 240x320 rotated target
// lazily, binds it and clears it for this frame's draws; keyboardBottomEnd()
// binds the top target back so the rest of the frame lands where it did.
// Begin returns false when the panel cannot be made (VRAM, no renderer) --
// the caller then draws on the top screen instead.
bool keyboardBottomBegin();
void keyboardBottomEnd();
// True once that target exists, i.e. the panel is the game's surface: the
// boot console shares its framebuffer, so writes to it would be scribbled
// over by every frame's transfer (Log.cpp stops sending stdout there).
bool bottomPanelOwned();

// The logical (panel-space, 400x240) viewport. Recorded for
// renderGetViewport()/ActiveRenderInfo; the GPU viewport is the whole rotated
// target and is installed by C3D_FrameDrawOn, because every viewport the game
// asks for is either the full screen or -- GuiEnchantment's book, the one
// sub-viewport in the game -- not on this milestone's path.
void setViewport(int x, int y, int width, int height);
void getViewport(int* values);

// Stage the mesh and draw it. Returns false only when the layout cannot be
// represented (short positions, or a stride the fixed 32-byte attribute order
// does not cover) -- the same contract renderDrawInterleaved documents.
bool draw(const RenderInterleavedMesh& mesh, const GpuState& state);

// draw() variant for meshes ALREADY living in the linear heap and stable for
// the whole frame (the display-list section buffers allocated through
// allocLinear below): no arena staging happens at all, so per-frame cost is
// the state flush and nothing else. Meshes without baked vertex colours are
// not representable here (there is no staged copy to receive the
// current-colour fill) -- drawLinear hands those to the staged path itself.
bool drawLinear(const RenderInterleavedMesh& mesh, const GpuState& state);

// Linear-heap storage for long-lived draw data (display-list sections). The
// platform surface stays free of console headers, so the PICA-facing
// allocation contract (the PICA reads linear memory through its physical
// window; a buffer must outlive the command queue referencing it, and a
// section buffer's whole frame is submitted after the CPU-side build phase
// that rewrote it) is owned here.
void* allocLinear(std::size_t bytes);
void freeLinear(void* p);

} // namespace ds
