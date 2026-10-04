// RenderAPI_CTR_3DS.cpp — Nintendo 3DS render backend: the GL-shaped surface.
//
// This file is the contract half of the 3DS renderer; the PICA200 half lives
// in src/3ds/render/ (DsRender.cpp owns the citro3d context, frame lifecycle
// and draw path, DsMatrix.cpp the matrix stacks, DsTexture.cpp the texture
// storage, DsShader.v.pica the vertex shader). The split mirrors the Wii
// backend (RenderAPI_GX_WII.cpp over WiiNative*): the game keeps speaking
// OpenGL -- capabilities, blend factors, matrix modes, texture names -- and
// this layer holds that state and pushes it to the hardware at draw time.
//
// What is intentionally still narrow, with the hardware reason:
//
//   * Fog, lighting, colour-material and shade-model calls are accepted and
//     dropped. Minecraft bakes its lighting into vertex colours and the
//     brightness attribute on the Tessellator path, so the PICA needs neither
//     fixed-function lighting nor a fog unit for the GUI/milestone draw set;
//     world fog arrives with the terrain phase.
//   * Mipmaps, anisotropy and MSAA are reported unsupported, so RenderEngine
//     never builds mip chains that would be thrown away.
//   * The retained-mode surface is now real for display lists (see the
//     DisplayListEntry block): WorldRenderer::updateRenderer() records
//     sections into them and RenderGlobal replays them through RenderList,
//     both landing on the same ds::draw path as live geometry. Occlusion
//     queries stay stubs -- the desktop Fancy Occlusion path is the only
//     consumer and it is desktop GL.
//
// Geometry *capture* stays implemented here for real: it is platform-neutral
// CPU work, both PS2 and Wii do it identically, and returning false would push
// callers down untested failure paths for no gain.

#include "platform/RenderAPI.h"

#include <cstring>
#include <unordered_map>
#include <utility>

#include "3ds/render/DsMatrix.h"
#include "3ds/render/DsRender.h"
#include "3ds/render/DsTexture.h"

namespace
{
// Fixed top-screen geometry (see ClientPlatformPolicy_3DS / lwjgl::Display).
constexpr int kScreenW = 400;
constexpr int kScreenH = 240;

// Logical texture names. 0 is never handed out (GL convention: an uninitialised
// name is 0), so the counter starts at 1 and only moves forward.
int s_nextTextureName = 1;

// Same convention for display-list and occlusion-query names.
int s_nextDisplayList = 1;
int s_nextOcclusionQuery = 1;

// The GL-shaped fixed-function state, pushed to the PICA by every draw.
ds::GpuState s_state;

// glTexImage2D/glTexSubImage2D address the *bound* texture object, and
// renderTextureImageRgba/SubImage carry no id of their own -- so the name to
// upload into is remembered from the last bind / begin-upload, exactly the
// way the desktop backend relies on GL's binding instead.
int s_uploadTarget = 0;

// GL keeps one binding per texture unit; the PICA has one wired sampler.
// The game's lightmap path selects GL_TEXTURE1 (OpenGlHelper's
// lightmapTexUnit) and binds the 16x16 lightmap there -- with a no-op
// unit selector those binds would land on the only unit and clobber
// whatever the world is sampling from (the terrain atlas), leaving every
// captured section state replaying samples of the lightmap: a black
// world. So the selection is tracked: binds on any unit still retarget
// uploads (glTexImage2D addresses the active unit's binding), but only
// the default unit's binds change what the shader samples.
int s_activeTextureUnit = 0x84C0; // GL_TEXTURE0

// GL_TEXTURE_2D is enabled *per texture unit*, and the game's lightmap path
// depends on that: EntityRenderer::disableLightmap() does
//     setActiveTexture(GL_TEXTURE1); glDisable(GL_TEXTURE_2D);
// and enableLightmap() re-enables it there, intending only the lightmap
// sampler to switch off. On one global flag (which is how this backend read
// it until the world came back untextured) that call turns *unit 0* off as
// well, and nothing turns it back on: every chunk section recorded after a
// lightmap pair replays its captured state untextured -- vertex colours
// modulating nothing, i.e. flat white blocks -- and GuiMainMenu's
// drawPanorama, which deliberately issues no enable of its own (it assumes
// texturing is on the way every other textured draw in the game does),
// paints white over the menu background. The PS2 and Wii backends already
// keep this bit per unit for exactly this reason; see
// RenderAPI_GS_PS2.cpp and WiiNativeState's texture_enabled mask.
//
// Index 0 mirrors into s_state.texture2d, the only unit the PICA sampler
// reads. Index 1 is tracked for parity so a later lightmap TexEnv stage
// has its switch to consult.
bool s_texture2dByUnit[2] = { false, false };

// Which unit's bit a capability call touches. Only GL_TEXTURE1 (the
// lightmap unit) is anything other than the default, matching how
// RenderAPI_GS_PS2.cpp resolves the same question.
int trackedTextureUnitIndex()
{
	return s_activeTextureUnit == 0x84C1 ? 1 : 0;
}

// Retained-mode display lists. The PICA has no GPU-side list, so the mesh
// and fixed-function state reaching each list between Begin/End are
// captured on the CPU and replayed through the same ds::draw path as live
// geometry. The one GL semantic that needs care is the matrix: the game
// records a Push/Translate/Scale/Pop *inside* the list (WorldRenderer's
// section origin), which at call time composes onto the CALLER's camera
// matrix. Capturing it as a delta would need an inverse, so Begin pushes
// an identity onto the model-view stack instead: every matrix op recorded
// during the list builds the section transform directly, and the bake
// below multiplies it into the vertex positions. The replayed mesh is in
// section space, and the caller's live camera composes on top -- the same
// clip result as the GL list, without replaying any matrix at call time.
struct DisplayListEntry
{
	ds::GpuState state;      // full fixed-function state at capture
	RenderCapturedMesh mesh; // 32-byte vertices, positions pre-transformed

