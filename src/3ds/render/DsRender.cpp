// DsRender.cpp -- the citro3d half of the 3DS render backend: context, frame
// lifecycle, render target, vertex staging and the GL-to-PICA state
// translation. The GL-shaped surface the game drives lives in
// RenderAPI_CTR_3DS.cpp; the matrix stacks in DsMatrix.cpp; textures in
// DsTexture.cpp.
//
// == Frame lifecycle ==========================================================
//
// A colour clear is where a frame starts. Minecraft renders its loading
// screens from inside the tick (LoadingScreenRenderer draws between two
// halves of startGame), so there is no single "begin frame" call the backend
// could hang a present on; instead the game's own rhythm -- clear, draw...,
// renderSubmitFrame(), swap -- is the only reliable shape. Concretely:
//
//   * ds::clear() opens a citro3d frame if there is not one (C3D_FrameBegin
//     with C3D_FRAME_SYNCDRAW, then C3D_FrameDrawOn installs the target and
//     its full-target viewport). A mid-frame colour clear does not close
//     anything: citro3d presents at C3D_FrameEnd, so both the half-drawn
//     earlier scene and the new one land in the same presented frame -- which
//     is exactly what the overwriting loading screen wants.
//   * ds::submitFrame() (renderSubmitFrame, Minecraft.cpp's last draw point)
//     closes the frame: C3D_FrameEnd queues the display transfer and hands
//     the command list to the GX queue, asynchronously.
//   * ds::present() (Display::swapBuffers) closes a frame if one is somehow
//     still open, then reports whether a frame has been *submitted* since
//     the last time it said yes. Only a swap that submitted nothing waits
//     for a VBlank itself: submitted frames are held to the LCD refresh by
//     endFrame()'s early-finish VBlank wait (SYNCDRAW alone does NOT pace
//     the refresh -- it waits on PSC0, the GPU draining its command list),
//     and stacking a second wait on top would halve the frame rate during
//     normal play.
//
// == Bottom panel (dual-screen GUI + keyboard fallback) ========================
//
// The dual-screen GUI owns the BOTTOM LCD for its menus and gameplay widgets
// (title options, touch hotbar, containers -- every GuiScreen on this port),
// and the on-screen keyboard fallback draws its panel there too. Both need a
// second render target in this frame's lifecycle. keyboardBottomBegin()
// (renderBottomPanelBegin()'s ds-level half) creates it lazily (VRAM: colour
// + depth for a 240x320 rotated storage -- the same quarter turn and
// RGBA8 -> RGB8 transfer flags as the top target, just sized for a 320x240
// panel), binds it and clears it; keyboardBottomEnd() binds the top target
// back so the rest of the frame lands where it did. Passes run strictly one
// after another -- each Begin splits the frame, then hands out a freshly
// cleared panel, so a pass replaces (never stacks on) what an earlier pass of
// the same frame drew.
//
// Transfers follow citro3d's used-bit rule: a target is transferred to its
// LCD at C3D_FrameEnd only when C3D_FrameDrawOn marked it used during that
// frame, and the panel is single-buffered (main_3ds.cpp, like the console it
// replaces), so a frame that stops drawing on it would leave the last
// keyboard image on the LCD indefinitely. endFrame() therefore pushes one
// black frame on the first frame after drawing stops -- FrameDrawOn marks the
// target used for that one clear -- and afterwards the panel simply is not
// transferred anymore, which also leaves the console's framebuffer free for
// the next printf.
//
// Taking the panel over also takes the pixel format with it: consoleInit()
// leaves its screen at RGB565 for the console's text (libctru console.c),
// which cannot be scanned against this target's RGB8 transfer. The first
// keyboardBottomBegin() therefore switches the bottom LCD to BGR8, and
// CrashHandler_3ds.cpp switches it back to RGB565 when a crash report needs
// the console readable again.
//
// == Orientation and depth ====================================================
//
// The top panel is 400x240, but its framebuffer is stored 240x400: the LCD is
// scanned sideways. Clip space therefore needs a fixed quarter turn, and the
// PICA's depth range differs from GL's. Both ride on one matrix, kPicaTilt,
// pre-multiplied into whatever projection the game sets up -- the same R * D
// that citro3d's own Mtx_OrthoTilt / Mtx_PerspTilt bake in for their callers,
// here derived from their source and applied uniformly so the ortho GUI and
// the perspective world come out right-side up with one shader:
//
//   x' =  y          (quarter turn in NDC x/y)
//   y' = -x
//   z' = 0.5z - 0.5w (GL clip z [-w,+w] -> PICA [-w,0]; GL near stays at -w)
//   w' =  w
//
// citro3d's C3D_Init then installs C3D_DepthMap(true, -1, 0), which stores
// depth negated: the near plane becomes the LARGEST stored value and the
// default depth test is GPU_GREATER. Rather than fight that, the backend
// keeps it and flips every non-symmetric GL comparison at draw time -- see
// picaDepthFunc().
//
// == Vertex staging ===========================================================
//
// The PICA reads vertex data through the physical window at 0x18000000+, so
// buffers must come from the linear heap (BufInfo_Add rejects anything
// below it -- the game's own Tessellator allocations never qualify), and the
// bytes must stay intact from the C3D_DrawArrays call until the command list
// drains at frame end. Both are satisfied the same way: every mesh is copied
// into one of a small ring of linear arenas, and the ring is only rewound at
// C3D_FrameBegin(C3D_FRAME_SYNCDRAW), which by construction waits for the
// previous frame's queue to finish. Quads -- the Tessellator's primitive --
// index through a static (0,1,2,0,2,3) index buffer, since the PICA has no
// quad rasteriser (the memcpy fan in draw() remains as the no-buffer
// fallback).
//
// == Command-buffer pressure ==================================================
//
// citro3d records every draw's state reprogramming into one command buffer
// allocated by C3D_Init, and libctru's GPUCMD_AddInternal svcBreak()s
// (USERBREAK_PANIC -- a crash, not a dropped draw) the moment a command
// would no longer fit. A Minecraft frame reprogrammes the full state per
// Tessellator::draw() -- two 4x4 matrices, depth/alpha/blend/cull, a texture
// unit and a TexEnv stage, roughly 80-100 words per draw -- and the world
// pass reaches the default 256 KB budget a few hundred draws in, mid-frame
// (measured: the first crashing run died at a 66,621-word offset against a
// 65,536-word buffer, from Tessellator::draw() through C3D_DrawArrays). Two
// halves keep that from ever happening: the buffer is 1 MB here (see
// kCommandBufferBytes), and ds::draw() hands the recorded chunk to the GX
// queue with C3D_FrameSplit when the remaining room runs low -- citro3d's
// own mechanism for long frames, which continues recording into the rest of
// the buffer with the GPU state carried across the chunk boundary (the
// chunks execute in order, at C3D_FrameEnd's gxCmdQueueRun). The arenas are
// unaffected: their data stays untouched for the whole frame, and the ring is
// only rewound at the next C3D_FrameBegin(C3D_FRAME_SYNCDRAW), which waits
// for that queue to drain.

