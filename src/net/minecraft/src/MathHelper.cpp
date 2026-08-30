#include "MathHelper.h"
#include "java/Math.h"
#include "java/Random.h"
#include "java/Arithmetic.h"

#include <cmath>

float MathHelper::SIN_TABLE[MathHelper::SIN_TABLE_SIZE];

namespace
{
	// Derived from the tuning macro rather than from MathHelper::SIN_TABLE_SIZE,
	// which is private to the class and not reachable from namespace scope.
	constexpr int_t SIN_ENTRIES = 1 << PLATFORM_SIN_TABLE_BITS;

	// Radians -> table index. Vanilla's literal is kept verbatim for the
	// 65536-entry table so a build at the default size stays bit-identical to
	// the original; any other size derives the same quantity, size / (2*pi).
	constexpr float SIN_TABLE_SCALE = SIN_ENTRIES == 0x10000
		? 10430.378f
		: (float)((double)SIN_ENTRIES / (3.1415926535897931 * 2.0));

	// Quarter turn, which is what turns the sine table into a cosine lookup.
	// 16384 at the vanilla size; exact at every power of two.
	constexpr float SIN_TABLE_QUARTER = (float)(SIN_ENTRIES / 4);
}

bool MathHelper::initSinTable()
{
	for (int_t i = 0; i < SIN_TABLE_SIZE; i++)
		SIN_TABLE[i] = (float)JavaMath::sin(((double)i * 3.1415926535897931 * 2.0) / (double)SIN_TABLE_SIZE);
	return true;
}

bool MathHelper::sinTableInitialized = MathHelper::initSinTable();

float MathHelper::sin(float f)
{
	return SIN_TABLE[JavaArithmetic::floatToInt(f * SIN_TABLE_SCALE) & SIN_TABLE_MASK];
}

float MathHelper::cos(float f)
{
	return SIN_TABLE[JavaArithmetic::floatToInt(f * SIN_TABLE_SCALE + SIN_TABLE_QUARTER) & SIN_TABLE_MASK];
}

float MathHelper::sqrt_float(float f)
{
#if PLATFORM_PS2
	return std::sqrt(f);
#else
	return (float)JavaMath::sqrt(f);
#endif
}

float MathHelper::sqrt_double(double d)
{
#if PLATFORM_PS2
	// The result is a float either way, so the only question is where the
	// rounding happens. JavaMath::sqrt is fdlibm's software double kernel, which
	// the R5900 has no hardware to shorten; narrowing first turns the whole call
	// into one sqrt.s. Rounding the argument before the root instead of the
	// result after it can move the answer by one float ulp, which is the same
	// class of trade the other PS2 float knobs accept.
	return std::sqrt((float)d);
#else
	return (float)JavaMath::sqrt(d);
#endif
}

int_t MathHelper::floor_float(float f)
{
	int_t i = JavaArithmetic::floatToInt(f);
	return f < (float)i ? JavaArithmetic::intSub(i, 1) : i;
}

int_t MathHelper::func_40346_b(double d)
{
	const int_t shifted = JavaArithmetic::doubleToInt(d + 1024.0);
	return JavaArithmetic::intSub(shifted, 1024);
}

int_t MathHelper::floor_double(double d)
{
	int_t i = JavaArithmetic::doubleToInt(d);
	return d < (double)i ? JavaArithmetic::intSub(i, 1) : i;
}

long_t MathHelper::floor_double_long(double d)
{
	long_t i = JavaArithmetic::doubleToLong(d);
	return d < (double)i ? JavaArithmetic::longSub(i, 1LL) : i;
}

float MathHelper::abs(float f)
{
	return f >= 0.0f ? f : -f;
}

int_t MathHelper::clamp_int(int_t value, int_t minimum, int_t maximum)
{
	return value < minimum ? minimum : (value > maximum ? maximum : value);
}

double MathHelper::abs_max(double d, double d1)
{
	if (d < 0.0)
		d = -d;
	if (d1 < 0.0)
		d1 = -d1;
	return d > d1 ? d : d1;
}

float MathHelper::clamp_float(float value, float minimum, float maximum)
{
	if (value < minimum)
		return minimum;
	if (value > maximum)
		return maximum;
	return value;
}

int_t MathHelper::bucketInt(int_t i, int_t j)
{
	if (i < 0)
	{
		const int_t positive = JavaArithmetic::intSub(0, i);
		const int_t quotient = JavaArithmetic::intDiv(JavaArithmetic::intSub(positive, 1), j);
		return JavaArithmetic::intSub(JavaArithmetic::intSub(0, quotient), 1);
	}
	else
	{
		return JavaArithmetic::intDiv(i, j);
	}
}

bool MathHelper::stringNullOrLengthZero(const jstring &s)
{
	return s.isNull() || s.length() == 0;
}

int_t MathHelper::getRandomIntegerInRange(Random &random, int_t minimum, int_t maximum)
{
	if (minimum >= maximum)
		return minimum;
	const int_t range = JavaArithmetic::intAdd(JavaArithmetic::intSub(maximum, minimum), 1);
	return JavaArithmetic::intAdd(random.nextInt(range), minimum);
}
