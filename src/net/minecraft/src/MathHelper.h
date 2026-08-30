#pragma once

#include "java/Type.h"
#include "java/String.h"
#include "platform/PlatformTuning.h"

class Random;

// net.minecraft.src.MathHelper
class MathHelper
{
public:
	MathHelper() = default;

	static float sin(float f);
	static float cos(float f);
	static float sqrt_float(float f);
	static float sqrt_double(double d);
	static int_t floor_float(float f);
	static int_t func_40346_b(double d);
	static int_t floor_double(double d);
	static long_t floor_double_long(double d);
	static float abs(float f);
	static int_t clamp_int(int_t value, int_t minimum, int_t maximum);
	static double abs_max(double d, double d1);
	static float clamp_float(float value, float minimum, float maximum);
	static int_t bucketInt(int_t i, int_t j);
	static bool stringNullOrLengthZero(const jstring &s);
	static int_t getRandomIntegerInRange(Random &random, int_t minimum, int_t maximum);

private:
	// Sized by PLATFORM_SIN_TABLE_BITS rather than vanilla's hardcoded 0x10000.
	// The desktop profile sets 16, which is the original 65536-entry table; see
	// the knob's comment for why a console shrinks it.
	static constexpr int_t SIN_TABLE_SIZE = 1 << PLATFORM_SIN_TABLE_BITS;
	static constexpr int_t SIN_TABLE_MASK = SIN_TABLE_SIZE - 1;

	static float SIN_TABLE[SIN_TABLE_SIZE];
	static bool initSinTable();
	static bool sinTableInitialized;
};
