#include "platform/RenderAPI.h"


#include "ps2/render/Ps2CaptureLayout.h"
#include "ps2/render/Ps2RenderApi.h"
#include "ps2/render/Ps2NativeDraw.h"
#include "ps2/render/Ps2Texture.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace
{
    static int s_activeTextureUnit = 0;
    static bool s_lightmapEnabled = false;

    static float ps2PackedLightFactor(int brightness)
    {
        // Equivalent to WorldProvider::lightBrightnessTable for light levels
        // 0..15. A lookup avoids a floating-point divide for every captured
        // vertex on the R5900.
        static const float lightTable[16] =
        {
            0.0f, 0.01754386f, 0.03703704f, 0.05882353f,
            0.08333333f, 0.11111111f, 0.14285715f, 0.17948718f,
            0.22222222f, 0.27272728f, 0.33333334f, 0.40740740f,
            0.5f, 0.61904764f, 0.77777779f, 1.0f
        };
        const int sky = (brightness >> 20) & 15;
        const int block = (brightness >> 4) & 15;
        return lightTable[std::max(sky, block)];
    }

    static std::uint32_t ps2ApplyPackedLight(std::uint32_t color, int brightness)
    {
        const float factor = ps2PackedLightFactor(brightness);
        const unsigned int red = color & 0xffu;
        const unsigned int green = (color >> 8) & 0xffu;
        const unsigned int blue = (color >> 16) & 0xffu;
        const unsigned int alpha = (color >> 24) & 0xffu;
        const unsigned int litRed = static_cast<unsigned int>(static_cast<float>(red) * factor + 0.5f);
        const unsigned int litGreen = static_cast<unsigned int>(static_cast<float>(green) * factor + 0.5f);
        const unsigned int litBlue = static_cast<unsigned int>(static_cast<float>(blue) * factor + 0.5f);
        return (alpha << 24) | (litBlue << 16) | (litGreen << 8) | litRed;
    }
}


void renderEnable(RenderCapability capability)
{
    switch (capability)
    {
        case RenderCapability::Texture2D:
            if (s_activeTextureUnit == 1)
            {
                s_lightmapEnabled = true;
                ps2_render_set_lightmap_enabled(true);
            }
            else
            {
                ps2_render_enable(ps2RenderValue(Ps2RenderCapability::Texture2D));
            }
            break;
        case RenderCapability::ColorMaterial: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::ColorMaterial)); break;
        case RenderCapability::CullFace: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::CullFace)); break;
        case RenderCapability::AlphaTest: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::AlphaTest)); break;
        case RenderCapability::Blend: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::Blend)); break;
        case RenderCapability::DepthTest: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::DepthTest)); break;
        case RenderCapability::Fog: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::Fog)); break;
        case RenderCapability::Lighting: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::Lighting)); break;
        case RenderCapability::Normalize: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::Normalize)); break;
        case RenderCapability::RescaleNormal: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::RescaleNormal)); break;
        case RenderCapability::Light0: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::Light0)); break;
        case RenderCapability::Light1: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::Light1)); break;
        case RenderCapability::PolygonOffsetFill: ps2_render_enable(ps2RenderValue(Ps2RenderCapability::PolygonOffsetFill)); break;
    }
}

void renderDisable(RenderCapability capability)
{
    switch (capability)
    {
        case RenderCapability::Texture2D:
            if (s_activeTextureUnit == 1)
            {
                s_lightmapEnabled = false;
                ps2_render_set_lightmap_enabled(false);
            }
            else
            {
                ps2_render_disable(ps2RenderValue(Ps2RenderCapability::Texture2D));
            }
            break;
        case RenderCapability::ColorMaterial: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::ColorMaterial)); break;
        case RenderCapability::CullFace: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::CullFace)); break;
        case RenderCapability::AlphaTest: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::AlphaTest)); break;
        case RenderCapability::Blend: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::Blend)); break;
        case RenderCapability::DepthTest: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::DepthTest)); break;
        case RenderCapability::Fog: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::Fog)); break;
        case RenderCapability::Lighting: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::Lighting)); break;
        case RenderCapability::Normalize: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::Normalize)); break;
        case RenderCapability::RescaleNormal: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::RescaleNormal)); break;
        case RenderCapability::Light0: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::Light0)); break;
        case RenderCapability::Light1: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::Light1)); break;
        case RenderCapability::PolygonOffsetFill: ps2_render_disable(ps2RenderValue(Ps2RenderCapability::PolygonOffsetFill)); break;
    }
}

