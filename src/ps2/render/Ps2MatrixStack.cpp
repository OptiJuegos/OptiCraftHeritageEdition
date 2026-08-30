#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/render/Ps2MatrixStack.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <math.h>

#include "ps2/render/Ps2RenderApi.h"

namespace
{
	static const int kStackDepth = 32;
	static const unsigned int kModelView = ps2RenderValue(Ps2RenderMatrixMode::ModelView);
	static const unsigned int kProjection = ps2RenderValue(Ps2RenderMatrixMode::Projection);
	static const unsigned int kTexture = ps2RenderValue(Ps2RenderMatrixMode::Texture);
	static const unsigned int kQueryModelView = ps2RenderValue(Ps2RenderMatrixQuery::ModelView);
	static const unsigned int kQueryProjection = ps2RenderValue(Ps2RenderMatrixQuery::Projection);
	static const unsigned int kQueryTexture = ps2RenderValue(Ps2RenderMatrixQuery::Texture);

	// 16-byte aligned so ps2_matrix_multiply's VU0 path can lqc2/sqc2 these
	// directly (see multiplyVu0 below). Every element of an aligned array of a
	// 64-byte type stays aligned, so the whole push/pop stack qualifies.
	alignas(16) static Ps2Mat4 s_projection[kStackDepth] = {{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	}};
	alignas(16) static Ps2Mat4 s_modelView[kStackDepth] = {{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	}};
	alignas(16) static Ps2Mat4 s_texture = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	static int s_modelViewTop = 0;
	static int s_modelViewOverflow = 0;
	static int s_projectionTop = 0;
	static int s_projectionOverflow = 0;
	static unsigned int s_mode = kModelView;

	static float* currentMatrix()
	{
		if (s_mode == kProjection)
			return s_projection[s_projectionTop];
		if (s_mode == kTexture)
			return s_texture;
		return s_modelView[s_modelViewTop];
	}

#if defined(__mips__)
	static inline bool isAligned16(const void* pointer)
	{
		return (reinterpret_cast<std::uintptr_t>(pointer) & 0xFu) == 0u;
	}

	// Same accumulator-free idiom as ps2_vu0_xform4 (Ps2ClipGuard.h): a 4x4
	// column-major multiply is just `left` transforming each of `right`'s four
	// columns as a vector, so this is that transform applied to four vectors at
	// once instead of one. The matrix is loaded once for the whole call instead
	// of the 64 scalar loads/stores the loop below needs.
	static inline void multiplyVu0(Ps2Mat4 out, const Ps2Mat4 left, const Ps2Mat4 right)
	{
		__asm__ __volatile__ (
			"lqc2       $vf16, 0x00(%[l])       \n"
			"lqc2       $vf17, 0x10(%[l])       \n"
			"lqc2       $vf18, 0x20(%[l])       \n"
			"lqc2       $vf19, 0x30(%[l])       \n"
			"lqc2       $vf20, 0x00(%[r])       \n"
			"lqc2       $vf21, 0x10(%[r])       \n"
			"lqc2       $vf22, 0x20(%[r])       \n"
			"lqc2       $vf23, 0x30(%[r])       \n"
			"vmulx.xyzw $vf24, $vf16, $vf20x    \n"
			"vmulx.xyzw $vf25, $vf16, $vf21x    \n"
			"vmulx.xyzw $vf26, $vf16, $vf22x    \n"
			"vmulx.xyzw $vf27, $vf16, $vf23x    \n"
			"vmuly.xyzw $vf28, $vf17, $vf20y    \n"
			"vmuly.xyzw $vf29, $vf17, $vf21y    \n"
			"vmuly.xyzw $vf30, $vf17, $vf22y    \n"
			"vmuly.xyzw $vf31, $vf17, $vf23y    \n"
			"vadd.xyzw  $vf24, $vf24, $vf28     \n"
			"vadd.xyzw  $vf25, $vf25, $vf29     \n"
			"vadd.xyzw  $vf26, $vf26, $vf30     \n"
			"vadd.xyzw  $vf27, $vf27, $vf31     \n"
			"vmulz.xyzw $vf28, $vf18, $vf20z    \n"
			"vmulz.xyzw $vf29, $vf18, $vf21z    \n"
			"vmulz.xyzw $vf30, $vf18, $vf22z    \n"
			"vmulz.xyzw $vf31, $vf18, $vf23z    \n"
			"vadd.xyzw  $vf24, $vf24, $vf28     \n"
			"vadd.xyzw  $vf25, $vf25, $vf29     \n"
			"vadd.xyzw  $vf26, $vf26, $vf30     \n"
			"vadd.xyzw  $vf27, $vf27, $vf31     \n"
			"vmulw.xyzw $vf28, $vf19, $vf20w    \n"
			"vmulw.xyzw $vf29, $vf19, $vf21w    \n"
			"vmulw.xyzw $vf30, $vf19, $vf22w    \n"
			"vmulw.xyzw $vf31, $vf19, $vf23w    \n"
			"vadd.xyzw  $vf24, $vf24, $vf28     \n"
			"vadd.xyzw  $vf25, $vf25, $vf29     \n"
			"vadd.xyzw  $vf26, $vf26, $vf30     \n"
			"vadd.xyzw  $vf27, $vf27, $vf31     \n"
			"sqc2       $vf24, 0x00(%[o])       \n"
			"sqc2       $vf25, 0x10(%[o])       \n"
			"sqc2       $vf26, 0x20(%[o])       \n"
			"sqc2       $vf27, 0x30(%[o])       \n"
			:
			: [l] "r" (left), [r] "r" (right), [o] "r" (out)
			: "memory");
	}
#endif
}

