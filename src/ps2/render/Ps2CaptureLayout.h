#pragma once

#ifdef PS2_PLATFORM

#include <cstddef>
#include <cstdint>

namespace Ps2CaptureLayout
{
    // Compact PS2 captured vertex produced by RenderAPI_GS_PS2:
    // xyz (3 x 32-bit), uv (2 x 32-bit), rgba (1 x 32-bit).
    constexpr std::size_t Slots = 6u;
    constexpr int Stride = static_cast<int>(Slots * sizeof(std::int32_t));
    constexpr int PositionOffset = 0;
    constexpr int TexCoordOffset = 12;
    constexpr int ColorOffset = 20;

    static_assert(Stride == 24, "PS2 captured vertex layout must remain 24 bytes");
}

#endif // PS2_PLATFORM