	// Sections replay many times per second and rewrite their bytes only on
	// a rebuild, so when the captured mesh carries per-vertex colours
	// (terrain does; colourless sky/HUD lists do not) the bytes move OUT of
	// the heap vector into a linear-heap buffer owned by the entry, and
	// ds::drawLinear submits them in place instead of re-copying them into
	// the frame arena on every replay. Grow-only: a rebuilt section reuses
	// the allocation when the new mesh fits (steady-state streaming stops
	// churning the linear heap), and a bigger mesh allocates BEFORE freeing
	// the old buffer so a failed growth keeps the previous mesh instead of
	// losing the section. Colours are required because the linear path
	// never patches vertex data mid-frame.
	std::uint8_t* linear = nullptr;
	std::size_t linearCapacity = 0;

	DisplayListEntry() = default;
	~DisplayListEntry() { releaseLinearStorage(); }
	DisplayListEntry(DisplayListEntry&& other) noexcept { *this = std::move(other); }
	DisplayListEntry& operator=(DisplayListEntry&& other) noexcept
	{
		if (this != &other)
		{
			releaseLinearStorage();
			state = other.state;
			mesh = std::move(other.mesh);
			linear = other.linear;
			linearCapacity = other.linearCapacity;
			other.linear = nullptr;
			other.linearCapacity = 0;
		}
		return *this;
	}
	DisplayListEntry(const DisplayListEntry&) = delete;
	DisplayListEntry& operator=(const DisplayListEntry&) = delete;

