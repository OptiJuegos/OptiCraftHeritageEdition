#pragma once

#include <cstdint>

enum class WiiRenderCapability : uint32_t
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

enum class WiiRenderCompare : uint32_t
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

enum class WiiRenderBlendFactor : uint32_t
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

enum class WiiRenderFace : uint32_t
{
    Front = 0x0404,
    Back = 0x0405,
    FrontAndBack = 0x0408
};

enum class WiiRenderColorMaterialMode : uint32_t
{
    Ambient = 0x1200,
    AmbientAndDiffuse = 0x1602
};

enum class WiiRenderShadeModel : uint32_t
{
    Flat = 0x1D00,
    Smooth = 0x1D01
};

enum class WiiRenderMatrixMode : uint32_t
{
    ModelView = 0x1700,
    Projection = 0x1701,
    Texture = 0x1702
};

enum class WiiRenderFogMode : uint32_t
{
    Exp = 0x0800,
    Exp2 = 0x0801,
    Linear = 0x2601
};

enum class WiiRenderFogParameter : uint32_t
{
    Density = 0x0B62,
    Start = 0x0B63,
    End = 0x0B64,
    Mode = 0x0B65,
    Color = 0x0B66
};

enum class WiiRenderLightParameter : uint32_t
{
    Ambient = 0x1200,
    Diffuse = 0x1201,
    Specular = 0x1202,
    Position = 0x1203
};

enum class WiiRenderLightModelParameter : uint32_t
{
    Ambient = 0x0B53
};

enum class WiiRenderMatrixQuery : uint32_t
{
    ModelView = 0x0BA6,
    Projection = 0x0BA7,
    Texture = 0x0BA8
};

enum class WiiRenderStringQuery : uint32_t
{
    Vendor = 0x1F00,
    Renderer = 0x1F01,
    Version = 0x1F02,
    Extensions = 0x1F03
};

enum class WiiRenderTextureTarget : uint32_t
{
    Texture2D = 0x0DE1
};

enum class WiiRenderPixelFormat : uint32_t
{
    Rgba = 0x1908
};

enum class WiiRenderPixelType : uint32_t
{
    UnsignedByte = 0x1401
};

enum class WiiRenderPrimitive : uint32_t
{
    Points = 0x0000,
    Lines = 0x0001,
    LineLoop = 0x0002,
    LineStrip = 0x0003,
    Triangles = 0x0004,
    TriangleStrip = 0x0005,
    TriangleFan = 0x0006,
    Quads = 0x0007
};

namespace WiiRenderClearMask
{
constexpr uint32_t Depth = 0x00000100;
constexpr uint32_t Color = 0x00004000;
}

constexpr uint32_t WII_RENDER_NO_ERROR = 0;

constexpr uint32_t wiiRenderValue(WiiRenderCapability value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderCompare value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderBlendFactor value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderFace value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderColorMaterialMode value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderShadeModel value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderMatrixMode value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderFogMode value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderFogParameter value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderLightParameter value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderLightModelParameter value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderMatrixQuery value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderStringQuery value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderTextureTarget value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderPixelFormat value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderPixelType value) { return static_cast<uint32_t>(value); }
constexpr uint32_t wiiRenderValue(WiiRenderPrimitive value) { return static_cast<uint32_t>(value); }