#include "3ds/render/DsRender.h"

#include <3ds.h>
#include <citro3d.h>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdint>

#include "3ds/render/DsMatrix.h"
// picasso's -h output: the uniform registers of DsShader.v.pica.
#include "DsShader_shbin.h"
// The assembled shader words, embedded by cmake/DsShaderEmbed.cmake (this is
// the CMake-side stand-in for devkitPro's bin2s + assembler bridge).
#include "DsShader_shbin_data.h"
#include "3ds/render/DsTexture.h"
#include "platform/Log.h"

namespace ds
{
namespace
{

// Framebuffer (rotated storage) and panel (logical) dimensions.
constexpr int kTargetWidth = 240;
constexpr int kTargetHeight = 400;
constexpr int kPanelWidth = 400;
constexpr int kPanelHeight = 240;

// The bottom LCD, as the keyboard panel sees it: the storage is rotated 90
// degrees exactly like the top screen's (240 wide x 320 tall for a 320x240
// panel), so the same kPicaTilt quarter turn and the same RGBA8 -> RGB8
// transfer flags as the top target apply unchanged.
constexpr int kBottomTargetWidth = 240;
constexpr int kBottomTargetHeight = 320;

// The fixed 32-byte Minecraft vertex, loaded as five attributes in order.
// The colour attribute is u8x4 in R,G,B,A byte order: the Tessellator packs
// (A<<24)|(B<<16)|(G<<8)|R on this little-endian target (Tessellator.cpp,
// setColorRGBA), which is that byte sequence. The PICA200 vertex loader does
// NOT normalise integer attribute formats (GPUREG_ATTRIBBUFFERS_FORMAT has a
// 2-bit type per attribute and no normalise bit; devkitPro's loop_subdivision
// example feeds a u8 valence to its geometry shader as a raw integer), so the
// colour arrives in the shader as raw 0..255 and DsShader.v.pica rescales it
// to [0,1] before it reaches the TexEnv inputs. The fourth normal byte is
// padding and unread.
constexpr int kVertexStride = 32;
constexpr int kAttribCount = 5;
// Attribute k feeds shader input register k, for all five: 0x43210.
// (Same nibble convention AttrInfo_AddLoader and BufInfo_Add both use.)
constexpr u64 kAttribPermutation = 0x43210;

// Staging arenas: see the vertex staging section above. 4 x 32768 vertices is
// one megabyte each -- generous for a menu/HUD frame. One 32768-slot arena is
// also the single-draw ceiling: a dense 16x16x16 section captures to ~21k
// vertices (measured: "draw with 21096 vertices exceeds one arena" in the
// 2026-09-25 log, a dropped chunk), so 16384-slot arenas were silently losing
// the densest sections of all. A dense world frame still needs more than one
// pass through the ring, so rather than dropping draws the ring grows one
// arena at a time up to kArenaMaxCount; arenas persist once grown because the
// frames that needed them keep needing them. The cap keeps the linear heap
// budget at 8 MB, as it was with the old 16 x 16384 ring, and only past the
// cap does the overflow path drop the draw and log once rather than corrupt
// one that is still in flight.
constexpr int kArenaInitialCount = 4;
constexpr int kArenaMaxCount = 8;
constexpr int kArenaVertices = 32768;

// The command buffer citro3d records into, in bytes. C3D_DEFAULT_CMDBUF_SIZE
// is 0x40000 (256 KB / 65,536 words); a world frame spends ~66K words of
// full-state reprogramming, so the default is a mid-frame svcBreak -- see the
// command-buffer pressure section above. 1 MB is still small against the
// 64 MB Old-3DS application region.
constexpr u32 kCommandBufferBytes = 0x100000;

// Room that ds::draw() insists on having left in the command buffer before
// emitting one more draw's state: comfortably above the ~100 words a fully
// reprogrammed state costs, cheap to check per draw.
constexpr u32 kCommandReserveWords = 1024;

// RGBA8 target transferred down to the LCD's RGB8 scanout.
constexpr u32 kDisplayTransferFlags =
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
     GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

// T = R * D, in GL column-major layout. See the orientation section above;
// the full derivation lives in DsShader.v.pica's header comment.
constexpr float kPicaTilt[16] = {
	 0.0f, -1.0f,  0.0f, 0.0f, // column 0
	 1.0f,  0.0f,  0.0f, 0.0f, // column 1
	 0.0f,  0.0f,  0.5f, 0.0f, // column 2
	 0.0f,  0.0f, -0.5f, 1.0f, // column 3
};

bool s_c3dActive = false;
C3D_RenderTarget* s_target = nullptr;
// The target the current pass draws into: s_target (top screen), or
// s_bottomTarget while a bottom-panel pass runs. clear() targets this, so a
// screen's own mid-draw renderClear() hits the surface it is drawing on.
C3D_RenderTarget* s_boundTarget = nullptr;
// The bottom panel, for the dual-screen GUI and the keyboard fallback
// (keyboardBottomBegin below).
// Created lazily -- most sessions never open a text field, and the target
// costs ~600 KB of VRAM (colour + depth) for as long as it exists.
C3D_RenderTarget* s_bottomTarget = nullptr;
// Set while a frame draws the panel, read at endFrame() to decide whether the
// panel needs a transfer this frame -- see the bottom-panel section below.
bool s_bottomDrawnThisFrame = false;
// The LCD still shows the panel: it needs one black frame once drawing stops.
bool s_bottomPanelShown = false;
DVLB_s* s_dvlb = nullptr;
shaderProgram_s s_program;
bool s_programReady = false;

bool s_inFrame = false;
u32 s_framesSubmitted = 0;
u32 s_framesPresented = 0;

// Quads -- the Tessellator's primitive: the PICA has no quad rasteriser, so
// they arrive as triangles either pre-expanded (the memcpy fan in draw(),
// kept as the no-IB fallback) or via a static index buffer shared by every
// quad draw. One arena holds kArenaVertices staged vertices, so one arena of
// quad data needs (kArenaVertices / 4) * 6 u16 indices; the pattern below is
// GL's own quad split, (0,1,2) then (0,2,3), so winding and culling see the
// same faces either way.
constexpr int kQuadIndexCount = kArenaVertices / 4 * 6;

void* s_arenas[kArenaMaxCount] = {};
int s_arenaCount = kArenaInitialCount;
int s_arenaIndex = 0;
int s_arenaCursor = 0;
u16* s_quadIndices = nullptr;
bool s_arenaOverflowLogged = false;
bool s_arenaGrowthLogged = false;

float s_clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
double s_clearDepth = 1.0;
int s_viewport[4] = {0, 0, kPanelWidth, kPanelHeight};

float clamp01(float v)
{
	return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// out = left * right, all column-major GL float[16]s.
void matrixMultiply(float* out, const float* left, const float* right)
{
	float tmp[16];
	for (int c = 0; c < 4; ++c)
	{
		for (int r = 0; r < 4; ++r)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k)
				sum += left[k * 4 + r] * right[c * 4 + k];
			tmp[c * 4 + r] = sum;
		}
	}
	std::memcpy(out, tmp, sizeof(tmp));
}

// GL column-major -> citro3d's rows: r[i] is mathematical row i, so element
// (i, c) of the GL matrix is r[i]'s member for column c. Assigning by member
// name (never by flat index -- C3D_FVec stores w,z,y,x in memory) is exactly
// how citro3d's own Mtx_Ortho family builds its matrices, which is what makes
// the member-name comparison against Mtx_OrthoTilt that derived kPicaTilt
// trustworthy.
void toC3dMtx(C3D_Mtx* out, const float* gl)
{
	for (int i = 0; i < 4; ++i)
	{
		out->r[i].x = gl[0 * 4 + i]; // element (i, 0)
		out->r[i].y = gl[1 * 4 + i]; // element (i, 1)
		out->r[i].z = gl[2 * 4 + i]; // element (i, 2)
		out->r[i].w = gl[3 * 4 + i]; // element (i, 3)
	}
}

// GL depth comparisons are "0=near, 1=far"; citro3d's negated storage makes
// near the largest value, so the symmetric functions keep their names and
// the ordered ones swap partners. (LESS keeps GL's "smaller wins = nearer
// wins" meaning -- expressed as GREATER over the stored values.)
GPU_TESTFUNC picaDepthFunc(RenderCompare func)
{
	switch (func)
	{
	case RenderCompare::Never:       return GPU_NEVER;
	case RenderCompare::Less:         return GPU_GREATER;
	case RenderCompare::Equal:        return GPU_EQUAL;
	case RenderCompare::LessEqual:    return GPU_GEQUAL;
	case RenderCompare::Greater:      return GPU_LESS;
	case RenderCompare::NotEqual:     return GPU_NOTEQUAL;
	case RenderCompare::GreaterEqual: return GPU_LEQUAL;
	case RenderCompare::Always:      return GPU_ALWAYS;
	}
	return GPU_ALWAYS;
}

// The alpha test compares the final fragment alpha, which lives in the same
// [0,1] terms on both sides -- no depth-map flip here.
GPU_TESTFUNC picaAlphaFunc(RenderCompare func)
{
	switch (func)
	{
	case RenderCompare::Never:       return GPU_NEVER;
	case RenderCompare::Less:         return GPU_LESS;
	case RenderCompare::Equal:        return GPU_EQUAL;
	case RenderCompare::LessEqual:    return GPU_LEQUAL;
	case RenderCompare::Greater:      return GPU_GREATER;
	case RenderCompare::NotEqual:     return GPU_NOTEQUAL;
	case RenderCompare::GreaterEqual: return GPU_GEQUAL;
	case RenderCompare::Always:       return GPU_ALWAYS;
	}
	return GPU_ALWAYS;
}

GPU_BLENDFACTOR picaBlendFactor(RenderBlendFactor factor)
{
	switch (factor)
	{
	case RenderBlendFactor::Zero:              return GPU_ZERO;
	case RenderBlendFactor::One:               return GPU_ONE;
	case RenderBlendFactor::SrcColor:          return GPU_SRC_COLOR;
	case RenderBlendFactor::OneMinusSrcColor:   return GPU_ONE_MINUS_SRC_COLOR;
	case RenderBlendFactor::SrcAlpha:          return GPU_SRC_ALPHA;
	case RenderBlendFactor::OneMinusSrcAlpha:  return GPU_ONE_MINUS_SRC_ALPHA;
	case RenderBlendFactor::DstAlpha:          return GPU_DST_ALPHA;
	case RenderBlendFactor::OneMinusDstAlpha:  return GPU_ONE_MINUS_DST_ALPHA;
	case RenderBlendFactor::DstColor:          return GPU_DST_COLOR;
	case RenderBlendFactor::OneMinusDstColor:  return GPU_ONE_MINUS_DST_COLOR;
	}
	return GPU_ONE;
}

// The PICA culls a named winding, and its "front" (GPU_CULL_FRONT_CCW) is
// counter-clockwise -- GL's own default front face, and kPicaTilt is a pure
// rotation (positive determinant), so winding survives the turn intact.
// GL_FRONT_AND_BACK has no PICA equivalent: the closest honest answer is to
// cull one face, which no game path currently asks for anyway.
GPU_CULLMODE picaCullMode(RenderFace face)
{
	switch (face)
	{
	case RenderFace::Front:       return GPU_CULL_FRONT_CCW;
	case RenderFace::Back:        return GPU_CULL_BACK_CCW;
	case RenderFace::FrontAndBack: return GPU_CULL_FRONT_CCW;
	}
	return GPU_CULL_NONE;
}

// CPU-side start of the current frame, for endFrame's 60 fps VBlank hold.
static s64 s_frameStartMs = 0;
// How long this frame's frameBegin() blocked inside C3D_FrameBegin
// (SYNCDRAW), ms. On real hardware the GX queue has normally drained by then
// (the wait reads ~0), so endFrame's VBlank hold is the only pacer. Wherever
// the queue signals one refresh late (Azahar), begin() is itself a
// full-refresh wait and the hold on top doubles the frame period -- those
// frames skip the hold entirely.
static s64 s_frameSyncWaitMs = 0;

// True while a system applet (the swkbd text dialog, see 3ds/DsSwkbd.h) owns
// the foreground. The 3DS does not suspend the app while a library applet
// runs -- it keeps executing in the background -- but the applet owns both
// LCDs and the GPU work that reaches them, so the renderer must not open
// citro3d frames, and the frame loop should be paced by VBlank instead (an
// unpresented swapBuffers already does exactly that when present() reports
// no frame).
bool s_appletForeground = false;

// C3D_TexInit-style texture work is a plain allocation and stays allowed
// while the applet is up; what must stop is frame work -- frameBegin() is the
// single choke point every draw and clear already go through, so gating it
// here is the whole suppression.

void endFrame()
{
	if (!s_inFrame)
		return;
	// The bottom panel only reaches its LCD while something draws on it in
	// the frame: C3D_FrameDrawOn marks the target used and C3D_FrameEnd
	// queues a display transfer for every used target, so a frame that
	// ignores the panel leaves the LCD holding the last keyboard image
	// forever (the panel is single-buffered, main_3ds.cpp). On the first
	// frame after the keyboard stops drawing, clear the target to black --
	// the FrameDrawOn below marks it used, so the black lands exactly once
	// and after that the panel simply stops being transferred.
	if (s_bottomPanelShown && !s_bottomDrawnThisFrame && s_bottomTarget != nullptr &&
	    C3D_FrameDrawOn(s_bottomTarget))
	{
		C3D_RenderTargetClear(s_bottomTarget, C3D_CLEAR_ALL, 0, 0);
		C3D_FrameDrawOn(s_target); // the frame's own target, for completeness
		s_bottomPanelShown = false;
	}
	s_bottomDrawnThisFrame = false;
	C3D_FrameEnd(0);
	// 60 fps cap: nothing else paces the loop on the LCD refresh. A frame
	// whose CPU side finished early is held until the next VBlank --
	// without it the light scenes (the menus) submit far past 60 fps, and
	// gameplay free-ran at the serialized CPU+GPU rate instead of settling
	// on the refresh. Edge-aligned submits are what make the steady state
	// work: after a hold, the next SYNCDRAW only ever waits for the GPU
	// tail that crossed the edge, so holding is exactly what KEEPS the
	// pipeline at 60 whenever the per-frame budget fits one refresh --
	// standing the hold down when SYNCDRAW had waited (the old guard)
	// disabled the cap precisely where the work was heaviest. A frame
	// whose CPU side already overran the refresh is NOT delayed further
	// (no forced ladder); one whose total work genuinely exceeds a refresh
	// simply presents on every other VBlank. The hold is skipped when this
	// frame's frameBegin() already blocked a full-refresh scale inside
	// SYNCDRAW (the GX queue pace-setter case -- see the pacing counters):
	// stacking both waits is what cadences Azahar to every other VBlank
	// (30 fps with a ~3 ms CPU frame) instead of the refresh.
	if (s_frameStartMs != 0 && s_frameSyncWaitMs < 4 && osGetTime() - s_frameStartMs < 15)
		gspWaitForVBlank();
	s_inFrame = false;
	s_boundTarget = nullptr;
	++s_framesSubmitted;
}

// Open a citro3d frame if there is not one. SYNCDRAW is what makes the arena
// ring safe: C3D_FrameBegin waits for the previous queue to drain before
// letting the frame start, so the arenas can be rewound here.
void frameBegin()
{
	if (s_inFrame || s_target == nullptr)
		return;
	// A system applet owns the screens while this is set: opening citro3d
	// frames (and queueing any GPU work into them) is suspended until it
	// closes -- every draw/clear/bottom-panel pass no-ops because none of
	// them can get past here.
	if (s_appletForeground)
		return;
	// SYNCDRAW doubles as the pace-setter where the GX queue signals a
	// refresh late; the wait length decides below whether endFrame's VBlank
	// hold may stack on top (measured for exactly that decision).
	const s64 syncWaitStartMs = osGetTime();
	if (!C3D_FrameBegin(C3D_FRAME_SYNCDRAW))
		return;
	s_frameSyncWaitMs = osGetTime() - syncWaitStartMs;
	s_frameStartMs = osGetTime();
	if (!C3D_FrameDrawOn(s_target))
	{
		C3D_FrameEnd(0);
		return;
	}
	s_inFrame = true;
	s_boundTarget = s_target;
	s_arenaIndex = 0;
	s_arenaCursor = 0;
}

// Reserve `vertices` slots in the arena ring, wrapping to the next arena when
// the current one cannot hold the draw. Null (and one logged line) when the
// whole frame's ring is full -- see the staging section above.
u8* reserveArena(int vertices)
{
	if (vertices > kArenaVertices)
	{
		if (!s_arenaOverflowLogged)
		{
			s_arenaOverflowLogged = true;
			MC_LOG_WARN("render",
				"3ds: draw with %d vertices exceeds one arena (%d); dropped\n",
				vertices, kArenaVertices);
		}
		return nullptr;
	}
	if (s_arenaCursor + vertices > kArenaVertices)
	{
		if (s_arenaIndex + 1 >= s_arenaCount)
		{
			if (s_arenaCount >= kArenaMaxCount)
			{
				if (!s_arenaOverflowLogged)
				{
					s_arenaOverflowLogged = true;
					MC_LOG_WARN("render",
						"3ds: vertex arenas exhausted for this frame (%d x %d vertices); draws dropped\n",
						kArenaMaxCount, kArenaVertices);
				}
				return nullptr;
			}
			// Grow the ring instead of dropping the draw. linearAlloc
			// mid-frame touches no arena the GPU may still be reading, and
			// the new block is staged exactly like the boot-time ones.
			void* grown = linearAlloc(static_cast<std::size_t>(kArenaVertices) * kVertexStride);
			if (grown == nullptr)
			{
				if (!s_arenaOverflowLogged)
				{
					s_arenaOverflowLogged = true;
					MC_LOG_WARN("render",
						"3ds: vertex arena %d allocation failed; draws dropped\n",
						s_arenaCount);
				}
				return nullptr;
			}
			s_arenas[s_arenaCount] = grown;
			++s_arenaCount;
			if (!s_arenaGrowthLogged)
			{
				s_arenaGrowthLogged = true;
				MC_LOG_INFO("render",
					"3ds: vertex arena ring grown to %d arenas\n",
					s_arenaCount);
			}
		}
		++s_arenaIndex;
		s_arenaCursor = 0;
	}
	u8* out = static_cast<u8*>(s_arenas[s_arenaIndex]) +
	          static_cast<std::size_t>(s_arenaCursor) * kVertexStride;
	s_arenaCursor += vertices;
	return out;
}

// Submit the recorded chunk to the GX queue and keep recording into the rest
// of the buffer once the remaining room no longer covers one more draw's
// state. GPU state is continuous across the boundary (the chunks execute in
// order), and the staged arenas stay valid for the whole frame -- see the
// command-buffer pressure section of the header. GPUCMD_GetBuffer is the
// libctru accessor for the current chunk's size and fill level; <3ds.h>
// already pulls it in.
void splitCommandBufferIfNeeded()
{
	u32* address = nullptr;
	u32 size = 0;
	u32 offset = 0;
	GPUCMD_GetBuffer(&address, &size, &offset);
	if (offset + kCommandReserveWords > size)
		C3D_FrameSplit(0);
}

// Translate the GL-shaped state into citro3d calls and point the vertex fetch
// at the staged copy. citro3d flushes all of this per C3D_DrawArrays, so this
// is safe to call with different values for every draw in a frame.
void applyState(const GpuState& state, const void* vertexBase)
{
	// Depth: the PICA's writemask covers colour *and* depth writes (there is
	// no separate glColorMask), and 0 -- "write nothing" -- is a value the
	// hardware accepts, matching glColorMask(f,f,f,f)+glDepthMask(f).
	u32 writeMask = 0;
	if (state.colorWriteR) writeMask |= GPU_WRITE_RED;
	if (state.colorWriteG) writeMask |= GPU_WRITE_GREEN;
	if (state.colorWriteB) writeMask |= GPU_WRITE_BLUE;
	if (state.colorWriteA) writeMask |= GPU_WRITE_ALPHA;
	if (state.depthWrite)  writeMask |= GPU_WRITE_DEPTH;
	C3D_DepthTest(state.depthTest, picaDepthFunc(state.depthFunc),
	              static_cast<GPU_WRITEMASK>(writeMask));

	C3D_AlphaTest(state.alphaTest, picaAlphaFunc(state.alphaFunc),
	              static_cast<int>(clamp01(state.alphaRef) * 255.0f + 0.5f));
	C3D_CullFace(state.cullFace ? picaCullMode(state.cullFaceMode) : GPU_CULL_NONE);

	// No blend-enable switch exists through citro3d's surface, so a disabled
	// blend is spelled as the identity factors instead of whatever the last
	// enabled draw happened to leave behind.
	const GPU_BLENDFACTOR srcFactor =
		state.blend ? picaBlendFactor(state.blendSrc) : GPU_ONE;
	const GPU_BLENDFACTOR dstFactor =
		state.blend ? picaBlendFactor(state.blendDst) : GPU_ZERO;
	C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, srcFactor, dstFactor,
	               srcFactor, dstFactor);

