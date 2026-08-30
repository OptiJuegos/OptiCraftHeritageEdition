#pragma once

#ifdef PS2_PLATFORM

struct Ps2ProjectionState
{
    float left = 0.0f;
    float right = 1.0f;
    float bottom = 0.0f;
    float top = 1.0f;
    float nearPlane = 0.0f;
    float farPlane = 1.0f;
    bool orthographic = false;
};

extern Ps2ProjectionState g_ps2ProjectionState;

inline const Ps2ProjectionState& ps2_projection_state()
{
    return g_ps2ProjectionState;
}

inline bool ps2_projection_is_orthographic()
{
    return g_ps2ProjectionState.orthographic;
}

void ps2_projection_set_ortho(float left, float right, float bottom, float top,
                              float nearPlane, float farPlane);
void ps2_projection_clear_ortho();
void ps2_projection_push(unsigned int matrixMode);
void ps2_projection_pop(unsigned int matrixMode);

#endif // PS2_PLATFORM