void renderBlendFunc(RenderBlendFactor source, RenderBlendFactor destination)
{
    auto ps2Factor = [](RenderBlendFactor factor) -> unsigned int
    {
        switch (factor)
        {
            case RenderBlendFactor::Zero: return ps2RenderValue(Ps2RenderBlendFactor::Zero);
            case RenderBlendFactor::One: return ps2RenderValue(Ps2RenderBlendFactor::One);
            case RenderBlendFactor::SrcColor: return ps2RenderValue(Ps2RenderBlendFactor::SrcColor);
            case RenderBlendFactor::OneMinusSrcColor: return ps2RenderValue(Ps2RenderBlendFactor::OneMinusSrcColor);
            case RenderBlendFactor::SrcAlpha: return ps2RenderValue(Ps2RenderBlendFactor::SrcAlpha);
            case RenderBlendFactor::OneMinusSrcAlpha: return ps2RenderValue(Ps2RenderBlendFactor::OneMinusSrcAlpha);
            case RenderBlendFactor::DstAlpha: return ps2RenderValue(Ps2RenderBlendFactor::DstAlpha);
            case RenderBlendFactor::OneMinusDstAlpha: return ps2RenderValue(Ps2RenderBlendFactor::OneMinusDstAlpha);
            case RenderBlendFactor::DstColor: return ps2RenderValue(Ps2RenderBlendFactor::DstColor);
            case RenderBlendFactor::OneMinusDstColor: return ps2RenderValue(Ps2RenderBlendFactor::OneMinusDstColor);
        }
        return ps2RenderValue(Ps2RenderBlendFactor::One);
    };
    ps2_render_blend_func(ps2Factor(source), ps2Factor(destination));
}

void renderDepthMask(bool enabled)
{
    ps2_render_depth_mask(enabled);
}

void renderDepthFunc(RenderCompare function)
{
    auto ps2Compare = [](RenderCompare compare) -> unsigned int
    {
        switch (compare)
        {
            case RenderCompare::Never: return ps2RenderValue(Ps2RenderCompare::Never);
            case RenderCompare::Less: return ps2RenderValue(Ps2RenderCompare::Less);
            case RenderCompare::Equal: return ps2RenderValue(Ps2RenderCompare::Equal);
            case RenderCompare::LessEqual: return ps2RenderValue(Ps2RenderCompare::LessEqual);
            case RenderCompare::Greater: return ps2RenderValue(Ps2RenderCompare::Greater);
            case RenderCompare::NotEqual: return ps2RenderValue(Ps2RenderCompare::NotEqual);
            case RenderCompare::GreaterEqual: return ps2RenderValue(Ps2RenderCompare::GreaterEqual);
            case RenderCompare::Always: return ps2RenderValue(Ps2RenderCompare::Always);
        }
        return ps2RenderValue(Ps2RenderCompare::Always);
    };
    ps2_render_depth_func(ps2Compare(function));
}

void renderAlphaFunc(RenderCompare function, float reference)
{
    auto ps2Compare = [](RenderCompare compare) -> unsigned int
    {
        switch (compare)
        {
            case RenderCompare::Never: return ps2RenderValue(Ps2RenderCompare::Never);
            case RenderCompare::Less: return ps2RenderValue(Ps2RenderCompare::Less);
            case RenderCompare::Equal: return ps2RenderValue(Ps2RenderCompare::Equal);
            case RenderCompare::LessEqual: return ps2RenderValue(Ps2RenderCompare::LessEqual);
            case RenderCompare::Greater: return ps2RenderValue(Ps2RenderCompare::Greater);
            case RenderCompare::NotEqual: return ps2RenderValue(Ps2RenderCompare::NotEqual);
            case RenderCompare::GreaterEqual: return ps2RenderValue(Ps2RenderCompare::GreaterEqual);
            case RenderCompare::Always: return ps2RenderValue(Ps2RenderCompare::Always);
        }
        return ps2RenderValue(Ps2RenderCompare::Always);
    };
    ps2_render_alpha_func(ps2Compare(function), reference);
}