	void releaseLinearStorage() noexcept
	{
		if (linear != nullptr)
			ds::freeLinear(linear);
		linear = nullptr;
		linearCapacity = 0;
	}
};

std::unordered_map<int, std::vector<DisplayListEntry>> s_displayLists;
int s_recordingList = -1;
RenderMatrixMode s_recordingListMode = RenderMatrixMode::ModelView;
// Next entry slot the open capture fills. Reusing the slot (instead of
// clearing the list and pushing fresh entries) keeps each mesh vector's
// capacity across section rebuilds: streaming rebuilds up to
// PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME sections per frame, and the old
// clear()+resize() paid a free+malloc pair per rebuilt mesh -- the same
// fragmentation churn the PS2 backend documents in WorldRenderer.cpp. With
// the slots retained, a steady-state rebuild touches no allocator at all.
int s_recordingIndex = 0;

// Copy `mesh` into the open display list, baking the recording transform
// into every position. Returns false for the shapes the captured layout
// cannot hold (the live draw path rejects the same ones).
bool captureDisplayListMesh(const RenderInterleavedMesh& mesh)
{
	if (mesh.data == nullptr || mesh.count <= 0)
		return true; // Nothing to keep, and the layout is representable.
	if (mesh.positionShort || mesh.stride != 32)
		return false;

	std::vector<DisplayListEntry>& entries = s_displayLists[s_recordingList];
	DisplayListEntry& entry = s_recordingIndex < static_cast<int>(entries.size())
	    ? entries[s_recordingIndex] // retained slot, capacity kept
	    : entries.emplace_back();
	++s_recordingIndex;
	entry.state = s_state;
	entry.mesh.stride = 32;
	entry.mesh.primitive = mesh.primitive;
	entry.mesh.positionShort = false;
	entry.mesh.hasTexture = mesh.hasTexture;
	entry.mesh.texCoordOffset = 12;
	entry.mesh.hasColor = mesh.hasColor;
	entry.mesh.colorOffset = 20;
	entry.mesh.hasNormals = mesh.hasNormals;
	entry.mesh.normalOffset = 24;
	entry.mesh.hasBrightness = mesh.hasBrightness;
	entry.mesh.brightnessOffset = 28;
	entry.mesh.vertexCount = mesh.count;

	const float* transform = ds::matrix::modelViewTop();
	const std::int32_t* src = static_cast<const std::int32_t*>(mesh.data) +
	                          mesh.first * 8;
	const std::size_t byteCount = static_cast<std::size_t>(mesh.count) * 8u * sizeof(std::int32_t);

	// Linear-resident path (see the entry comment): per-vertex-coloured
	// meshes -- every terrain section -- leave the heap entirely. The copy
	// lands in the entry's own linear buffer and is never staged again.
	if (mesh.hasColor)
	{
		if (entry.linearCapacity < byteCount)
		{
			std::uint8_t* grown = static_cast<std::uint8_t*>(ds::allocLinear(byteCount));
			if (grown != nullptr)
			{
				entry.releaseLinearStorage();
				entry.linear = grown;
				entry.linearCapacity = byteCount;
			}
		}
		if (entry.linear != nullptr)
		{
			std::memcpy(entry.linear, src, byteCount);
			std::int32_t* dst = reinterpret_cast<std::int32_t*>(entry.linear);
			for (int i = 0; i < mesh.count; ++i)
			{
				// v' = M * (x, y, z, 1), column-major GL float[16].
				float* position = reinterpret_cast<float*>(dst + i * 8);
				const float x = position[0];
				const float y = position[1];
				const float z = position[2];
				position[0] = transform[0] * x + transform[4] * y + transform[8] * z +
				              transform[12];
				position[1] = transform[1] * x + transform[5] * y + transform[9] * z +
				              transform[13];
				position[2] = transform[2] * x + transform[6] * y + transform[10] * z +
				              transform[14];
			}
			// The heap vector stops being this entry's storage.
			std::vector<std::int32_t>().swap(entry.mesh.raw);
			return true;
		}
	}
	else
	{
		// Colourless meshes need the staged path's current-colour fill, so
		// they stay on the heap vector -- and any linear buffer from an
		// earlier coloured capture of this slot is dropped with the storage.
		entry.releaseLinearStorage();
	}

	entry.mesh.raw.resize(static_cast<std::size_t>(mesh.count) * 8u);

	std::int32_t* dst = entry.mesh.raw.data();
	std::memcpy(dst, src, entry.mesh.raw.size() * sizeof(std::int32_t));
	for (int i = 0; i < mesh.count; ++i)
	{
		// v' = M * (x, y, z, 1), column-major GL float[16].
		float* position = reinterpret_cast<float*>(dst + i * 8);
		const float x = position[0];
		const float y = position[1];
		const float z = position[2];
		position[0] = transform[0] * x + transform[4] * y + transform[8] * z +
		              transform[12];
		position[1] = transform[1] * x + transform[5] * y + transform[9] * z +
		              transform[13];
		position[2] = transform[2] * x + transform[6] * y + transform[10] * z +
		              transform[14];
	}
	// The entry was written in place: no push_back, and no allocation unless
	// this pass needs more slots than the previous one filled.
	return true;
}

// Replay a recorded list through the live draw path. The entry's captured
// state is used as-is, matching GL's list semantics (binds recorded inside
// the list replay with it); the caller's camera matrix stays whatever the
// caller set, because the section transform is baked into the positions.
void replayDisplayList(int displayList)
{
	const auto it = s_displayLists.find(displayList);
	if (it == s_displayLists.end())
		return;
	for (const DisplayListEntry& entry : it->second)
	{
		// Linear-resident entries carry their bytes outside mesh.raw -- the
		// vertex count is the liveness check for them (raw.empty() would
		// always be true).
		const bool linearResident = entry.linear != nullptr && entry.mesh.vertexCount > 0;
		if (!linearResident && entry.mesh.empty())
			continue;
		RenderInterleavedMesh view;
		view.data = linearResident ? static_cast<const void*>(entry.linear)
		                           : static_cast<const void*>(entry.mesh.raw.data());
		view.stride = 32;
		view.first = 0;
		view.count = entry.mesh.vertexCount;
		view.primitive = entry.mesh.primitive;
		view.positionShort = entry.mesh.positionShort;
		view.hasTexture = entry.mesh.hasTexture;
		view.texCoordOffset = entry.mesh.texCoordOffset;
		view.hasColor = entry.mesh.hasColor;
		view.colorOffset = entry.mesh.colorOffset;
		view.hasNormals = entry.mesh.hasNormals;
		view.normalOffset = entry.mesh.normalOffset;
		view.hasBrightness = entry.mesh.hasBrightness;
		view.brightnessOffset = entry.mesh.brightnessOffset;
		// A drawLinear() false would mean the layout is unrepresentable, but
		// the capture side already refuses exactly those meshes -- falling
		// through to the staged path would double the frame's work for no
		// gain, so only its colourless-mesh self-fallback matters.
		if (linearResident)
			ds::drawLinear(view, entry.state);
		else
			ds::draw(view, entry.state);
	}
}
} // namespace

