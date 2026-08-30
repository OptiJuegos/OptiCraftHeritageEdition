#pragma once

#if defined(WII_PLATFORM)

#include <gccore.h>

// Portable-C matrix helpers for the Wii render backend.
//
// On GEKKO libogc maps every gu*() name to its paired-single ps_gu*() routine.
// Those routines load the Unit01 constant out of libogc's writable .sdata, and
// this port has observed that constant being overwritten at runtime: once it is,
// the paired-single path injects translation into the matrix basis and pushes
// GUI and world geometry outside the clip volume. Routing every matrix
// operation the renderer performs through libogc's portable C implementations
// removes that dependency -- they build their constants from immediates.
//
// This is the single authority for that decision. Call sites use the helpers
// below instead of gu*() so the paired-single path cannot come back one
// function at a time.
//
// guMtx44Identity(), guMtx44Copy(), guMtx44Inverse(), guOrtho(), guFrustum()
// and guPerspective() have no paired-single variant, so they are safe to call
// directly and are deliberately not wrapped here.

inline void wiiMtxIdentity(Mtx mt)
{
    c_guMtxIdentity(mt);
}

inline void wiiMtxConcat(const Mtx a, const Mtx b, Mtx ab)
{
    c_guMtxConcat(a, b, ab);
}

inline void wiiMtx44Concat(const Mtx44 a, const Mtx44 b, Mtx44 ab)
{
    c_guMtx44Concat(a, b, ab);
}

inline void wiiMtxTrans(Mtx mt, f32 xT, f32 yT, f32 zT)
{
    c_guMtxTrans(mt, xT, yT, zT);
}

inline void wiiMtxScale(Mtx mt, f32 xS, f32 yS, f32 zS)
{
    c_guMtxScale(mt, xS, yS, zS);
}

inline void wiiMtxApplyTrans(const Mtx src, Mtx dst, f32 xT, f32 yT, f32 zT)
{
    c_guMtxApplyTrans(src, dst, xT, yT, zT);
}

inline void wiiMtxApplyScale(const Mtx src, Mtx dst, f32 xS, f32 yS, f32 zS)
{
    c_guMtxApplyScale(src, dst, xS, yS, zS);
}

inline void wiiMtxRotAxisDeg(Mtx mt, guVector* axis, f32 degrees)
{
    c_guMtxRotAxisRad(mt, axis, DegToRad(degrees));
}

inline u32 wiiMtxInverse(const Mtx src, Mtx inv)
{
    return c_guMtxInverse(src, inv);
}

inline void wiiMtxTranspose(const Mtx src, Mtx xPose)
{
    c_guMtxTranspose(src, xPose);
}

inline void wiiVecMultiply(const Mtx mt, const guVector* src, guVector* dst)
{
    c_guVecMultiply(mt, src, dst);
}

inline void wiiVecMultiplySR(const Mtx mt, const guVector* src, guVector* dst)
{
    c_guVecMultiplySR(mt, src, dst);
}

#endif