void ps2_matrix_identity(Ps2Mat4 matrix)
{
	matrix[0] = 1.0f; matrix[1] = 0.0f; matrix[2] = 0.0f; matrix[3] = 0.0f;
	matrix[4] = 0.0f; matrix[5] = 1.0f; matrix[6] = 0.0f; matrix[7] = 0.0f;
	matrix[8] = 0.0f; matrix[9] = 0.0f; matrix[10] = 1.0f; matrix[11] = 0.0f;
	matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = 0.0f; matrix[15] = 1.0f;
}

void ps2_matrix_copy(Ps2Mat4 destination, const Ps2Mat4 source)
{
	std::memcpy(destination, source, 16 * sizeof(float));
}

void ps2_matrix_multiply(Ps2Mat4 out, const Ps2Mat4 left, const Ps2Mat4 right)
{
#if defined(__mips__)
	// lqc2/sqc2 fault on an unaligned address. Every internal caller in this
	// file uses alignas(16) storage and always takes this path; the alignment
	// check only has to catch a public-API caller (e.g. an external
	// glMultMatrixf pointer) that cannot make the same guarantee, and falls
	// back to the scalar loop below for that rare case instead of risking a
	// COP2 address-error exception on real hardware.
	if (isAligned16(out) && isAligned16(left) && isAligned16(right))
	{
		multiplyVu0(out, left, right);
		return;
	}
#endif
	for (int column = 0; column < 4; ++column)
	{
		for (int row = 0; row < 4; ++row)
		{
			float value = 0.0f;
			for (int component = 0; component < 4; ++component)
				value += left[component * 4 + row] * right[column * 4 + component];
			out[column * 4 + row] = value;
		}
	}
}

void ps2_matrix_transform_point(const Ps2Mat4 matrix,
	float x, float y, float z,
	float& outX, float& outY, float& outZ, float& outW)
{
	outX = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
	outY = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
	outZ = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
	outW = matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15];
}

void ps2_matrix_set_mode(unsigned int mode)
{
	s_mode = mode;
}

unsigned int ps2_matrix_current_mode()
{
	return s_mode;
}

bool ps2_matrix_load_identity()
{
	ps2_matrix_identity(currentMatrix());
	return s_mode == kProjection;
}

