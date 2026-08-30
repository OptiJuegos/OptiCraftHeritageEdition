#include "platform/RenderAPI.h"


#include "wii/gx_wii.h"


#include "wii/render/WiiNativeStateSnapshot.h"
#include "wii/render/WiiNativeDraw.h"
#include "wii/render/WiiNativeTexture.h"
#include "wii/render/WiiRenderTypes.h"
#include <cstring>


void renderEnable(RenderCapability capability)
{
    switch (capability)
    {
        case RenderCapability::Texture2D: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::Texture2D)); break;
        case RenderCapability::ColorMaterial: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::ColorMaterial)); break;
        case RenderCapability::CullFace: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::CullFace)); break;
        case RenderCapability::AlphaTest: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::AlphaTest)); break;
        case RenderCapability::Blend: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::Blend)); break;
        case RenderCapability::DepthTest: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::DepthTest)); break;
        case RenderCapability::Fog: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::Fog)); break;
        case RenderCapability::Lighting: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::Lighting)); break;
        case RenderCapability::Normalize: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::Normalize)); break;
        case RenderCapability::RescaleNormal: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::RescaleNormal)); break;
        case RenderCapability::Light0: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::Light0)); break;
        case RenderCapability::Light1: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::Light1)); break;
        case RenderCapability::PolygonOffsetFill: wii_gx_native_enable(wiiRenderValue(WiiRenderCapability::PolygonOffsetFill)); break;
    }
}

void renderDisable(RenderCapability capability)
{
    switch (capability)
    {
        case RenderCapability::Texture2D: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::Texture2D)); break;
        case RenderCapability::ColorMaterial: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::ColorMaterial)); break;
        case RenderCapability::CullFace: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::CullFace)); break;
        case RenderCapability::AlphaTest: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::AlphaTest)); break;
        case RenderCapability::Blend: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::Blend)); break;
        case RenderCapability::DepthTest: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::DepthTest)); break;
        case RenderCapability::Fog: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::Fog)); break;
        case RenderCapability::Lighting: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::Lighting)); break;
        case RenderCapability::Normalize: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::Normalize)); break;
        case RenderCapability::RescaleNormal: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::RescaleNormal)); break;
        case RenderCapability::Light0: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::Light0)); break;
        case RenderCapability::Light1: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::Light1)); break;
        case RenderCapability::PolygonOffsetFill: wii_gx_native_disable(wiiRenderValue(WiiRenderCapability::PolygonOffsetFill)); break;
    }
}

void renderBlendFunc(RenderBlendFactor source, RenderBlendFactor destination)
{
    auto nativeFactor = [](RenderBlendFactor factor) -> WiiRenderBlendFactor
    {
        switch (factor)
        {
            case RenderBlendFactor::Zero: return WiiRenderBlendFactor::Zero;
            case RenderBlendFactor::One: return WiiRenderBlendFactor::One;
            case RenderBlendFactor::SrcColor: return WiiRenderBlendFactor::SrcColor;
            case RenderBlendFactor::OneMinusSrcColor: return WiiRenderBlendFactor::OneMinusSrcColor;
            case RenderBlendFactor::SrcAlpha: return WiiRenderBlendFactor::SrcAlpha;
            case RenderBlendFactor::OneMinusSrcAlpha: return WiiRenderBlendFactor::OneMinusSrcAlpha;
            case RenderBlendFactor::DstAlpha: return WiiRenderBlendFactor::DstAlpha;
            case RenderBlendFactor::OneMinusDstAlpha: return WiiRenderBlendFactor::OneMinusDstAlpha;
            case RenderBlendFactor::DstColor: return WiiRenderBlendFactor::DstColor;
            case RenderBlendFactor::OneMinusDstColor: return WiiRenderBlendFactor::OneMinusDstColor;
        }
        return WiiRenderBlendFactor::One;
    };
    wii_gx_native_blend_func(wiiRenderValue(nativeFactor(source)), wiiRenderValue(nativeFactor(destination)));
}

void renderDepthMask(bool enabled)
{
    wii_gx_native_depth_mask(enabled ? 1 : 0);
}