void renderCullFace(RenderFace face)
{
    switch (face)
    {
        case RenderFace::Front: ps2_render_cull_face(ps2RenderValue(Ps2RenderFace::Front)); break;
        case RenderFace::Back: ps2_render_cull_face(ps2RenderValue(Ps2RenderFace::Back)); break;
        case RenderFace::FrontAndBack: ps2_render_cull_face(ps2RenderValue(Ps2RenderFace::FrontAndBack)); break;
    }
}

void renderColorMask(bool red, bool green, bool blue, bool alpha)
{
    ps2_render_color_mask(red, green, blue, alpha);
}

void renderBindTexture(int texture)
{
    if (s_activeTextureUnit == 0)
        ps2_texture_bind(static_cast<unsigned int>(texture));
}

void renderSetActiveTextureUnit(int textureUnit)
{
    s_activeTextureUnit = textureUnit == 0x84C1 ? 1 : 0;
}

void renderSetClientActiveTextureUnit(int textureUnit)
{
    (void)textureUnit;
}

void renderSetMultiTextureCoord(int textureUnit, float u, float v)
{
    if (textureUnit == 0x84C1)
        ps2_render_set_lightmap_coord(u, v);
}

void renderSetLightmapColors(const std::uint32_t* colors, int count)
{
    ps2_render_set_lightmap_colors(reinterpret_cast<const unsigned int*>(colors), count);
}

void renderColor4f(float r, float g, float b, float a)
{
    ps2_render_color4f(r, g, b, a);
}

void renderColor3f(float r, float g, float b)
{
    ps2_render_color4f(r, g, b, 1.0f);
}

void renderNormal3f(float x, float y, float z)
{
    ps2_render_normal3f(x, y, z);
}

void renderGenerateTextures(int count, int *textures)
{
    if (count <= 0 || textures == nullptr)
        return;
    ps2_texture_generate_names(count, reinterpret_cast<unsigned int *>(textures));
}

void renderDeleteTextures(int count, const int *textures)
{
    if (count <= 0 || textures == nullptr)
        return;
    ps2_texture_delete_names(count, reinterpret_cast<const unsigned int *>(textures));
}

void renderTextureSubImageRgba(int level, int x, int y, int width, int height, const void *pixels)
{
    ps2_texture_upload_sub_rgba(ps2_texture_bound_name(), level, x, y, width, height, pixels);
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
    ps2_texture_upload_rgba(ps2_texture_bound_name(), level, width, height, pixels);
}

// The GS backend stores the sampling state it can represent directly.
void renderTextureParameters(bool blur, bool mipmaps, bool clamp)
{
    ps2_texture_set_parameters(blur, mipmaps, clamp);
}

int renderGetMaxAnisotropy() { return 1; }
int renderGetMaxSamples() { return 0; }

bool renderTextureBeginUpload(int texture, int, int, int, bool, bool, bool tileAtlas)
{
    ps2_texture_bind(static_cast<unsigned int>(texture));
    ps2_texture_set_tile_atlas((unsigned int)texture, tileAtlas);
    return true;
}
bool renderTextureIsValid(int texture) { return texture >= 0 && ps2_texture_valid((unsigned int)texture); }
void renderResetResources()
{
    s_activeTextureUnit = 0;
    s_lightmapEnabled = false;
    ps2_render_set_lightmap_enabled(false);
}


void renderFogf(RenderFogParameter parameter, float value)
{
    switch (parameter)
    {
        case RenderFogParameter::Density: ps2_render_fogf(ps2RenderValue(Ps2RenderFogParameter::Density), value); break;
        case RenderFogParameter::Start: ps2_render_fogf(ps2RenderValue(Ps2RenderFogParameter::Start), value); break;
        case RenderFogParameter::End: ps2_render_fogf(ps2RenderValue(Ps2RenderFogParameter::End), value); break;
        case RenderFogParameter::Mode: ps2_render_fogf(ps2RenderValue(Ps2RenderFogParameter::Mode), value); break;
        case RenderFogParameter::Color: ps2_render_fogf(ps2RenderValue(Ps2RenderFogParameter::Color), value); break;
        // No NV distance-mode equivalent on the GS.
        case RenderFogParameter::DistanceMode: break;
    }
}