	// Matrices: the tilt rides on the projection, the model-view goes
	// through as-is (the shader applies projection * modelView * position).
	float tilted[16];
	matrixMultiply(tilted, kPicaTilt, matrix::projectionTop());
	C3D_Mtx projection;
	C3D_Mtx modelView;
	toC3dMtx(&projection, tilted);
	toC3dMtx(&modelView, matrix::modelViewTop());
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, VSH_FVEC_projection, &projection);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, VSH_FVEC_modelView, &modelView);

	// Texture: bind and hand the shader the padding ratio. An untextured
	// draw leaves the binding alone -- stage 0 below simply does not read it.
	// The components are (ratioU, ratioV, 1-ratioV, 1): the shader folds U
	// with the ratio directly (the sampler never mirrors columns) but V
	// mirrored -- see the texcoord block of DsShader.v.pica for why the V
	// window is [1-ratioV, 1] in GPU space.
	float uvScale[2] = {1.0f, 1.0f};
	if (state.texture2d)
		texture::bind(state.boundTexture, uvScale);
	C3D_FVUnifSet(GPU_VERTEX_SHADER, VSH_FVEC_texScale,
	              uvScale[0], uvScale[1], 1.0f - uvScale[1], 1.0f);

	// TexEnv stage 0: texel * vertex colour when texturing, vertex colour
	// alone when not. Stages 1..5 stay the pass-through their init-time
	// C3D_TexEnvInit left them as.
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	if (state.texture2d)
	{
		C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR,
		              GPU_PRIMARY_COLOR);
		C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
	}
	else
	{
		C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR,
		              GPU_PRIMARY_COLOR);
		C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
	}

	// Vertex fetch: one buffer, five attributes, the fixed stride.
	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, vertexBase, kVertexStride, kAttribCount,
	            kAttribPermutation);
}

} // namespace