void ps2_matrix_push()
{
	if (s_mode == kProjection)
	{
		if (s_projectionOverflow > 0 || s_projectionTop >= kStackDepth - 1)
		{
			++s_projectionOverflow;
			MC_LOG_WARN("render", "[PS2] projection matrix stack full (top=%d, overflow=%d)\n",
				s_projectionTop, s_projectionOverflow);
			return;
		}
		ps2_matrix_copy(s_projection[s_projectionTop + 1], s_projection[s_projectionTop]);
		++s_projectionTop;
		return;
	}
	if (s_mode == kTexture)
		return;
	if (s_modelViewOverflow > 0 || s_modelViewTop >= kStackDepth - 1)
	{
		++s_modelViewOverflow;
		MC_LOG_WARN("render", "[PS2] matrix stack full (top=%d, overflow=%d)\n",
			s_modelViewTop, s_modelViewOverflow);
		return;
	}

	ps2_matrix_copy(s_modelView[s_modelViewTop + 1], s_modelView[s_modelViewTop]);
	++s_modelViewTop;
}

void ps2_matrix_pop()
{
	if (s_mode == kProjection)
	{
		if (s_projectionOverflow > 0)
		{
			--s_projectionOverflow;
			return;
		}
		if (s_projectionTop > 0)
			--s_projectionTop;
		return;
	}
	if (s_mode == kTexture)
		return;
	if (s_modelViewOverflow > 0)
	{
		--s_modelViewOverflow;
		return;
	}
	if (s_modelViewTop > 0)
		--s_modelViewTop;
}

void ps2_matrix_get(unsigned int query, float* values)
{
	if (values == nullptr)
		return;
	if (query == kQueryProjection)
		ps2_matrix_copy(values, s_projection[s_projectionTop]);
	else if (query == kQueryModelView)
		ps2_matrix_copy(values, s_modelView[s_modelViewTop]);
	else if (query == kQueryTexture)
		ps2_matrix_copy(values, s_texture);
}

void ps2_matrix_multiply_current(const Ps2Mat4 matrix)
{
	alignas(16) Ps2Mat4 result;
	ps2_matrix_multiply(result, currentMatrix(), matrix);
	ps2_matrix_copy(currentMatrix(), result);
}

void ps2_matrix_translate(float x, float y, float z)
{
	alignas(16) Ps2Mat4 translation;
	ps2_matrix_identity(translation);
	translation[12] = x;
	translation[13] = y;
	translation[14] = z;
	ps2_matrix_multiply_current(translation);
}

void ps2_matrix_scale(float x, float y, float z)
{
	alignas(16) Ps2Mat4 scale;
	ps2_matrix_identity(scale);
	scale[0] = x;
	scale[5] = y;
	scale[10] = z;
	ps2_matrix_multiply_current(scale);
}

void ps2_matrix_rotate(float angle, float axisX, float axisY, float axisZ)
{
	const float length = sqrtf(axisX * axisX + axisY * axisY + axisZ * axisZ);
	if (length < 1e-6f)
		return;
	axisX /= length;
	axisY /= length;
	axisZ /= length;

	const float radians = angle * (3.14159265f / 180.0f);
	const float cosine = cosf(radians);
	const float sine = sinf(radians);
	const float inverseCosine = 1.0f - cosine;
	alignas(16) Ps2Mat4 rotation;
	rotation[0] = cosine + axisX * axisX * inverseCosine;
	rotation[4] = axisX * axisY * inverseCosine - axisZ * sine;
	rotation[8] = axisX * axisZ * inverseCosine + axisY * sine;
	rotation[12] = 0.0f;
	rotation[1] = axisY * axisX * inverseCosine + axisZ * sine;
	rotation[5] = cosine + axisY * axisY * inverseCosine;
	rotation[9] = axisY * axisZ * inverseCosine - axisX * sine;
	rotation[13] = 0.0f;
	rotation[2] = axisZ * axisX * inverseCosine - axisY * sine;
	rotation[6] = axisZ * axisY * inverseCosine + axisX * sine;
	rotation[10] = cosine + axisZ * axisZ * inverseCosine;
	rotation[14] = 0.0f;
	rotation[3] = 0.0f;
	rotation[7] = 0.0f;
	rotation[11] = 0.0f;
	rotation[15] = 1.0f;
	ps2_matrix_multiply_current(rotation);
}

const float* ps2_matrix_projection()
{
	return s_projection[s_projectionTop];
}

const float* ps2_matrix_model_view()
{
	return s_modelView[s_modelViewTop];
}

extern "C" int ps2_dbg_mv_top()
{
	return s_modelViewTop;
}

#endif // PS2_PLATFORM
