// DsMatrix.cpp -- GL-compatible matrix stacks for the 3DS render backend.
//
// Faithful reimplementation of the fixed-function stack the shared game code
// drives (glMatrixMode/glPushMatrix/glOrtho/glRotatef and friends). Two
// details are load-bearing rather than stylistic:
//
//   * Column-major storage. GL names a matrix by column, m[c*4+r]. Every
//     un-project in ActiveRenderInfo and every Tessellator helper assumes it,
//     so get() must hand back exactly that layout.
//   * Post-multiplication. glMultMatrix(right) does current = current * right,
//     which is why the ortho/frustum/rotate builders are multiplied on the
//     right and not the left. Getting this backwards renders transposed
//     geometry that still "looks roughly right" in an ortho menu, which is the
//     worst kind of wrong.
//
// The PICA's clip space is different from GL's (see DsShader.v.pica), but that
// difference is a property of the projection, not of these stacks: it is
// applied as one fixed matrix at upload time so the recorded GL projection
// stays readable back through renderGetMatrix().

#include "3ds/render/DsMatrix.h"

#include <cmath>
#include <cstring>

namespace ds
{
namespace matrix
{
namespace
{
// The three GL stacks, plus their push depths. Depth is bounded by the game
// (it pairs every push with a pop), but the arrays are generous so an
// unbalanced path degrades to "ignored" instead of scribbling over .bss.
constexpr int kStackDepth = 32;

struct Stack
{
	float items[kStackDepth][16];
	int top;
};

Stack s_modelView;
Stack s_projection;
Stack s_texture;
Stack* s_mode = &s_modelView;

void identity(float* m)
{
	std::memset(m, 0, 16 * sizeof(float));
	m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// out = left * right, both column-major.
void multiply(float* out, const float* left, const float* right)
{
	float tmp[16];
	for (int c = 0; c < 4; ++c)
	{
		for (int r = 0; r < 4; ++r)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k)
				sum += left[k * 4 + r] * right[c * 4 + k];
			tmp[c * 4 + r] = sum;
		}
	}
	std::memcpy(out, tmp, sizeof(tmp));
}

// current = current * right.
void postMultiply(const float* right)
{
	float combined[16];
	multiply(combined, s_mode->items[s_mode->top], right);
	std::memcpy(s_mode->items[s_mode->top], combined, sizeof(combined));
}

Stack* stackFor(RenderMatrixQuery query)
{
	switch (query)
	{
		case RenderMatrixQuery::Projection: return &s_projection;
		case RenderMatrixQuery::Texture:    return &s_texture;
		case RenderMatrixQuery::ModelView:  break;
	}
	return &s_modelView;
}
} // namespace

void setMode(RenderMatrixMode mode)
{
	switch (mode)
	{
		case RenderMatrixMode::Projection: s_mode = &s_projection; break;
		case RenderMatrixMode::Texture:    s_mode = &s_texture;    break;
		case RenderMatrixMode::ModelView:  s_mode = &s_modelView;  break;
	}
}

RenderMatrixMode currentMode()
{
	if (s_mode == &s_projection)
		return RenderMatrixMode::Projection;
	if (s_mode == &s_texture)
		return RenderMatrixMode::Texture;
	return RenderMatrixMode::ModelView;
}

void loadIdentity()
{
	identity(s_mode->items[s_mode->top]);
}

void push()
{
	if (s_mode->top + 1 >= kStackDepth)
		return; // Unbalanced push: ignore it rather than overflow.
	std::memcpy(s_mode->items[s_mode->top + 1], s_mode->items[s_mode->top],
	            sizeof(s_mode->items[0]));
	++s_mode->top;
}

void pop()
{
	if (s_mode->top > 0)
		--s_mode->top;
}

void translate(float x, float y, float z)
{
	float t[16];
	identity(t);
	t[12] = x;
	t[13] = y;
	t[14] = z;
	postMultiply(t);
}

void rotate(float degrees, float x, float y, float z)
{
	const double len = std::sqrt(static_cast<double>(x) * x +
	                             static_cast<double>(y) * y +
	                             static_cast<double>(z) * z);
	if (len == 0.0)
		return; // glRotatef with a zero axis is a documented no-op.

	x = static_cast<float>(x / len);
	y = static_cast<float>(y / len);
	z = static_cast<float>(z / len);

	const double angle = degrees * (3.14159265358979323846 / 180.0);
	const double c = std::cos(angle);
	const double s = std::sin(angle);
	const double ic = 1.0 - c;

	float r[16];
	identity(r);
	r[0]  = static_cast<float>(c + x * x * ic);
	r[1]  = static_cast<float>(y * x * ic + z * s);
	r[2]  = static_cast<float>(z * x * ic - y * s);
	r[4]  = static_cast<float>(x * y * ic - z * s);
	r[5]  = static_cast<float>(c + y * y * ic);
	r[6]  = static_cast<float>(z * y * ic + x * s);
	r[8]  = static_cast<float>(x * z * ic + y * s);
	r[9]  = static_cast<float>(y * z * ic - x * s);
	r[10] = static_cast<float>(c + z * z * ic);
	postMultiply(r);
}

void scale(float x, float y, float z)
{
	float s[16];
	identity(s);
	s[0] = x;
	s[5] = y;
	s[10] = z;
	postMultiply(s);
}

void ortho(double left, double right, double bottom, double top,
           double nearValue, double farValue)
{
	const double dr = right - left;
	const double dt = top - bottom;
	const double df = farValue - nearValue;
	if (dr == 0.0 || dt == 0.0 || df == 0.0)
		return; // Degenerate box; glOrtho would produce infinities.

	float m[16];
	std::memset(m, 0, sizeof(m));
	m[0]  = static_cast<float>(2.0 / dr);
	m[5]  = static_cast<float>(2.0 / dt);
	m[10] = static_cast<float>(-2.0 / df);
	m[12] = static_cast<float>(-(right + left) / dr);
	m[13] = static_cast<float>(-(top + bottom) / dt);
	m[14] = static_cast<float>(-(farValue + nearValue) / df);
	m[15] = 1.0f;
	postMultiply(m);
}

void frustum(double left, double right, double bottom, double top,
             double nearValue, double farValue)
{
	const double dr = right - left;
	const double dt = top - bottom;
	const double df = farValue - nearValue;
	if (dr == 0.0 || dt == 0.0 || df == 0.0 || nearValue == 0.0)
		return;

	float m[16];
	std::memset(m, 0, sizeof(m));
	m[0]  = static_cast<float>(nearValue * 2.0 / dr);
	m[5]  = static_cast<float>(nearValue * 2.0 / dt);
	m[8]  = static_cast<float>((right + left) / dr);
	m[9]  = static_cast<float>((top + bottom) / dt);
	m[10] = static_cast<float>(-(farValue + nearValue) / df);
	m[11] = -1.0f;
	m[14] = static_cast<float>(-2.0 * farValue * nearValue / df);
	postMultiply(m);
}

void get(RenderMatrixQuery query, float* values)
{
	if (values == nullptr)
		return;
	std::memcpy(values, stackFor(query)->items[stackFor(query)->top],
	            16 * sizeof(float));
}

const float* projectionTop()
{
	return s_projection.items[s_projection.top];
}

const float* modelViewTop()
{
	return s_modelView.items[s_modelView.top];
}

void resetAll()
{
	s_modelView.top = 0;
	s_projection.top = 0;
	s_texture.top = 0;
	s_mode = &s_modelView;
	identity(s_modelView.items[0]);
	identity(s_projection.items[0]);
	identity(s_texture.items[0]);
}

} // namespace matrix
} // namespace ds
