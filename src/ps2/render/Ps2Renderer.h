#pragma once

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2NativeDraw.h"

struct Ps2RendererFrame
{
    Ps2NativeFrameContext native;
    bool valid;
};

bool ps2_renderer_capture_frame(Ps2RendererFrame& out);
bool ps2_renderer_prepare_translated_context(Ps2NativeDrawContext& out,
                                              const Ps2RendererFrame& frame,
                                              float tx, float ty, float tz,
                                              bool fullyInside);
bool ps2_renderer_draw_prepared(const Ps2NativeMeshView& mesh,
                                const Ps2NativeDrawContext& context);

#endif