// ---------------------------------------------------------------------------
// Geometry submission
// ---------------------------------------------------------------------------

bool renderDrawInterleaved(const RenderInterleavedMesh& mesh)
{
	if (s_recordingList >= 0)
		return captureDisplayListMesh(mesh);
	return ds::draw(mesh, s_state);
}

bool renderCaptureInterleaved(const RenderInterleavedMesh& mesh, RenderCapturedMesh& out, bool append)
{
	// Same fixed 32-byte Minecraft vertex layout the PS2 and Wii backends
	// capture (position=f32x3, texcoord=f32x2, color=u32, normal=s8x3,
	// brightness=f32). See RenderInterleavedMesh in RenderAPI.h.
	if (mesh.data == nullptr || mesh.stride != 32 || mesh.count <= 0)
		return false;
	if (!append)
		out.clear();
	if (!out.empty() && out.stride != 32)
		return false;
	if (out.empty())
	{
		out.stride = 32;
		out.primitive = mesh.primitive;
		out.positionShort = mesh.positionShort;
		out.hasTexture = mesh.hasTexture;
		out.texCoordOffset = 12;
		out.hasColor = mesh.hasColor;
		out.colorOffset = 20;
		out.hasNormals = mesh.hasNormals;
		out.normalOffset = 24;
		out.hasBrightness = mesh.hasBrightness;
		out.brightnessOffset = 28;
	}
	else
	{
		out.hasTexture = out.hasTexture || mesh.hasTexture;
		out.hasColor = out.hasColor || mesh.hasColor;
		out.hasNormals = out.hasNormals || mesh.hasNormals;
		out.hasBrightness = out.hasBrightness || mesh.hasBrightness;
	}

	const std::int32_t* src = static_cast<const std::int32_t*>(mesh.data) + mesh.first * 8;
	const std::size_t base = out.raw.size();
	out.raw.insert(out.raw.end(), src, src + static_cast<std::size_t>(mesh.count) * 8u);
	for (int v = 0; v < mesh.count; ++v)
	{
		std::int32_t* dst = out.raw.data() + base + static_cast<std::size_t>(v) * 8u;
		if (!mesh.hasTexture) { dst[3] = 0; dst[4] = 0; }
		if (!mesh.hasColor) dst[5] = static_cast<std::int32_t>(0xFFFFFFFFu);
		if (!mesh.hasNormals) dst[6] = 0;
		if (!mesh.hasBrightness) dst[7] = 0;
	}
	out.vertexCount += mesh.count;
	return true;
}

bool renderDrawCaptured(const RenderCapturedMesh& mesh)
{
	if (mesh.empty() || mesh.stride != 32)
		return false;
	if (s_recordingList >= 0)
	{
		// Captured sub-meshes reach the list through the extra-texture
		// groups WorldRenderer records inside it; capture them as further
		// list entries with their own (already bound) state.
		RenderInterleavedMesh view;
		view.data = mesh.raw.data();
		view.stride = 32;
		view.first = 0;
		view.count = mesh.vertexCount;
		view.primitive = mesh.primitive;
		view.positionShort = mesh.positionShort;
		view.hasTexture = mesh.hasTexture;
		view.texCoordOffset = mesh.texCoordOffset;
		view.hasColor = mesh.hasColor;
		view.colorOffset = mesh.colorOffset;
		view.hasNormals = mesh.hasNormals;
		view.normalOffset = mesh.normalOffset;
		view.hasBrightness = mesh.hasBrightness;
		view.brightnessOffset = mesh.brightnessOffset;
		return captureDisplayListMesh(view);
	}
	// The captured blob is the same interleaved layout the live path takes;
	// hand the draw to the same entry point and the state flush works for
	// both (GuiIngame's HUD mesh cache and anything else that replays).
	RenderInterleavedMesh view;
	view.data = mesh.raw.data();
	view.stride = 32;
	view.first = 0;
	view.count = mesh.vertexCount;
	view.primitive = mesh.primitive;
	view.positionShort = mesh.positionShort;
	view.hasTexture = mesh.hasTexture;
	view.texCoordOffset = mesh.texCoordOffset;
	view.hasColor = mesh.hasColor;
	view.colorOffset = mesh.colorOffset;
	view.hasNormals = mesh.hasNormals;
	view.normalOffset = mesh.normalOffset;
	view.hasBrightness = mesh.hasBrightness;
	view.brightnessOffset = mesh.brightnessOffset;
	return ds::draw(view, s_state);
}

// ---------------------------------------------------------------------------
// Persistent meshes (model geometry)
// ---------------------------------------------------------------------------
// The PICA consumes the staging arena per draw, so there is no native
// retained-geometry object here either: a "persistent mesh" is a captured
// RAM mesh held per handle and replayed through renderDrawCaptured, exactly
// the PS2's Ps2ModelGeometryCache contract. The win is the per-frame
// tessellation of model boxes -- a ModelRenderer part is invariant geometry
// (positions bake the compile-time scale); the matrix stack animates it and
// the live state supplies texture/tint/lighting at replay, so replay must
// NOT bake the recording transform the way the section display lists do.

