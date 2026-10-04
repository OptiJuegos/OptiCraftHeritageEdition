#pragma once

// GL-shaped matrix stacks for the 3DS render backend.
//
// Everything here is deliberately OpenGL: matrices are float[16] in GL's
// column-major order (element (row r, column c) lives at m[c*4+r]), the
// post-multiplication order of glMultMatrix, and the projection/model-view
// split the game drives through renderMatrixMode(). The conversion into the
// PICA's own row layout happens once per draw, in DsRender.cpp -- keeping the
// stacks GL-native is what lets renderGetMatrix() hand ActiveRenderInfo
// exactly the numbers it would have read off the desktop renderer.

#include "platform/RenderAPI.h"

namespace ds
{
namespace matrix
{

// Which stack the mutating calls below act on.
void setMode(RenderMatrixMode mode);

// The mode those calls would act on right now. The orientation probe in
// DsRender.cpp is the first reader -- it needs to restore the mode it found
// after borrowing the stacks for its own matrices; the game itself always
// re-selects a mode before touching them.
RenderMatrixMode currentMode();

void loadIdentity();
void push();
void pop();

void translate(float x, float y, float z);
void rotate(float degrees, float x, float y, float z);
void scale(float x, float y, float z);
void ortho(double left, double right, double bottom, double top,
           double nearValue, double farValue);
void frustum(double left, double right, double bottom, double top,
             double nearValue, double farValue);

// Copies the top of the requested stack into values[0..15], GL column-major.
void get(RenderMatrixQuery query, float* values);

// Tops of both stacks, as the shader wants them. The projection is the GL one;
// DsRender.cpp applies the fixed PICA tilt on top of it before uploading.
const float* projectionTop();
const float* modelViewTop();

// Drop every push -- renderResetResources() reinitialises the pipeline.
void resetAll();

} // namespace matrix
} // namespace ds