void renderFogi(RenderFogParameter parameter, RenderFogMode value)
{
    // The native EE paths implement the requested equation. Direct VU1 terrain
    // accepts linear fog only and falls back to VU0 for EXP/EXP2 passes.
    if (parameter != RenderFogParameter::Mode)
        return;
    switch (value)
    {
        case RenderFogMode::Exp: ps2_render_fog_mode(ps2RenderValue(Ps2RenderFogMode::Exp)); break;
        case RenderFogMode::Exp2: ps2_render_fog_mode(ps2RenderValue(Ps2RenderFogMode::Exp2)); break;
        case RenderFogMode::Linear: ps2_render_fog_mode(ps2RenderValue(Ps2RenderFogMode::Linear)); break;
        // No NV eye-radial equivalent on the GS.
        case RenderFogMode::EyeRadial: break;
    }
}

void renderFogColor(const float* values)
{
    ps2_render_fog_color(values);
}

void renderLightfv(int lightIndex, RenderLightParameter parameter, const float* values)
{
    switch (parameter)
    {
        case RenderLightParameter::Ambient: ps2_render_lightfv(lightIndex, ps2RenderValue(Ps2RenderLightParameter::Ambient), values); break;
        case RenderLightParameter::Diffuse: ps2_render_lightfv(lightIndex, ps2RenderValue(Ps2RenderLightParameter::Diffuse), values); break;
        case RenderLightParameter::Specular: ps2_render_lightfv(lightIndex, ps2RenderValue(Ps2RenderLightParameter::Specular), values); break;
        case RenderLightParameter::Position: ps2_render_lightfv(lightIndex, ps2RenderValue(Ps2RenderLightParameter::Position), values); break;
    }
}

void renderLightModelAmbient(const float* values)
{
    ps2_render_light_model_ambient(values);
}

void renderColorMaterial(RenderFace face, RenderColorMaterialMode mode)
{
    const unsigned int ps2Face = face == RenderFace::Front ? ps2RenderValue(Ps2RenderFace::Front)
        : face == RenderFace::Back ? ps2RenderValue(Ps2RenderFace::Back)
        : ps2RenderValue(Ps2RenderFace::FrontAndBack);
    const unsigned int ps2Mode = mode == RenderColorMaterialMode::Ambient
        ? ps2RenderValue(Ps2RenderColorMaterialMode::Ambient)
        : ps2RenderValue(Ps2RenderColorMaterialMode::AmbientAndDiffuse);
    ps2_render_color_material(ps2Face, ps2Mode);
}

void renderShadeModel(RenderShadeModel model)
{
    ps2_render_shade_model(model == RenderShadeModel::Smooth
        ? ps2RenderValue(Ps2RenderShadeModel::Smooth)
        : ps2RenderValue(Ps2RenderShadeModel::Flat));
}

void renderClear(unsigned int mask)
{
    ps2_render_clear(mask);
}

void renderFinishGpu()
{
}

// PS2 presents from ps2_render_swap_buffers(); there is nothing to start early.
void renderSubmitFrame()
{
}

void renderClearColor(float r, float g, float b, float a)
{
    ps2_render_clear_color(r, g, b, a);
}

void renderClearDepth(double depth)
{
    ps2_render_clear_depth(static_cast<float>(depth));
}

void renderPolygonOffset(float factor, float units)
{
    ps2_render_polygon_offset(factor, units);
}

void renderLineWidth(float width)
{
    ps2_render_line_width(width);
}

void renderViewport(int x, int y, int width, int height)
{
    ps2_render_viewport(x, y, width, height);
}

void renderGetViewport(int* values)
{
    ps2_render_get_viewport(values);
}

void renderGetMatrix(RenderMatrixQuery query, float* values)
{
    ps2_render_get_matrix(query == RenderMatrixQuery::Projection ? ps2RenderValue(Ps2RenderMatrixQuery::Projection)
        : query == RenderMatrixQuery::Texture ? ps2RenderValue(Ps2RenderMatrixQuery::Texture)
        : ps2RenderValue(Ps2RenderMatrixQuery::ModelView), values);
}