// Handle table for the persistent-mesh API. Replay goes through
// renderDrawCaptured under the LIVE state (unlike the display lists, which
// snapshot state at record time), because ModelRenderer animates through the
// matrix stack every frame.
static std::unordered_map<int, RenderCapturedMesh> s_persistentMeshes;

int renderCreatePersistentMesh()
{
	static int s_nextPersistentMesh = 1;
	const int handle = s_nextPersistentMesh++;
	s_persistentMeshes[handle];
	return handle;
}

void renderDestroyPersistentMesh(int handle)
{
	s_persistentMeshes.erase(handle);
}

bool renderCompilePersistentMesh(int handle, const RenderInterleavedMesh& mesh)
{
	const auto it = s_persistentMeshes.find(handle);
	if (it == s_persistentMeshes.end())
		return false;
	return renderCaptureInterleaved(mesh, it->second, false);
}

bool renderDrawPersistentMesh(int handle)
{
	const auto it = s_persistentMeshes.find(handle);
	if (it == s_persistentMeshes.end() || it->second.empty())
		return false;
	return renderDrawCaptured(it->second);
}

// ---------------------------------------------------------------------------
// Fixed-function state
// ---------------------------------------------------------------------------


void renderEnable(RenderCapability capability)
{
	switch (capability)
	{
	case RenderCapability::Texture2D:
		s_texture2dByUnit[trackedTextureUnitIndex()] = true;
		// Only unit 0 feeds the sampler; a lightmap-unit enable must not
		// stand in for it (and must not be discarded either -- see the
		// s_texture2dByUnit note).
		s_state.texture2d = s_texture2dByUnit[0];
		break;
	case RenderCapability::Blend:     s_state.blend = true;     break;
	case RenderCapability::DepthTest: s_state.depthTest = true; break;
	case RenderCapability::AlphaTest: s_state.alphaTest = true; break;
	case RenderCapability::CullFace:  s_state.cullFace = true;  break;
	default:
		// Lighting/normalize/fog/polygon-offset have no PICA path behind this
		// surface yet; the lighting the world shows arrives through vertex
		// colours and the brightness attribute.
		break;
	}
}

void renderDisable(RenderCapability capability)
{
	switch (capability)
	{
	case RenderCapability::Texture2D:
		// Per unit: this is the call EntityRenderer::disableLightmap makes
		// while GL_TEXTURE1 is active, and it has to leave unit 0 sampling
		// the terrain atlas.
		s_texture2dByUnit[trackedTextureUnitIndex()] = false;
		s_state.texture2d = s_texture2dByUnit[0];
		break;
	case RenderCapability::Blend:     s_state.blend = false;     break;
	case RenderCapability::DepthTest: s_state.depthTest = false; break;
	case RenderCapability::AlphaTest: s_state.alphaTest = false; break;
	case RenderCapability::CullFace:  s_state.cullFace = false;  break;
	default: break;
	}
}

void renderBlendFunc(RenderBlendFactor source, RenderBlendFactor destination)
{
	s_state.blendSrc = source;
	s_state.blendDst = destination;
}

void renderDepthMask(bool enabled)      { s_state.depthWrite = enabled; }
void renderDepthFunc(RenderCompare function) { s_state.depthFunc = function; }
void renderAlphaFunc(RenderCompare function, float reference)
{
	s_state.alphaFunc = function;
	s_state.alphaRef = reference;
}
void renderCullFace(RenderFace face)    { s_state.cullFaceMode = face; }

void renderColorMask(bool red, bool green, bool blue, bool alpha)
{
	s_state.colorWriteR = red;
	s_state.colorWriteG = green;
	s_state.colorWriteB = blue;
	s_state.colorWriteA = alpha;
}

void renderBindTexture(int texture)
{
	// Upload addressing follows the active unit's binding, whatever unit
	// that is; only the default unit's binds feed the sampler the shader
	// reads (see s_activeTextureUnit).
	s_uploadTarget = texture;
	if (s_activeTextureUnit == 0x84C0)
		s_state.boundTexture = texture;
}

void renderSetActiveTextureUnit(int textureUnit)
{
	// GL_TEXTURE0 (0x84C0) is the only unit wired to the PICA sampler.
	// Other units' binds exist for upload addressing only -- see
	// renderBindTexture and the s_activeTextureUnit note above.
	s_activeTextureUnit = textureUnit;
}

void renderSetClientActiveTextureUnit(int textureUnit) { (void)textureUnit; }
void renderSetMultiTextureCoord(int textureUnit, float u, float v)
{
	// Lightmap coordinates travel in the mesh (brightness attribute), so
	// there is no second texcoord channel to feed here.
	(void)textureUnit; (void)u; (void)v;
}