void renderDepthFunc(RenderCompare function)
{
    auto nativeCompare = [](RenderCompare compare) -> WiiRenderCompare
    {
        switch (compare)
        {
            case RenderCompare::Never: return WiiRenderCompare::Never;
            case RenderCompare::Less: return WiiRenderCompare::Less;
            case RenderCompare::Equal: return WiiRenderCompare::Equal;
            case RenderCompare::LessEqual: return WiiRenderCompare::LessEqual;
            case RenderCompare::Greater: return WiiRenderCompare::Greater;
            case RenderCompare::NotEqual: return WiiRenderCompare::NotEqual;
            case RenderCompare::GreaterEqual: return WiiRenderCompare::GreaterEqual;
            case RenderCompare::Always: return WiiRenderCompare::Always;
        }
        return WiiRenderCompare::Always;
    };
    wii_gx_native_depth_func(wiiRenderValue(nativeCompare(function)));
}

void renderAlphaFunc(RenderCompare function, float reference)
{
    auto nativeCompare = [](RenderCompare compare) -> WiiRenderCompare
    {
        switch (compare)
        {
            case RenderCompare::Never: return WiiRenderCompare::Never;
            case RenderCompare::Less: return WiiRenderCompare::Less;
            case RenderCompare::Equal: return WiiRenderCompare::Equal;
            case RenderCompare::LessEqual: return WiiRenderCompare::LessEqual;
            case RenderCompare::Greater: return WiiRenderCompare::Greater;
            case RenderCompare::NotEqual: return WiiRenderCompare::NotEqual;
            case RenderCompare::GreaterEqual: return WiiRenderCompare::GreaterEqual;
            case RenderCompare::Always: return WiiRenderCompare::Always;
        }
        return WiiRenderCompare::Always;
    };
    wii_gx_native_alpha_func(wiiRenderValue(nativeCompare(function)), reference);
}

void renderCullFace(RenderFace face)
{
    switch (face)
    {
        case RenderFace::Front: wii_gx_native_cull_face(wiiRenderValue(WiiRenderFace::Front)); break;
        case RenderFace::Back: wii_gx_native_cull_face(wiiRenderValue(WiiRenderFace::Back)); break;
        case RenderFace::FrontAndBack: wii_gx_native_cull_face(wiiRenderValue(WiiRenderFace::FrontAndBack)); break;
    }
}

void renderColorMask(bool red, bool green, bool blue, bool alpha)
{
    wii_gx_native_color_mask(red ? 1 : 0, green ? 1 : 0, blue ? 1 : 0, alpha ? 1 : 0);
}

void renderBindTexture(int texture)
{
    wii_native_texture_bind(texture);
}

void renderSetActiveTextureUnit(int textureUnit)
{
    const int unit = textureUnit == 0x84C1 ? 1 : 0;
    wii_gx_native_set_active_texture_unit(unit);
}

void renderSetClientActiveTextureUnit(int textureUnit)
{
    // Tessellator carries the secondary lightmap coordinate explicitly in the
    // interleaved brightness word, so no separate GX client-array state is
    // required here. Keep accepting the OpenGL texture-unit enum for API parity.
    (void)textureUnit;
}

void renderSetMultiTextureCoord(int textureUnit, float u, float v)
{
    const int unit = textureUnit == 0x84C1 ? 1 : 0;
    wii_gx_native_set_multi_texture_coord(unit, u, v);
}

void renderSetLightmapColors(const std::uint32_t* colors, int count)
{
    (void)colors;
    (void)count;
}

void renderColor4f(float r, float g, float b, float a)
{
    wii_gx_native_color4f(r, g, b, a);
}

void renderColor3f(float r, float g, float b)
{
    wii_gx_native_color3f(r, g, b);
}

void renderNormal3f(float x, float y, float z)
{
    wii_gx_native_normal3f(x, y, z);
}

void renderGenerateTextures(int count, int *textures)
{
    if (count <= 0 || textures == nullptr)
        return;
    wii_native_texture_generate_names(count, textures);
}

void renderDeleteTextures(int count, const int *textures)
{
    if (count <= 0 || textures == nullptr)
        return;
    wii_native_texture_delete_names(count, textures);
}

void renderTextureSubImageRgba(int level, int x, int y, int width, int height, const void *pixels)
{
    int texture = -1;
    GXTexObj object;
    if (wii_native_texture_get_current(&texture, &object))
        wii_native_texture_upload_sub_rgba(texture, level, x, y, width, height,
                                           static_cast<const unsigned char *>(pixels));
}