const unsigned char* renderGetString(RenderStringQuery query)
{
    // One name for every query; the backend has no driver strings to report.
    (void)query;
    return ps2_render_get_string();
}

bool renderSupportsFeature(RenderFeature feature)
{
    switch (feature)
    {
        case RenderFeature::Mipmaps: return false;
        case RenderFeature::AnisotropicFiltering: return false;
        case RenderFeature::MultisampleAntialiasing: return false;
        case RenderFeature::FancyFogDistance:
        case RenderFeature::OcclusionQuery:
            return false;
    }
    return false;
}

unsigned int renderGetError()
{
    return ps2_render_get_error();
}

void renderFogHint(RenderHintMode mode)
{
    ps2_render_fog_hint(mode == RenderHintMode::Nicest);
}


void renderMatrixMode(RenderMatrixMode mode)
{
    switch (mode)
    {
        case RenderMatrixMode::ModelView: ps2_render_matrix_mode(ps2RenderValue(Ps2RenderMatrixMode::ModelView)); break;
        case RenderMatrixMode::Projection: ps2_render_matrix_mode(ps2RenderValue(Ps2RenderMatrixMode::Projection)); break;
        case RenderMatrixMode::Texture: ps2_render_matrix_mode(ps2RenderValue(Ps2RenderMatrixMode::Texture)); break;
    }
}

void renderLoadIdentity()
{
    ps2_render_load_identity();
}

void renderPushMatrix()
{
    ps2_render_push_matrix();
}

void renderPopMatrix()
{
    ps2_render_pop_matrix();
}

void renderTranslate(float x, float y, float z)
{
    ps2_render_translate(x, y, z);
}

void renderRotate(float angle, float x, float y, float z)
{
    ps2_render_rotate(angle, x, y, z);
}

void renderScale(float x, float y, float z)
{
    ps2_render_scale(x, y, z);
}