void renderSetLightmapColors(const std::uint32_t* colors, int count)
{
	// The 16-entry lightmap colour table the PS2/Wii emulate in hardware: the
	// citro3d milestone modulates through TexEnv instead, fed by the vertex
	// brightness attribute, so the table has nothing to drive yet.
	(void)colors; (void)count;
}

// GL's current-colour register. The tessellator bakes a colour into vertex
// data when one was set between begin/end, but a draw whose vertices carry
// none (the sky dome, the horizon band, the sun/moon quads) takes the last
// colour set through this path -- that is the whole of GL's fixed-function
// current colour, and ds::draw writes it into the staged colour words of
// every hasColor == false mesh. PS2/Wii keep the same register (see
// ps2_render_color4f and WiiNativeState's current_color); until this was
// wired the 3DS dropped it, and those draws came out with whatever the
// capture path had parked in the colour words instead of the sky colour
// the game had set with renderColor3f.
void renderColor4f(float r, float g, float b, float a)
{
	s_state.currentColor[0] = r;
	s_state.currentColor[1] = g;
	s_state.currentColor[2] = b;
	s_state.currentColor[3] = a;
}

void renderColor3f(float r, float g, float b)
{
	renderColor4f(r, g, b, 1.0f);
}

void renderNormal3f(float, float, float) {}
// Normals still have no backend register to keep: the Tessellator bakes them
// into vertex data and the PICA milestone applies lighting through vertex
// colours, so there is nothing on the hardware side to feed (the fixed
// attribute slot stays loaded and unread).

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------

void renderGenerateTextures(int count, int* textures)
{
	if (count <= 0 || textures == nullptr)
		return;
	for (int i = 0; i < count; ++i)
		textures[i] = s_nextTextureName++;
}

void renderDeleteTextures(int count, const int* textures)
{
	if (count <= 0 || textures == nullptr)
		return;
	for (int i = 0; i < count; ++i)
	{
		ds::texture::destroy(textures[i]);
		if (textures[i] == s_uploadTarget)
			s_uploadTarget = 0;
	}
}

void renderTextureSubImageRgba(int level, int x, int y, int width, int height, const void* pixels)
{
	// Level 0 only: no mipmap support is reported, so nothing else is asked
	// for -- animated textures (water/lava) are sub-rects of the base image.
	if (level != 0)
		return;
	ds::texture::upload(s_uploadTarget, x, y, width, height, pixels);
}

void renderTextureImageRgba(int level, int width, int height, const void* pixels)
{
	if (level != 0)
		return;
	ds::texture::upload(s_uploadTarget, 0, 0, width, height, pixels);
}

void renderTextureParameters(bool blur, bool mipmaps, bool clamp)
{
	(void)mipmaps; // No mip chain exists to filter with.
	ds::texture::setParameters(s_uploadTarget, blur, clamp);
}

int renderGetMaxAnisotropy() { return 1; } // no anisotropic filtering: factor 1 is "off"
int renderGetMaxSamples() { return 0; }    // no MSAA through this surface yet

bool renderTextureBeginUpload(int texture, int width, int height, int maxLevel,
                              bool blur, bool clamp, bool tileAtlas, bool highPrecision)
{
	// tileAtlas/highPrecision select PC texture formats (compressed atlases,
	// float precision); every PICA200 texture here is RGBA8, so both are
	// accepted and ignored. Mip levels beyond 0 are dropped with the same
	// reasoning as the level guard on the upload calls. Failure here means
	// the image genuinely cannot be held (over the PICA's 1024-texel limit)
	// and RenderEngine records the resource as failed -- the honest answer,
	// now that there is real storage behind a "yes".
	(void)maxLevel; (void)tileAtlas; (void)highPrecision;
	s_uploadTarget = texture;
	if (!ds::texture::allocate(texture, width, height))
		return false;
	ds::texture::setParameters(texture, blur, clamp);
	return true;
}

bool renderTextureIsValid(int texture)
{
	// GL hands "true" for any bound name; the PS2 checks its VRAM records.
	// Storage is what actually matters to the callers (RenderEngine's
	// post-upload check, the legacy UI caches) -- they all ask *after* an
	// upload attempt, never between generate and first upload.
	return texture > 0 && texture < s_nextTextureName && ds::texture::exists(texture);
}

void renderResetResources()
{
	s_nextTextureName = 1;
	s_nextDisplayList = 1;
	s_nextOcclusionQuery = 1;
	s_uploadTarget = 0;
	s_activeTextureUnit = 0x84C0;
	s_recordingList = -1;
	s_displayLists.clear();
	ds::texture::resetAll();
	ds::matrix::resetAll();
}

// ---------------------------------------------------------------------------
// Fog and lighting
// ---------------------------------------------------------------------------