// Specifies one mip level of the bound texture from tightly packed RGBA8.
//
// The desktop internal format stays GL_RGBA -- the third glTexImage2D argument
// is what decides the uploaded cost, and a desktop driver stores that as 32 bits
// per texel. The console backends pick their own storage (PS2: CT16, or PSMT8
// plus a CT16 CLUT when PS2_ENABLE_PSMT8 is on), which is why they take the
// pixels rather than a format enum.
void renderTextureImageRgba(int level, int width, int height, const void *pixels)
{
    // wii_native_texture_begin_upload must have run first; RenderEngine still
    // calls it explicitly because only it knows the mip count up front.
    int texture = -1;
    GXTexObj object;
    if (wii_native_texture_get_current(&texture, &object))
        wii_native_texture_upload_level_rgba(texture, level, width, height,
                                             static_cast<const unsigned char *>(pixels));
}

//
// The Wii stores filtering/wrap inside GXTexObj. Most textures receive these
// values during upload; the live update here is required by Minecraft 1.2.5's
// lightmap, which switches an already-created 16x16 texture to linear+clamp.
// PS2 resolves wrap per primitive from UV bounds instead of per texture.
void renderTextureParameters(bool blur, bool mipmaps, bool clamp)
{
    (void)mipmaps;
    int texture = -1;
    GXTexObj object;
    if (wii_native_texture_get_current(&texture, &object))
        wii_native_texture_set_parameters(texture, blur, clamp);
}

void renderApplyTextureQuality(bool, int, bool mipmapLinear, int anisotropy)
{
    int texture = -1;
    GXTexObj object;
    if (wii_native_texture_get_current(&texture, &object))
        wii_native_texture_set_quality(texture, mipmapLinear, anisotropy);
}

int renderGetMaxAnisotropy() { return 4; }
int renderGetMaxSamples() { return 0; }

bool renderTextureBeginUpload(int texture, int width, int height, int maxLevel, bool blur, bool clamp, bool)
{
    renderBindTexture(texture);
    return wii_native_texture_begin_upload(texture, width, height, maxLevel, blur, clamp);
}
bool renderTextureIsValid(int texture)
{
    if (texture < 0)
        return false;
    GXTexObj object;
    return wii_native_texture_get_texobj_c(texture, &object) != 0;
}
void renderResetResources() { wii_native_texture_clear_all(); }


void renderFogf(RenderFogParameter parameter, float value)
{
    WiiRenderFogParameter nativeParameter = WiiRenderFogParameter::Density;
    switch (parameter)
    {
        case RenderFogParameter::Density: nativeParameter = WiiRenderFogParameter::Density; break;
        case RenderFogParameter::Start: nativeParameter = WiiRenderFogParameter::Start; break;
        case RenderFogParameter::End: nativeParameter = WiiRenderFogParameter::End; break;
        case RenderFogParameter::Mode: nativeParameter = WiiRenderFogParameter::Mode; break;
        case RenderFogParameter::Color: nativeParameter = WiiRenderFogParameter::Color; break;
        case RenderFogParameter::DistanceMode: return;
    }
    wii_gx_native_fogf(wiiRenderValue(nativeParameter), value);
}

void renderFogi(RenderFogParameter parameter, RenderFogMode value)
{
    if (parameter == RenderFogParameter::DistanceMode)
        return;
    WiiRenderFogMode nativeMode = WiiRenderFogMode::Linear;
    switch (value)
    {
        case RenderFogMode::Exp: nativeMode = WiiRenderFogMode::Exp; break;
        case RenderFogMode::Exp2: nativeMode = WiiRenderFogMode::Exp2; break;
        case RenderFogMode::Linear: nativeMode = WiiRenderFogMode::Linear; break;
        case RenderFogMode::EyeRadial: return;
    }
    wii_gx_native_fogi(wiiRenderValue(WiiRenderFogParameter::Mode), static_cast<int>(wiiRenderValue(nativeMode)));
}

void renderFogColor(const float* values)
{
    wii_gx_native_fogfv(wiiRenderValue(WiiRenderFogParameter::Color), values);
}

void renderLightfv(int lightIndex, RenderLightParameter parameter, const float* values)
{
    WiiRenderLightParameter nativeParameter = WiiRenderLightParameter::Position;
    switch (parameter)
    {
        case RenderLightParameter::Ambient: nativeParameter = WiiRenderLightParameter::Ambient; break;
        case RenderLightParameter::Diffuse: nativeParameter = WiiRenderLightParameter::Diffuse; break;
        case RenderLightParameter::Specular: nativeParameter = WiiRenderLightParameter::Specular; break;
        case RenderLightParameter::Position: nativeParameter = WiiRenderLightParameter::Position; break;
    }
    wii_gx_native_lightfv(wiiRenderValue(WiiRenderCapability::Light0) + static_cast<unsigned int>(lightIndex), wiiRenderValue(nativeParameter), values);
}

