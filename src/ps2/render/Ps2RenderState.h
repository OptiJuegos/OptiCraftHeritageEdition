#pragma once

#ifdef PS2_PLATFORM


// PS2-native raster state captured at the compatibility boundary.
//
// The public OpenGL shim still owns the API-facing state while the migration is
// in progress, but the gsKit/VU draw backends consume this structure instead of
// reaching back into the GL state object. As more callers become native this
// structure can be populated directly without changing the draw backends.
struct Ps2RenderState {
    bool cullFace;
    bool frontFaceCCW;
    bool cullBackFace;

    unsigned char flatR;
    unsigned char flatG;
    unsigned char flatB;
    unsigned char flatA;

    bool fogEnabled;
    unsigned int fogMode;
    float fogDensity;
    float fogStart;
    float fogEnd;
    float fogR;
    float fogG;
    float fogB;

    // Precomputed at capture time so the per-vertex LINEAR fog path is a
    // multiply-add instead of a subtract-and-divide. Valid only when
    // fogEnd > fogStart (see ps2_capture_render_state).
    bool fogLinearValid;
    float fogLinearScale;
    float fogLinearBias;

    bool smoothShading;

    // Transitional fixed-function lighting hook. The lighting implementation
    // still lives in gles_ps2.cpp, but the backend no longer reads its globals
    // directly. This callback can be replaced by native light data later.
    void (*lightVertex)(int index,
                        unsigned char& r,
                        unsigned char& g,
                        unsigned char& b);
};

static inline float ps2_fast_negative_exp(float x)
{
    if (x <= 0.0f) return 1.0f;
    if (x >= 16.0f) return 0.0f;

    // (1 + x/256)^-256 is a close, monotonic e^-x approximation and costs
    // one divide plus eight multiplies. That is dramatically cheaper on the
    // EE than calling the software-libm expf() once per rendered vertex.
    float value = 1.0f / (1.0f + x * (1.0f / 256.0f));
    value *= value;
    value *= value;
    value *= value;
    value *= value;
    value *= value;
    value *= value;
    value *= value;
    value *= value;
    return value;
}

static inline float ps2_render_fog_factor(const Ps2RenderState& state, float depth)
{
    if (!state.fogEnabled || depth < 0.0f)
        return 1.0f;

    float factor = 1.0f;
    if (state.fogMode == 0x0800u)
    {
        factor = ps2_fast_negative_exp(state.fogDensity * depth);
    }
    else if (state.fogMode == 0x0801u)
    {
        const float d = state.fogDensity * depth;
        factor = ps2_fast_negative_exp(d * d);
    }
    else if (state.fogLinearValid)
    {
        factor = state.fogLinearBias - depth * state.fogLinearScale;
    }

    if (factor < 0.0f) return 0.0f;
    if (factor > 1.0f) return 1.0f;
    return factor;
}

#endif // PS2_PLATFORM