void renderFogf(RenderFogParameter, float) {}
void renderFogi(RenderFogParameter, RenderFogMode) {}
void renderFogColor(const float*) {}
void renderLightfv(int, RenderLightParameter, const float*) {}
void renderLightModelAmbient(const float*) {}
void renderColorMaterial(RenderFace, RenderColorMaterialMode) {}
void renderShadeModel(RenderShadeModel) {}
// All accepted and dropped: see the file header. World fog joins with the
// terrain milestone (a PICA fog unit exists -- C3D_Fog -- but nothing on the
// GUI/HUD path queries it, so wiring it before there is geometry to fog
// would be untestable either way).

// ---------------------------------------------------------------------------
// Frame control
// ---------------------------------------------------------------------------

void renderClear(unsigned int mask)
{
	// In this port a colour clear is also where a frame starts -- see the
	// frame lifecycle note in DsRender.cpp.
	ds::clear(mask);
}

void renderFinishGpu()
{
	// Deliberately not a C3D_FrameSync: RenderGlobal calls this every frame
	// when "smooth FPS" is enabled, and the queue already drains at
	// C3D_FrameEnd -- the next C3D_FrameBegin(C3D_FRAME_SYNCDRAW) is the
	// synchronisation point that guarantees the wait. Anything stronger here
	// would cost a whole extra VBlank per frame on that setting.
}

void renderSubmitFrame()
{
	ds::submitFrame();
}

void renderClearColor(float r, float g, float b, float a)
{
	ds::setClearColor(r, g, b, a);
}

void renderClearDepth(double depth)
{
	ds::setClearDepth(depth);
}

void renderPolygonOffset(float, float) {} // no polygon offset through citro3d yet
void renderLineWidth(float) {}            // no line rasteriser to give a width to

void renderViewport(int x, int y, int width, int height)
{
	ds::setViewport(x, y, width, height);
}

void renderGetViewport(int* values)
{
	ds::getViewport(values);
}

// ---------------------------------------------------------------------------
// Bottom-screen panel pass (see the bottom-panel section of DsRender.cpp)
// ---------------------------------------------------------------------------

namespace
{
// Pairs renderBottomPanelBegin()'s matrix push with End's pop, so End is a
// no-op when Begin refused -- a caller may only run the two as a pair, but
// there is nothing to unwind when nothing was pushed.
bool s_bottomPass = false;
}

bool renderBottomPanelBegin()
{
	if (!ds::keyboardBottomBegin())
		return false;
	s_bottomPass = true;
	// The same projection pair Minecraft.cpp lays down for the top screen's
	// GUI, sized for this panel's own 320x240 canvas -- pushed rather than
	// overwritten so the caller's matrices survive the pass untouched.
	ds::matrix::setMode(RenderMatrixMode::Projection);
	ds::matrix::push();
	ds::matrix::loadIdentity();
	ds::matrix::ortho(0.0, 320.0, 240.0, 0.0, 1000.0, 3000.0);
	ds::matrix::setMode(RenderMatrixMode::ModelView);
	ds::matrix::push();
	ds::matrix::loadIdentity();
	ds::matrix::translate(0.0f, 0.0f, -2000.0f);
	return true;
}

void renderBottomPanelEnd()
{
	if (!s_bottomPass)
		return;
	s_bottomPass = false;
	ds::matrix::setMode(RenderMatrixMode::ModelView);
	ds::matrix::pop();
	ds::matrix::setMode(RenderMatrixMode::Projection);
	ds::matrix::pop();
	ds::matrix::setMode(RenderMatrixMode::ModelView);
	ds::keyboardBottomEnd();
}

// The on-screen keyboard's original names: same pair, same contract.
bool renderKeyboardBottomBegin()
{
	return renderBottomPanelBegin();
}

void renderKeyboardBottomEnd()
{
	renderBottomPanelEnd();
}

bool renderBottomPanelOwned()
{
	return ds::bottomPanelOwned();
}

void renderGetMatrix(RenderMatrixQuery query, float* values)
{
	// The stacks stay GL-native on purpose: ActiveRenderInfo un-projects
	// with these numbers and expects desktop-GL matrices, tilt or no tilt.
	ds::matrix::get(query, values);
}

const unsigned char* renderGetString(RenderStringQuery)
{
	return reinterpret_cast<const unsigned char*>("OptiCraft 3DS (citro3d)");
}

bool renderSupportsFeature(RenderFeature feature)
{
	// Honest answers, same as phase 1: no mipmaps, no anisotropy, no MSAA,
	// no occlusion query through citro3d's surface. Fancy fog distance is a
	// shader feature of the desktop renderer; the PICA fog unit cannot
	// express per-fragment radial fog the way that setting expects.
	(void)feature;
	return false;
}

unsigned int renderGetError() { return 0; }

void renderFogHint(RenderHintMode) {}

// ---------------------------------------------------------------------------
// Matrix stack
// ---------------------------------------------------------------------------

