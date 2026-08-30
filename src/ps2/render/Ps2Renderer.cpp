#include "ps2/render/Ps2Renderer.h"

#ifdef PS2_PLATFORM

bool ps2_renderer_capture_frame(Ps2RendererFrame& out)
{
    out.valid = ps2_native_prepare_frame_context(out.native);
    return out.valid;
}

bool ps2_renderer_prepare_translated_context(Ps2NativeDrawContext& out,
                                              const Ps2RendererFrame& frame,
                                              float tx, float ty, float tz,
                                              bool fullyInside)
{
    if (!frame.valid)
    {
        out.valid = false;
        out.fullyInside = fullyInside;
        return false;
    }

    return ps2_native_prepare_translated_context(out, frame.native,
                                                 tx, ty, tz, fullyInside);
}

bool ps2_renderer_draw_prepared(const Ps2NativeMeshView& mesh,
                                const Ps2NativeDrawContext& context)
{
    return ps2_native_draw_mesh_prepared(mesh, context);
}

#endif