bool init()
{
	if (s_c3dActive)
		return s_target != nullptr;

	if (!C3D_Init(kCommandBufferBytes))
	{
		MC_LOG_ERROR("render", "3ds: C3D_Init failed\n");
		return false;
	}
	s_c3dActive = true;

	// DVLB_ParseFile walks the SHBIN container's own offsets -- the size
	// argument is unused by libctru -- but the cast is still needed: the
	// shader sits in .rodata and the API predates const-correctness.
	s_dvlb = DVLB_ParseFile(const_cast<u32*>(DsShader_shbin),
	                        static_cast<u32>(DsShader_shbin_size * 4));
	if (s_dvlb == nullptr)
	{
		MC_LOG_ERROR("render", "3ds: DVLB_ParseFile rejected the vertex shader\n");
		fini();
		return false;
	}
	Result result = shaderProgramInit(&s_program);
	if (R_FAILED(result))
	{
		MC_LOG_ERROR("render", "3ds: shaderProgramInit failed (%08lx)\n", result);
		fini();
		return false;
	}
	s_programReady = true;
	result = shaderProgramSetVsh(&s_program, &s_dvlb->DVLE[0]);
	if (R_FAILED(result))
	{
		MC_LOG_ERROR("render", "3ds: shaderProgramSetVsh failed (%08lx)\n", result);
		fini();
		return false;
	}
	C3D_BindProgram(&s_program);

	// Fixed 32-byte vertex -> five attribute loaders. One configuration for
	// the whole backend: the layout is as much a part of the mesh contract as
	// the stride is.
	C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3);         // position
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2);         // texcoord
	AttrInfo_AddLoader(attrInfo, 2, GPU_UNSIGNED_BYTE, 4);  // colour RGBA
	AttrInfo_AddLoader(attrInfo, 3, GPU_BYTE, 4);          // normal xyz + pad
	AttrInfo_AddLoader(attrInfo, 4, GPU_FLOAT, 1);         // brightness

	// Pin every TexEnv stage to its pass-through default, so stage 0 is the
	// only one this backend ever has to think about.
	for (int i = 0; i < 6; ++i)
	{
		C3D_TexEnv* env = C3D_GetTexEnv(i);
		C3D_TexEnvInit(env);
	}

	s_target = C3D_RenderTargetCreate(kTargetWidth, kTargetHeight,
	                                  GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	if (s_target == nullptr)
	{
		MC_LOG_ERROR("render", "3ds: C3D_RenderTargetCreate(%dx%d) failed\n",
		             kTargetWidth, kTargetHeight);
		fini();
		return false;
	}
	C3D_RenderTargetSetOutput(s_target, GFX_TOP, GFX_LEFT, kDisplayTransferFlags);

	// GL fixed-function defaults. citro3d's own defaults differ (depth test
	// on, GREATER, with the negated depth map); the depth *function* here is
	// deliberately GREATER-shaped because the negated storage is being kept
	// -- see picaDepthFunc().
	C3D_DepthTest(false, GPU_GREATER, GPU_WRITE_ALL);
	C3D_AlphaTest(false, GPU_ALWAYS, 0);
	C3D_CullFace(GPU_CULL_NONE);
	C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ZERO,
	               GPU_ONE, GPU_ZERO);

	for (int i = 0; i < kArenaInitialCount; ++i)
	{
		void* arena = linearAlloc(static_cast<std::size_t>(kArenaVertices) * kVertexStride);
		if (arena == nullptr)
		{
			MC_LOG_ERROR("render", "3ds: vertex arena allocation failed\n");
			fini();
			return false;
		}
		s_arenas[i] = arena;
	}
	s_arenaCount = kArenaInitialCount;
	s_arenaIndex = 0;
	s_arenaCursor = 0;
	s_arenaOverflowLogged = false;
	s_arenaGrowthLogged = false;

	// The quad index pattern is data-independent, so it is built once. The
	// PICA reads it through the same physical window as the vertex arenas, so
	// it must come from the linear heap too, and it must outlive every
	// command queue that references it -- a static allocation freed in
	// fini() is the only lifetime that can never dangle.
	s_quadIndices = static_cast<u16*>(linearAlloc(
		static_cast<std::size_t>(kQuadIndexCount) * sizeof(u16)));
	if (s_quadIndices == nullptr)
	{
		MC_LOG_ERROR("render", "3ds: quad index buffer allocation failed\n");
		fini();
		return false;
	}
	for (int q = 0; q < kArenaVertices / 4; ++q)
	{
		const u16 base = static_cast<u16>(q * 4);
		s_quadIndices[q * 6 + 0] = base;
		s_quadIndices[q * 6 + 1] = static_cast<u16>(base + 1);
		s_quadIndices[q * 6 + 2] = static_cast<u16>(base + 2);
		s_quadIndices[q * 6 + 3] = base;
		s_quadIndices[q * 6 + 4] = static_cast<u16>(base + 2);
		s_quadIndices[q * 6 + 5] = static_cast<u16>(base + 3);
	}

	MC_LOG_INFO("render",
		"3ds: citro3d up -- target %dx%d RGBA8/D24S8, %d arenas x %d vertices (growable to %d)\n",
		kTargetWidth, kTargetHeight, kArenaInitialCount, kArenaVertices, kArenaMaxCount);
	return true;
}

