#pragma once

#include <cstdint>

// Fixed-function selectors for the native PS2 backend, mirroring
// wii/render/WiiRenderTypes.h.
//
// The numeric values are the OpenGL enumerants on purpose. They are not an
// OpenGL dependency -- Ps2RenderApi.h does not include a GL header and the
// backend never links one -- they are simply the numbering RenderAPI's callers
// already think in, and reusing it keeps the compatibility layer's own switch
// statements valid while both paths coexist.

enum class Ps2RenderCapability : uint32_t
{
    Texture2D = 0x0DE1,
    ColorMaterial = 0x0B57,
    CullFace = 0x0B44,
    AlphaTest = 0x0BC0,
    Blend = 0x0BE2,
    DepthTest = 0x0B71,
    Fog = 0x0B60,
    Lighting = 0x0B50,
    Normalize = 0x0BA1,
    RescaleNormal = 0x803A,
    Light0 = 0x4000,
    Light1 = 0x4001,
    PolygonOffsetFill = 0x8037
};

enum class Ps2RenderCompare : uint32_t
{
    Never = 0x0200,
    Less = 0x0201,
    Equal = 0x0202,
    LessEqual = 0x0203,
    Greater = 0x0204,
    NotEqual = 0x0205,
    GreaterEqual = 0x0206,
    Always = 0x0207
};

enum class Ps2RenderBlendFactor : uint32_t
{
    Zero = 0x0000,
    One = 0x0001,
    SrcColor = 0x0300,
    OneMinusSrcColor = 0x0301,
    SrcAlpha = 0x0302,
    OneMinusSrcAlpha = 0x0303,
    DstAlpha = 0x0304,
    OneMinusDstAlpha = 0x0305,
    DstColor = 0x0306,
    OneMinusDstColor = 0x0307
};

enum class Ps2RenderFace : uint32_t
{
    Front = 0x0404,
    Back = 0x0405,
    FrontAndBack = 0x0408
};

enum class Ps2RenderColorMaterialMode : uint32_t
{
    Ambient = 0x1200,
    AmbientAndDiffuse = 0x1602
};

enum class Ps2RenderMatrixMode : uint32_t
{
    ModelView = 0x1700,
    Projection = 0x1701,
    Texture = 0x1702
};

enum class Ps2RenderFogMode : uint32_t
{
    Exp = 0x0800,
    Exp2 = 0x0801,
    Linear = 0x2601
};

enum class Ps2RenderFogParameter : uint32_t
{
    Density = 0x0B62,
    Start = 0x0B63,
    End = 0x0B64,
    Mode = 0x0B65,
    Color = 0x0B66
};

enum class Ps2RenderLightParameter : uint32_t
{
    Ambient = 0x1200,
    Diffuse = 0x1201,
    Specular = 0x1202,
    Position = 0x1203
};

enum class Ps2RenderShadeModel : uint32_t
{
    Flat = 0x1D00,
    Smooth = 0x1D01
};

enum class Ps2RenderMatrixQuery : uint32_t
{
    ModelView = 0x0BA6,
    Projection = 0x0BA7,
    Texture = 0x0BA8
};

namespace Ps2RenderClearMask
{
constexpr uint32_t Depth = 0x00000100;
constexpr uint32_t Color = 0x00004000;
}

constexpr uint32_t PS2_RENDER_NO_ERROR = 0;

constexpr uint32_t ps2RenderValue(Ps2RenderCapability value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderCompare value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderBlendFactor value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderFace value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderColorMaterialMode value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderMatrixMode value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderFogMode value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderFogParameter value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderLightParameter value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderShadeModel value) { return static_cast<uint32_t>(value); }
constexpr uint32_t ps2RenderValue(Ps2RenderMatrixQuery value) { return static_cast<uint32_t>(value); }