void renderMatrixMode(RenderMatrixMode mode)   { ds::matrix::setMode(mode); }
void renderLoadIdentity()                      { ds::matrix::loadIdentity(); }
void renderPushMatrix()                        { ds::matrix::push(); }
void renderPopMatrix()                         { ds::matrix::pop(); }
void renderTranslate(float x, float y, float z){ ds::matrix::translate(x, y, z); }
void renderRotate(float angle, float x, float y, float z)
{
	ds::matrix::rotate(angle, x, y, z);
}
void renderScale(float x, float y, float z)    { ds::matrix::scale(x, y, z); }
void renderScaleDouble(double x, double y, double z)
{
	ds::matrix::scale(static_cast<float>(x), static_cast<float>(y),
	                  static_cast<float>(z));
}
void renderFrustum(double left, double right, double bottom, double top,
                   double nearValue, double farValue)
{
	ds::matrix::frustum(left, right, bottom, top, nearValue, farValue);
}
void renderOrtho(double left, double right, double bottom, double top,
                 double nearValue, double farValue)
{
	ds::matrix::ortho(left, right, bottom, top, nearValue, farValue);
}

// ---------------------------------------------------------------------------
// Retained-mode compatibility (display lists + occlusion queries)
// ---------------------------------------------------------------------------
//
//   * The retained-mode surface (display lists) is real: each list's mesh,
//     state and recorded transform are captured on the CPU between
//     Begin/End and replayed through the same ds::draw path as live
//     geometry (see the DisplayListEntry block). Occlusion queries remain
//     stubs: renderBeginOcclusionQuery records nothing and a result always
//     reads "finished, zero pixels passed" -- the same answer as "nothing
//     was drawn", which is all the desktop Fancy Occlusion path would ever
//     consult them for.

int renderGenerateDisplayLists(int count)
{
	if (count <= 0)
		return 0;
	const int first = s_nextDisplayList;
	s_nextDisplayList += count;
	return first;
}

void renderDeleteDisplayLists(int first, int count)
{
	// Drop the storage behind the names being reclaimed, then reclaim the
	// names themselves the way GL's own reuse works.
	for (int i = 0; i < count; ++i)
		s_displayLists.erase(first + i);
	if (count > 0 && first > 0 && first + count == s_nextDisplayList)
		s_nextDisplayList = first;
}

void renderBeginDisplayList(int displayList)
{
	if (s_recordingList >= 0)
		return; // GL would nest; nothing on this game's paths asks for it.
	// A Begin always redefines the list, matching GL's semantics -- but the
	// entry slots and their mesh capacity stay: the capture below overwrites
	// the slots in place, so a re-recorded section does not churn the heap
	// (see captureDisplayListMesh). Slots this pass does not fill are marked
	// empty here and replay skips them.
	for (DisplayListEntry& entry : s_displayLists[displayList])
		entry.mesh.clear();
	s_recordingIndex = 0;
	s_recordingList = displayList;
	s_recordingListMode = ds::matrix::currentMode();
	// The identity is the base the list's own matrix ops build on; the bake
	// in captureDisplayListMesh turns the result into vertex positions, and
	// pop() below restores the caller's model-view untouched.
	ds::matrix::setMode(RenderMatrixMode::ModelView);
	ds::matrix::push();
	ds::matrix::loadIdentity();
}

void renderEndDisplayList()
{
	if (s_recordingList < 0)
		return;
	ds::matrix::setMode(RenderMatrixMode::ModelView);
	ds::matrix::pop();
	ds::matrix::setMode(s_recordingListMode);
	s_recordingList = -1;
}

void renderCallDisplayList(int displayList)
{
	replayDisplayList(displayList);
}

void renderCallDisplayLists(int count, const int* displayLists)
{
	// RenderList's terrain batch: one replay call per recorded section id.
	if (count <= 0 || displayLists == nullptr)
		return;
	for (int i = 0; i < count; ++i)
		replayDisplayList(displayLists[i]);
}

void renderGenerateOcclusionQueries(int count, int* queries)
{
	if (count <= 0 || queries == nullptr)
		return;
	for (int i = 0; i < count; ++i)
		queries[i] = s_nextOcclusionQuery++;
}

void renderBeginOcclusionQuery(int) {}
void renderEndOcclusionQuery() {}
bool renderOcclusionQueryResultAvailable(int) { return true; }
unsigned int renderOcclusionQueryResult(int) { return 0; }

// ---------------------------------------------------------------------------
// Presentation helpers
// ---------------------------------------------------------------------------

bool renderCopyFramebufferToBoundTexture(int, int, int, int)
{
	// No framebuffer readback through citro3d's surface yet; the same
	// fallback the PS2 and Wii take, so GuiMainMenu and
	// LegacyColorGradePass render directly.
	return false;
}

void renderSetLegacyPresentationGamma(bool) {}