void renderLightModelAmbient(const float* values)
{
    wii_gx_native_light_modelfv(wiiRenderValue(WiiRenderLightModelParameter::Ambient), values);
}

void renderColorMaterial(RenderFace face, RenderColorMaterialMode mode)
{
    const WiiRenderFace nativeFace = face == RenderFace::Front ? WiiRenderFace::Front : face == RenderFace::Back ? WiiRenderFace::Back : WiiRenderFace::FrontAndBack;
    const WiiRenderColorMaterialMode nativeMode = mode == RenderColorMaterialMode::Ambient ? WiiRenderColorMaterialMode::Ambient : WiiRenderColorMaterialMode::AmbientAndDiffuse;
    wii_gx_native_color_material(wiiRenderValue(nativeFace), wiiRenderValue(nativeMode));
}

void renderShadeModel(RenderShadeModel model)
{
    const WiiRenderShadeModel nativeModel = model == RenderShadeModel::Smooth
        ? WiiRenderShadeModel::Smooth
        : WiiRenderShadeModel::Flat;
    wii_gx_native_shade_model(wiiRenderValue(nativeModel));
}

void renderClear(unsigned int mask)
{
    // A colour clear is where a frame starts. If one arrives while a frame is
    // still submitted and unpresented, the caller is drawing a new frame from
    // outside the main loop -- LoadingScreenRenderer does this from inside a
    // tick -- and the queued copy must not be what the next swap presents.
    if ((mask & RenderClearMask::Color) != 0)
        wiigl_discard_pending_frame();
    wii_gx_native_clear(mask);
}

void renderFinishGpu()
{
}

void renderSubmitFrame()
{
    wiigl_submit_frame();
}

void renderClearColor(float r, float g, float b, float a)
{
    wii_gx_native_clear_color(r, g, b, a);
}

void renderClearDepth(double depth)
{
    wii_gx_native_clear_depth(depth);
}

void renderPolygonOffset(float factor, float units)
{
    wii_gx_native_polygon_offset(factor, units);
}

void renderLineWidth(float width)
{
    wii_gx_native_line_width(width);
}

void renderViewport(int x, int y, int width, int height)
{
    wii_gx_native_viewport(x, y, width, height);
}

void renderGetViewport(int* values)
{
    if (values == nullptr)
        return;
    WiiNativeRenderStateSnapshot state;
    if (wii_gx_get_native_state_snapshot(&state) != 0)
    {
        values[0] = state.viewport[0];
        values[1] = state.viewport[1];
        values[2] = state.viewport[2];
        values[3] = state.viewport[3];
    }
    else
    {
        values[0] = 0;
        values[1] = 0;
        values[2] = wiigl_width();
        values[3] = wiigl_height();
    }
}

void renderGetMatrix(RenderMatrixQuery query, float* values)
{
    WiiRenderMatrixQuery nativeQuery = query == RenderMatrixQuery::Projection ? WiiRenderMatrixQuery::Projection : query == RenderMatrixQuery::Texture ? WiiRenderMatrixQuery::Texture : WiiRenderMatrixQuery::ModelView;
    wii_gx_native_get_floatv(wiiRenderValue(nativeQuery), values);
}

const unsigned char* renderGetString(RenderStringQuery query)
{
    WiiRenderStringQuery nativeQuery = query == RenderStringQuery::Vendor ? WiiRenderStringQuery::Vendor : query == RenderStringQuery::Renderer ? WiiRenderStringQuery::Renderer : query == RenderStringQuery::Version ? WiiRenderStringQuery::Version : WiiRenderStringQuery::Extensions;
    return wii_gx_native_get_string(wiiRenderValue(nativeQuery));
}

bool renderSupportsFeature(RenderFeature feature)
{
    switch (feature)
    {
        case RenderFeature::Mipmaps: return true;
        case RenderFeature::AnisotropicFiltering: return true;
        case RenderFeature::FancyFogDistance:
        case RenderFeature::OcclusionQuery:
        case RenderFeature::MultisampleAntialiasing:
            return false;
    }
    return false;
}

unsigned int renderGetError()
{
    return wii_gx_native_get_error();
}

void renderFogHint(RenderHintMode mode)
{
    (void)mode;
}


