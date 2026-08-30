#pragma once

#ifdef PS2_PLATFORM

typedef float Ps2Mat4[16];

void ps2_matrix_identity(Ps2Mat4 matrix);
void ps2_matrix_copy(Ps2Mat4 destination, const Ps2Mat4 source);
void ps2_matrix_multiply(Ps2Mat4 out, const Ps2Mat4 left, const Ps2Mat4 right);
void ps2_matrix_transform_point(const Ps2Mat4 matrix,
	float x, float y, float z,
	float& outX, float& outY, float& outZ, float& outW);

void ps2_matrix_set_mode(unsigned int mode);
unsigned int ps2_matrix_current_mode();
bool ps2_matrix_load_identity();
void ps2_matrix_push();
void ps2_matrix_pop();
void ps2_matrix_get(unsigned int query, float* values);
void ps2_matrix_translate(float x, float y, float z);
void ps2_matrix_scale(float x, float y, float z);
void ps2_matrix_rotate(float angle, float axisX, float axisY, float axisZ);
void ps2_matrix_multiply_current(const Ps2Mat4 matrix);

const float* ps2_matrix_projection();
const float* ps2_matrix_model_view();

#endif // PS2_PLATFORM