void renderScaleDouble(double x, double y, double z)
{
    ps2_render_scale(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

// The shared signatures stay double because the GL backend hands them to
// glFrustum/glOrtho unchanged. This is the boundary where the PS2 backend leaves
// double behind: everything past here is float, which is what the Mat4 and the
// GS take anyway. See the note in Ps2RenderApi.h.
void renderFrustum(double left, double right, double bottom, double top, double nearValue, double farValue)
{
    ps2_render_frustum(static_cast<float>(left), static_cast<float>(right),
                       static_cast<float>(bottom), static_cast<float>(top),
                       static_cast<float>(nearValue), static_cast<float>(farValue));
}

void renderOrtho(double left, double right, double bottom, double top, double nearValue, double farValue)
{
    ps2_render_ortho(static_cast<float>(left), static_cast<float>(right),
                     static_cast<float>(bottom), static_cast<float>(top),
                     static_cast<float>(nearValue), static_cast<float>(farValue));
}



bool renderCopyFramebufferToBoundTexture(int, int, int, int) { return false; }


bool renderDrawInterleaved(const RenderInterleavedMesh& mesh)
{
    if (mesh.data == nullptr || mesh.stride <= 0 || mesh.count <= 0)
        return false;

    const unsigned char* vertexBase = reinterpret_cast<const unsigned char*>(mesh.data) +
                                      static_cast<std::size_t>(mesh.first) * static_cast<std::size_t>(mesh.stride);

    Ps2NativeMeshView nativeMesh;
    nativeMesh.packedTerrain = false;
    nativeMesh.clampRuns = nullptr;
    nativeMesh.clampRunCount = 0;
    nativeMesh.vertices = vertexBase;
    nativeMesh.vertexStride = mesh.stride;
    nativeMesh.vertexSize = 3;
    nativeMesh.texCoords = mesh.hasTexture
        ? vertexBase + mesh.texCoordOffset
        : nullptr;
    nativeMesh.texCoordStride = mesh.stride;
    nativeMesh.texCoordEnabled = mesh.hasTexture;

    static std::vector<std::uint32_t> brightnessColors;
    if (mesh.hasBrightness)
    {
        brightnessColors.resize(static_cast<std::size_t>(mesh.count));
        for (int vertex = 0; vertex < mesh.count; ++vertex)
        {
            const unsigned char* source = vertexBase + static_cast<std::size_t>(vertex) * static_cast<std::size_t>(mesh.stride);
            const std::uint32_t color = mesh.hasColor
                ? *reinterpret_cast<const std::uint32_t*>(source + mesh.colorOffset)
                : 0xffffffffu;
            const int brightness = *reinterpret_cast<const int*>(source + mesh.brightnessOffset);
            brightnessColors[static_cast<std::size_t>(vertex)] = ps2ApplyPackedLight(color, brightness);
        }
        nativeMesh.colors = brightnessColors.data();
        nativeMesh.colorStride = static_cast<int>(sizeof(std::uint32_t));
        nativeMesh.colorEnabled = true;
    }
    else
    {
        nativeMesh.colors = mesh.hasColor
            ? vertexBase + mesh.colorOffset
            : nullptr;
        nativeMesh.colorStride = mesh.stride;
        nativeMesh.colorEnabled = mesh.hasColor;
    }
    nativeMesh.colorSize = 4;
    nativeMesh.colorFloat = false;
    nativeMesh.normals = mesh.hasNormals
        ? vertexBase + mesh.normalOffset
        : nullptr;
    nativeMesh.normalStride = mesh.stride;
    nativeMesh.normalFloat = false;
    nativeMesh.hasNormals = mesh.hasNormals;
    nativeMesh.drawMode = renderPrimitiveValue(mesh.primitive);
    nativeMesh.first = 0;
    nativeMesh.count = mesh.count;
    nativeMesh.slices = nullptr;
    nativeMesh.sliceCount = 0;

    // Per-vertex packed brightness already represents lightmap input for this
    // mesh. Keep the entity/current-coordinate CPU lightmap from multiplying it
    // a second time while the native draw consumes the pre-lit color array.
    const bool suppressCpuLightmap = mesh.hasBrightness && s_lightmapEnabled;
    if (suppressCpuLightmap)
        ps2_render_set_lightmap_enabled(false);
    const bool drawn = ps2_native_draw_mesh(nativeMesh);
    if (suppressCpuLightmap)
        ps2_render_set_lightmap_enabled(true);
    return drawn;
}


bool renderCaptureInterleaved(const RenderInterleavedMesh& mesh, RenderCapturedMesh& out, bool append)
{
    if (mesh.data == nullptr || mesh.stride != 32 || mesh.count <= 0)
        return false;
    if (!append)
        out.clear();

    constexpr int kStride = Ps2CaptureLayout::Stride;
    if (!out.empty() && (out.stride != kStride || out.primitive != mesh.primitive))
        return false;
    if (out.empty())
    {
        out.stride = kStride;
        out.primitive = mesh.primitive;
        out.hasTexture = true;
        out.texCoordOffset = Ps2CaptureLayout::TexCoordOffset;
        out.hasColor = true;
        out.colorOffset = Ps2CaptureLayout::ColorOffset;
        out.hasNormals = false;
        out.normalOffset = 0;
        out.hasBrightness = false;
        out.brightnessOffset = 0;
    }

    const int32_t* src = static_cast<const int32_t*>(mesh.data) + mesh.first * 8;
    const size_t base = out.raw.size();
    const size_t requiredRawInts = base + static_cast<size_t>(mesh.count) * Ps2CaptureLayout::Slots;
    if (requiredRawInts > out.raw.capacity())
        out.raw.reserve(requiredRawInts);
    out.raw.resize(requiredRawInts);
    int32_t* dst = out.raw.data() + base;
    for (int vertex = 0; vertex < mesh.count; ++vertex, src += 8, dst += Ps2CaptureLayout::Slots)
    {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = mesh.hasTexture ? src[3] : 0;
        dst[4] = mesh.hasTexture ? src[4] : 0;

        std::uint32_t color = mesh.hasColor
            ? static_cast<std::uint32_t>(src[5])
            : 0xffffffffu;
        if (mesh.hasBrightness)
            color = ps2ApplyPackedLight(color, src[7]);
        dst[5] = static_cast<int32_t>(color);
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