void renderMatrixMode(RenderMatrixMode mode)
{
    unsigned int nativeMode = 0;
    switch (mode)
    {
        case RenderMatrixMode::ModelView: nativeMode = wiiRenderValue(WiiRenderMatrixMode::ModelView); break;
        case RenderMatrixMode::Projection: nativeMode = wiiRenderValue(WiiRenderMatrixMode::Projection); break;
        case RenderMatrixMode::Texture: nativeMode = wiiRenderValue(WiiRenderMatrixMode::Texture); break;
    }
    wii_gx_native_matrix_mode(nativeMode);
}

void renderLoadIdentity()
{
    wii_gx_native_load_identity();
}

void renderPushMatrix()
{
    wii_gx_native_matrix_push();
}

void renderPopMatrix()
{
    wii_gx_native_matrix_pop();
}

void renderTranslate(float x, float y, float z)
{
    wii_gx_native_translatef(x, y, z);
}

void renderRotate(float angle, float x, float y, float z)
{
    wii_gx_native_rotatef(angle, x, y, z);
}

void renderScale(float x, float y, float z)
{
    wii_gx_native_scalef(x, y, z);
}

void renderScaleDouble(double x, double y, double z)
{
    wii_gx_native_scaled(x, y, z);
}

void renderFrustum(double left, double right, double bottom, double top, double nearValue, double farValue)
{
    wii_gx_native_frustum(left, right, bottom, top, nearValue, farValue);
}

void renderOrtho(double left, double right, double bottom, double top, double nearValue, double farValue)
{
    wii_gx_native_ortho(left, right, bottom, top, nearValue, farValue);
}



bool renderReadPixelsRgb(int x, int y, int width, int height, void* pixels)
{
    (void)x; (void)y;
    if (pixels == nullptr || width <= 0 || height <= 0) return false;
    return wiigl_read_display_rgb(static_cast<unsigned char*>(pixels), width, height);
}

bool renderCopyFramebufferToBoundTexture(int, int, int, int)
{
    // The native GX backend does not expose a cheap EFB-to-bound-texture copy
    // through RenderAPI yet. GuiMainMenu renders the panorama directly instead.
    return false;
}


bool renderDrawInterleaved(const RenderInterleavedMesh& mesh)
{
    if (mesh.data == nullptr || mesh.stride <= 0 || mesh.count <= 0)
        return false;

    // WiiNativeDraw consumes Minecraft's established interleaved layout.
    // Reject incompatible layouts here instead of leaking GX-specific details
    // back into Tessellator.
    if ((mesh.hasTexture && mesh.texCoordOffset != 12) ||
        (mesh.hasColor && mesh.colorOffset != 20) ||
        (mesh.hasNormals && mesh.normalOffset != 24) ||
        (mesh.hasBrightness && mesh.brightnessOffset != 28))
        return false;

    WiiNativeMeshView nativeMesh;
    nativeMesh.vertices = mesh.data;
    nativeMesh.vertexStride = mesh.stride;
    nativeMesh.texCoordEnabled = mesh.hasTexture;
    nativeMesh.colorEnabled = mesh.hasColor;
    nativeMesh.hasNormals = mesh.hasNormals;
    nativeMesh.brightnessEnabled = mesh.hasBrightness;
    nativeMesh.brightnessOffset = mesh.brightnessOffset;
    nativeMesh.primitive = static_cast<WiiRenderPrimitive>(renderPrimitiveValue(mesh.primitive));
    nativeMesh.first = mesh.first;
    nativeMesh.count = mesh.count;
    return wii_native_draw_mesh(nativeMesh);
}