void fini()
{
	if (!s_c3dActive)
		return;
	endFrame();
	for (auto& arena : s_arenas)
	{
		if (arena != nullptr)
		{
			linearFree(arena);
			arena = nullptr;
		}
	}
	if (s_quadIndices != nullptr)
	{
		linearFree(s_quadIndices);
		s_quadIndices = nullptr;
	}
	texture::resetAll();
	if (s_bottomTarget != nullptr)
	{
		C3D_RenderTargetDelete(s_bottomTarget);
		s_bottomTarget = nullptr;
	}
	s_bottomDrawnThisFrame = false;
	s_bottomPanelShown = false;
	if (s_target != nullptr)
	{
		C3D_RenderTargetDelete(s_target);
		s_target = nullptr;
	}
	if (s_programReady)
	{
		shaderProgramFree(&s_program);
		s_programReady = false;
	}
	if (s_dvlb != nullptr)
	{
		DVLB_Free(s_dvlb);
		s_dvlb = nullptr;
	}
	C3D_Fini();
	s_c3dActive = false;
}

void submitFrame()
{
	endFrame();
}

void setAppletForeground(bool active)
{
	// Idempotent and cheap enough to call every frame from
	// lwjgl::Display::processMessages(): the applet's lifetime is owned by
	// DsSwkbd, this only mirrors it into the renderer.
	if (s_appletForeground == active)
		return;
	s_appletForeground = active;
}

bool present()
{
	endFrame();
	if (s_framesPresented != s_framesSubmitted)
	{
		s_framesPresented = s_framesSubmitted;
		return true;
	}
	return false;
}

void setClearColor(float r, float g, float b, float a)
{
	s_clearColor[0] = r;
	s_clearColor[1] = g;
	s_clearColor[2] = b;
	s_clearColor[3] = a;
}

void setClearDepth(double depth)
{
	s_clearDepth = depth;
}

void clear(unsigned mask)
{
	// The *bound* target, not always the top screen: GuiInventory and
	// friends clear depth mid-draw, and inside a bottom-panel pass that
	// clear has to land on the panel they are drawing.
	C3D_RenderTarget* target = s_boundTarget != nullptr ? s_boundTarget : s_target;
	if (target == nullptr)
		return;
	u32 bits = 0;
	if (mask & RenderClearMask::Color)
		bits |= C3D_CLEAR_COLOR;
	if (mask & RenderClearMask::Depth)
		bits |= C3D_CLEAR_DEPTH;
	if (bits == 0)
		return;

	frameBegin();
	if (!s_inFrame)
		return;

	// GL's float clear colour packs into citro3d's 0xRRGGBBAA fill value;
	// GL's 0..1 clear depth (0 = near, 1 = far) has to be negated to match
	// the stored depth map, which is why far (the usual request) becomes 0 --
	// the same value citro3d's own examples clear to.
	const u32 color =
	    (static_cast<u32>(clamp01(s_clearColor[0]) * 255.0f + 0.5f) << 24) |
	    (static_cast<u32>(clamp01(s_clearColor[1]) * 255.0f + 0.5f) << 16) |
	    (static_cast<u32>(clamp01(s_clearColor[2]) * 255.0f + 0.5f) << 8) |
	     static_cast<u32>(clamp01(s_clearColor[3]) * 255.0f + 0.5f);
	const float depth = static_cast<float>(s_clearDepth);
	const u32 storedDepth =
		static_cast<u32>((1.0f - depth) * 16777215.0f + 0.5f);

	// GX_MemoryFill -- what C3D_FrameBufClear turns this into -- only
	// APPENDS to the GX queue, while the draws recorded so far only reach
	// that queue at C3D_FrameEnd (C3D_FrameSplit -> GX_ProcessCommandList).
	// Without splitting first, this fill would execute BEFORE every draw of
	// the frame: a mid-frame renderClear() (the screen pass's depth clear,
	// the HUD overlay's, GuiInventory's model clear) would wipe stale
	// content and leave the world's and HUD's depth written *after* it, so
	// GUI quads at the ortho plane lose the depth test against the 3D item
	// icons sitting nearer than the plane -- the hotbar's block icons punch
	// through the inventory menu. Splitting queues the recorded draws ahead
	// of the fill; citro3d's own C3D_SyncMemoryFill does the same. An
	// empty buffer makes C3Di_SplitFrame return false, so the frame-start
	// clear stays a no-op split.
	C3D_FrameSplit(0);
	C3D_RenderTargetClear(target, static_cast<C3D_ClearBits>(bits), color,
	                      storedDepth);
}