bool renderCaptureInterleaved(const RenderInterleavedMesh& mesh, RenderCapturedMesh& out, bool append)
{
    if (mesh.data == nullptr || mesh.stride != 32 || mesh.count <= 0)
        return false;
    if (!append) out.clear();
    if (!out.empty() && out.stride != 32) return false;
    if (out.empty()) {
        out.stride = 32; out.primitive = mesh.primitive;
        out.hasTexture = mesh.hasTexture; out.texCoordOffset = 12;
        out.hasColor = mesh.hasColor; out.colorOffset = 20;
        out.hasNormals = mesh.hasNormals; out.normalOffset = 24;
        out.hasBrightness = mesh.hasBrightness; out.brightnessOffset = 28;
    } else {
        out.hasTexture = out.hasTexture || mesh.hasTexture;
        out.hasColor = out.hasColor || mesh.hasColor;
        out.hasNormals = out.hasNormals || mesh.hasNormals;
        out.hasBrightness = out.hasBrightness || mesh.hasBrightness;
    }
    const int32_t* src = static_cast<const int32_t*>(mesh.data) + mesh.first * 8;
    const size_t base = out.raw.size();
    out.raw.insert(out.raw.end(), src, src + (size_t)mesh.count * 8u);
    for (int v = 0; v < mesh.count; ++v) {
        int32_t* dst = out.raw.data() + base + (size_t)v * 8u;
        if (!mesh.hasTexture) { dst[3] = 0; dst[4] = 0; }
        if (!mesh.hasColor) dst[5] = (int32_t)0xFFFFFFFFu;
        if (!mesh.hasNormals) dst[6] = 0;
        if (!mesh.hasBrightness) dst[7] = 0;
    }
    out.vertexCount += mesh.count;
    return true;
}

bool renderDrawCaptured(const RenderCapturedMesh& mesh)
{
    if (mesh.empty()) return false;
    RenderInterleavedMesh view;
    view.data = mesh.raw.data(); view.stride = mesh.stride; view.count = mesh.vertexCount;
    view.primitive = mesh.primitive; view.hasTexture = mesh.hasTexture; view.texCoordOffset = mesh.texCoordOffset;
    view.hasColor = mesh.hasColor; view.colorOffset = mesh.colorOffset;
    view.hasNormals = mesh.hasNormals; view.normalOffset = mesh.normalOffset;
    view.hasBrightness = mesh.hasBrightness; view.brightnessOffset = mesh.brightnessOffset;
    return renderDrawInterleaved(view);
}

int renderCreatePersistentMesh() { return wii_native_create_static_list_handle(); }
void renderDestroyPersistentMesh(int handle) { if (handle > 0) wii_native_destroy_static_list_handle(handle); }
bool renderCompilePersistentMesh(int handle, const RenderInterleavedMesh& mesh)
{
    if (handle <= 0 || mesh.data == nullptr || mesh.stride <= 0 || mesh.count <= 0) return false;
    return wii_native_compile_static_list(handle, mesh.data, mesh.stride,
        mesh.hasTexture, mesh.hasColor, mesh.hasNormals,
        mesh.hasBrightness, mesh.brightnessOffset,
        static_cast<WiiRenderPrimitive>(renderPrimitiveValue(mesh.primitive)), mesh.first, mesh.count);
}
bool renderDrawPersistentMesh(int handle) { return handle > 0 && wii_native_call_static_list(handle); }
bool renderCompileTerrainMesh(int handle, const RenderInterleavedMesh& mesh, const RenderTerrainCompileInfo& info)
{
    if (handle <= 0 || mesh.data == nullptr || mesh.stride <= 0 || mesh.count <= 0 || mesh.hasNormals) return false;
    // Vanilla 1.2.5 does not enable the lightmap texture unit for the normal
    // chunk passes. Keep the packed brightness in the CPU mesh contract for
    // immediate block/item rendering, but do not duplicate it into every GX
    // terrain display list.
    WiiNativeChunkFaceGroups nativeGroups;
    const WiiNativeChunkFaceGroups* nativeGroupsPtr = nullptr;
    if (info.faceGroups != nullptr)
    {
        static_assert(RenderTerrainFaceGroups::kGroupCount == WiiNativeChunkFaceGroups::kGroupCount,
                      "neutral and GX face group counts must agree");
        for (int g = 0; g < WiiNativeChunkFaceGroups::kGroupCount; ++g)
        {
            nativeGroups.quadCount[g] = info.faceGroups->quadCount[g];
            nativeGroups.planeMin[g] = info.faceGroups->planeMin[g];
            nativeGroups.planeMax[g] = info.faceGroups->planeMax[g];
        }
        nativeGroups.worldOriginX = info.worldOriginX;
        nativeGroups.worldOriginY = info.worldOriginY;
        nativeGroups.worldOriginZ = info.worldOriginZ;
        nativeGroupsPtr = &nativeGroups;
    }

    return wii_native_compile_chunk_list(handle, mesh.data, mesh.stride,
        mesh.hasTexture, mesh.hasColor, false, 28,
        static_cast<WiiRenderPrimitive>(renderPrimitiveValue(mesh.primitive)), mesh.first, mesh.count,
        info.translateX, info.translateY, info.translateZ, info.sectionSize, nativeGroupsPtr);
}