// The bottom panel's own frame pass; see the bottom-panel section in the
// header of this file. Used by the dual-screen GUI and the on-screen
// keyboard alike. Returns false when there is nothing to draw on, in
// which case the caller keeps drawing on the top screen.
bool keyboardBottomBegin()
{
	if (s_target == nullptr)
		return false;
	if (s_bottomTarget == nullptr)
	{
		s_bottomTarget = C3D_RenderTargetCreate(kBottomTargetWidth, kBottomTargetHeight,
		                                        GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
		if (s_bottomTarget == nullptr)
		{
			MC_LOG_WARN("render",
				"3ds: bottom-screen target allocation failed; keyboard panel stays on the top screen\n");
			return false;
		}
		C3D_RenderTargetSetOutput(s_bottomTarget, GFX_BOTTOM, GFX_LEFT, kDisplayTransferFlags);
		// The boot console owned this panel (main_3ds.cpp) and its line
		// buffer still holds the boot log: start it over, so that a printf
		// after the keyboard closes writes a fresh line from the top instead
		// of redrawing stale text over the black frame. consoleClear() also
		// runs gfxFlushBuffers() over this framebuffer, draining the CPU
		// cache lines the boot text left here -- without that, a later
		// eviction would write stale glyphs across the image the display
		// transfer is about to put down (GX writes memory behind the cache).
		consoleClear();
		// Take the screen back at the panel's pixel format. consoleInit()
		// reconfigures its LCD to RGB565 for its own text (libctru's
		// console.c forces GSP_RGB565_OES on the screen it is given), while
		// this target transfers RGBA8 -> RGB8: scanning a 3-byte image as
		// 2-byte pixels squeezes every row by 1.5x and re-packs the
		// channels -- red and blue interleave, the whole panel goes pink and
		// noisy, with the interface only faintly recognisable underneath.
		// BGR8 is the format gfxInitDefault gave this screen and the one the
		// top LCD keeps, so both panels end up scanned alike. The buffers
		// were sized 240x320x3 during gfxInit and gfxSetScreenFormat only
		// ever grows a buffer, so the console's cached pointer stays valid;
		// its output is suppressed while the panel is owned (Log.cpp), and
		// Crash() swaps the format back when the crash report needs readable
		// console text again (CrashHandler_3ds.cpp).
		gfxSetScreenFormat(GFX_BOTTOM, GSP_BGR8_OES);
	}
	frameBegin();
	if (!s_inFrame)
		return false;
	if (!C3D_FrameDrawOn(s_bottomTarget))
		return false;
	s_boundTarget = s_bottomTarget;
	// Colour starts black behind the panel's own rectangles (clear colour 0
	// is the same value clear() uses for the top screen); depth goes to far,
	// the same stored 0, so no draw ever reads the previous frame's.
	// Split first, exactly as clear() does: a fill only appends to the GX
	// queue while the draws recorded so far only reach it at C3D_FrameEnd,
	// so without the split this clear would run ahead of every pass already
	// recorded -- including an earlier bottom-panel pass of this same frame
	// (title menu, then the keyboard) -- and wipe it after the fact. With
	// the split the earlier passes land first and this Begin really does
	// hand out a freshly cleared panel.
	C3D_FrameSplit(0);
	u32 bits = C3D_CLEAR_COLOR | C3D_CLEAR_DEPTH;
	C3D_RenderTargetClear(s_bottomTarget, static_cast<C3D_ClearBits>(bits), 0, 0);
	s_bottomDrawnThisFrame = true;
	s_bottomPanelShown = true;
	return true;
}

void keyboardBottomEnd()
{
	if (s_target == nullptr || !s_inFrame)
		return;
	// Hand the frame back to the top screen: C3D_FrameDrawOn re-installs the
	// top target's framebuffer and its full-target viewport, so everything
	// drawn after the panel lands where it did before.
	C3D_FrameDrawOn(s_target);
	s_boundTarget = s_target;
}

bool bottomPanelOwned()
{
	return s_bottomTarget != nullptr;
}

void setViewport(int x, int y, int width, int height)
{
	// Recorded only: the GPU viewport is the full rotated target, installed
	// by C3D_FrameDrawOn. Every viewport the game asks for is the full screen
	// (Minecraft, EntityRenderer, GuiAchievement) -- except GuiEnchantment's
	// 320x240 book window, which is not on this milestone's path and would
	// need the logical->rotated rectangle transform once it is.
	s_viewport[0] = x;
	s_viewport[1] = y;
	s_viewport[2] = width;
	s_viewport[3] = height;
}

void getViewport(int* values)
{
	if (values != nullptr)
	{
		values[0] = s_viewport[0];
		values[1] = s_viewport[1];
		values[2] = s_viewport[2];
		values[3] = s_viewport[3];
	}
}

bool draw(const RenderInterleavedMesh& mesh, const GpuState& state)
{
	if (mesh.data == nullptr || mesh.count <= 0)
		return true; // Nothing to do, and the layout is representable.
	if (mesh.positionShort || mesh.stride != kVertexStride)
		return false; // The fixed five-attribute order covers exactly this.

	GPU_Primitive_t primitive = GPU_TRIANGLES;
	bool quads = false;
	switch (mesh.primitive)
	{
	case RenderPrimitive::Triangles:
		primitive = GPU_TRIANGLES;
		break;
	case RenderPrimitive::TriangleStrip:
		primitive = GPU_TRIANGLE_STRIP;
		break;
	case RenderPrimitive::TriangleFan:
		primitive = GPU_TRIANGLE_FAN;
		break;
	case RenderPrimitive::Quads:
		primitive = GPU_TRIANGLES; // Expanded below: the PICA has no quads.
		quads = true;
		break;
	default:
		// The PICA200 has no point or line rasteriser. The callers that use
		// them (the block-selection outline, debug geometry) already guard on
		// this returning false, the same answer the PS2/Wii would give for a
		// primitive they cannot express.
		return false;
	}

	const int quadCount = quads ? mesh.count / 4 : 0;
	if (quads && quadCount == 0)
		return true; // Fewer than four vertices cannot form even one quad.

	// Indexed quads: with the static index buffer the staged copy keeps the
	// quad's own 4 vertices and the GPU expands them with (0,1,2,0,2,3), so
	// the arena traffic and the colour fill below drop from 6 vertices per
	// quad to 4 -- and meshes replayed every frame (terrain display lists,
	// persistent model meshes, the HUD) stop re-paying the fan expansion on
	// every draw. The u16 indices hold because one draw can stage at most
	// kArenaVertices vertices.
	const bool indexedQuads = quads && s_quadIndices != nullptr;
	const int outCount = indexedQuads ? quadCount * 4 : (quads ? quadCount * 6 : mesh.count);

	frameBegin();
	if (s_target == nullptr || !s_inFrame)
		return false; // The renderer never came up; nothing can land.

	u8* staging = reserveArena(outCount);
	if (staging == nullptr)
		return false;

	const u8* src = static_cast<const u8*>(mesh.data) +
	                static_cast<std::size_t>(mesh.first) * kVertexStride;
	if (quads && !indexedQuads)
	{
		// The same fan split GL's GL_QUADS rasterises with -- (0,1,2) and
		// (0,2,3) -- so winding (and therefore culling) sees identical
		// faces to the desktop renderer.
		for (int q = 0; q < quadCount; ++q)
		{
			u8* out = staging + static_cast<std::size_t>(q) * 6 * kVertexStride;
			const u8* in = src + static_cast<std::size_t>(q) * 4 * kVertexStride;
			std::memcpy(out + 0 * kVertexStride, in + 0 * kVertexStride, kVertexStride);
			std::memcpy(out + 1 * kVertexStride, in + 1 * kVertexStride, kVertexStride);
			std::memcpy(out + 2 * kVertexStride, in + 2 * kVertexStride, kVertexStride);
			std::memcpy(out + 3 * kVertexStride, in + 0 * kVertexStride, kVertexStride);
			std::memcpy(out + 4 * kVertexStride, in + 2 * kVertexStride, kVertexStride);
			std::memcpy(out + 5 * kVertexStride, in + 3 * kVertexStride, kVertexStride);
		}
	}
	else
	{
		std::memcpy(staging, src, static_cast<std::size_t>(outCount) * kVertexStride);
	}

	// GL's current colour: vertices a mesh does not colour itself take the
	// one renderColor4f/3f last set -- the sky dome, the horizon band, the
	// sun/moon quads, the colourless GUI overlays. The tessellator leaves
	// those colour words unwritten (stale slots in the shared raw buffer)
	// and capture paths store white in them, so the register is applied
	// here, to the staged copy only, after the quad expansion so every
	// emitted vertex gets it. The packing matches the Tessellator's
	// little-endian layout: bytes R,G,B,A from the lowest address up.
	if (!mesh.hasColor)
	{
		const u32 packedColor =
		    (static_cast<u32>(clamp01(state.currentColor[3]) * 255.0f + 0.5f) << 24) |
		    (static_cast<u32>(clamp01(state.currentColor[2]) * 255.0f + 0.5f) << 16) |
		    (static_cast<u32>(clamp01(state.currentColor[1]) * 255.0f + 0.5f) << 8) |
		     static_cast<u32>(clamp01(state.currentColor[0]) * 255.0f + 0.5f);
		for (int i = 0; i < outCount; ++i)
			*reinterpret_cast<u32*>(staging + static_cast<std::size_t>(i) * kVertexStride + 20) =
			    packedColor;
	}

	splitCommandBufferIfNeeded();
	applyState(state, staging);
	if (indexedQuads)
		C3D_DrawElements(GPU_TRIANGLES, quadCount * 6, C3D_UNSIGNED_SHORT, s_quadIndices);
	else
		C3D_DrawArrays(primitive, 0, outCount);
	return true;
}

bool drawLinear(const RenderInterleavedMesh& mesh, const GpuState& state)
{
	// no per-vertex colours baked => the staged path's current-colour fill is
	// load-bearing for this mesh; it also copies out of a linear source fine.
	if (!mesh.hasColor)
		return draw(mesh, state);
	if (mesh.data == nullptr || mesh.count <= 0)
		return true;
	if (mesh.positionShort || mesh.stride != kVertexStride)
		return false;

	GPU_Primitive_t primitive = GPU_TRIANGLES;
	bool quads = false;
	switch (mesh.primitive)
	{
	case RenderPrimitive::Triangles:
		primitive = GPU_TRIANGLES;
		break;
	case RenderPrimitive::TriangleStrip:
		primitive = GPU_TRIANGLE_STRIP;
		break;
	case RenderPrimitive::TriangleFan:
		primitive = GPU_TRIANGLE_FAN;
		break;
	case RenderPrimitive::Quads:
		primitive = GPU_TRIANGLES;
		quads = true;
		break;
	default:
		return false;
	}
	const int quadCount = quads ? mesh.count / 4 : 0;
	if (quads && quadCount == 0)
		return true;
	// Same expansion math as draw(), minus the copy: quads index the data in
	// place, everything else submits it raw.
	const int outCount = quads ? quadCount * 4 : mesh.count;

	frameBegin();
	if (s_target == nullptr || !s_inFrame)
		return false;

	// Same split discipline as the staged path: the data here outlives the
	// whole command buffer by construction (the caller frees it only across
	// world loads), so a C3D_FrameSplit mid-frame is safe against it.
	splitCommandBufferIfNeeded();
	applyState(state, mesh.data);
	if (quads)
		C3D_DrawElements(GPU_TRIANGLES, quadCount * 6, C3D_UNSIGNED_SHORT, s_quadIndices);
	else
		C3D_DrawArrays(primitive, 0, outCount);
	return true;
}

void* allocLinear(std::size_t bytes)
{
	return linearAlloc(bytes);
}

void freeLinear(void* p)
{
	if (p != nullptr)
		linearFree(p);
}

} // namespace ds
