#pragma once

#include "java/Type.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace JavaArithmetic
{

inline byte_t byteFromBits(ubyte_t bits)
{
	byte_t value;
	static_assert(sizeof(value) == sizeof(bits), "Unexpected Java byte size");
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

inline short_t shortFromBits(ushort_t bits)
{
	short_t value;
	static_assert(sizeof(value) == sizeof(bits), "Unexpected Java short size");
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

inline int_t intFromBits(uint_t bits)
{
	int_t value;
	static_assert(sizeof(value) == sizeof(bits), "Unexpected Java int size");
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

inline long_t longFromBits(ulong_t bits)
{
	long_t value;
	static_assert(sizeof(value) == sizeof(bits), "Unexpected Java long size");
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

inline int_t intAdd(int_t lhs, int_t rhs)
{
	return intFromBits(static_cast<uint_t>(lhs) + static_cast<uint_t>(rhs));
}

inline int_t intSub(int_t lhs, int_t rhs)
{
	return intFromBits(static_cast<uint_t>(lhs) - static_cast<uint_t>(rhs));
}

inline int_t intAbs(int_t value)
{
	return value < 0 ? intSub(0, value) : value;
}

inline int_t intMul(int_t lhs, int_t rhs)
{
	return intFromBits(static_cast<uint_t>(lhs) * static_cast<uint_t>(rhs));
}

inline int_t intDiv(int_t lhs, int_t rhs)
{
	if (rhs == 0)
		throw std::domain_error("Java int division by zero");
	if (lhs == std::numeric_limits<int_t>::min() && rhs == -1)
		return std::numeric_limits<int_t>::min();
	return lhs / rhs;
}

inline long_t longAdd(long_t lhs, long_t rhs)
{
	return longFromBits(static_cast<ulong_t>(lhs) + static_cast<ulong_t>(rhs));
}

inline long_t longSub(long_t lhs, long_t rhs)
{
	return longFromBits(static_cast<ulong_t>(lhs) - static_cast<ulong_t>(rhs));
}

inline long_t longMul(long_t lhs, long_t rhs)
{
	return longFromBits(static_cast<ulong_t>(lhs) * static_cast<ulong_t>(rhs));
}

inline int_t intShl(int_t value, int_t distance)
{
	const uint_t shift = static_cast<uint_t>(distance) & 31u;
	return intFromBits(static_cast<uint_t>(value) << shift);
}

inline int_t intShr(int_t value, int_t distance)
{
	const uint_t shift = static_cast<uint_t>(distance) & 31u;
	uint_t bits = static_cast<uint_t>(value);
	if (shift == 0)
		return value;
	if ((bits & 0x80000000u) != 0)
		bits = (bits >> shift) | (~uint_t(0) << (32u - shift));
	else
		bits >>= shift;
	return intFromBits(bits);
}

inline int_t intUshr(int_t value, int_t distance)
{
	const uint_t shift = static_cast<uint_t>(distance) & 31u;
	return intFromBits(static_cast<uint_t>(value) >> shift);
}

inline long_t longShl(long_t value, int_t distance)
{
	const uint_t shift = static_cast<uint_t>(distance) & 63u;
	return longFromBits(static_cast<ulong_t>(value) << shift);
}

inline long_t longShr(long_t value, int_t distance)
{
	const uint_t shift = static_cast<uint_t>(distance) & 63u;
	ulong_t bits = static_cast<ulong_t>(value);
	if (shift == 0)
		return value;
	if ((bits & 0x8000000000000000ULL) != 0)
		bits = (bits >> shift) | (~ulong_t(0) << (64u - shift));
	else
		bits >>= shift;
	return longFromBits(bits);
}

inline long_t longUshr(long_t value, int_t distance)
{
	const uint_t shift = static_cast<uint_t>(distance) & 63u;
	return longFromBits(static_cast<ulong_t>(value) >> shift);
}

inline int_t doubleToInt(double value)
{
	if (std::isnan(value))
		return 0;
	if (value >= static_cast<double>(std::numeric_limits<int_t>::max()))
		return std::numeric_limits<int_t>::max();
	if (value <= static_cast<double>(std::numeric_limits<int_t>::min()))
		return std::numeric_limits<int_t>::min();
	return static_cast<int_t>(value);
}

// JLS 5.1.3 narrowing of float to int, evaluated entirely in single precision.
//
// This used to widen to double and defer to doubleToInt, which is exact but
// costs five libgcc soft-float calls on a CPU without a double FPU
// (__extendsfdf2, __unorddf2, __gedf2, __ledf2, __fixdfsi). MathHelper::sin and
// cos are the hot callers -- every model rotation and camera angle goes through
// one -- so the widening alone dominated them on the R5900.
//
// The single-precision form returns the same int for every float input. There
// is no float between 2147483520.0f (2^31 - 128) and 2147483648.0f (2^31), so
// the two clamp boundaries below select exactly the same inputs the double
// comparisons against INT_MAX/INT_MIN selected:
//   2147483520.0f  -> below both thresholds, truncates to 2147483520 either way
//   2147483648.0f  -> at/above both thresholds, clamps to INT_MAX either way
// and -2147483648.0f is exactly representable, so the negative clamp matches
// term for term. NaN is still mapped to 0 by the same rule.
inline int_t floatToInt(float value)
{
	if (std::isnan(value))
		return 0;
	// Written as literals, not as (float)INT_MAX: that cast rounds up to 2^31
	// and reads as if it were the representable maximum, which it is not.
	if (value >= 2147483648.0f)
		return std::numeric_limits<int_t>::max();
	if (value <= -2147483648.0f)
		return std::numeric_limits<int_t>::min();
	return static_cast<int_t>(value);
}

inline long_t doubleToLong(double value)
{
	if (std::isnan(value))
		return 0;
	const long double wide = static_cast<long double>(value);
	if (wide >= static_cast<long double>(std::numeric_limits<long_t>::max()))
		return std::numeric_limits<long_t>::max();
	if (wide <= static_cast<long double>(std::numeric_limits<long_t>::min()))
		return std::numeric_limits<long_t>::min();
	return static_cast<long_t>(value);
}

inline long_t floatToLong(float value)
{
	return doubleToLong(static_cast<double>(value));
}

inline int_t longToInt(long_t value)
{
	return intFromBits(static_cast<uint_t>(static_cast<ulong_t>(value)));
}

inline int_t roundFloat(float value)
{
	// Stays in single precision for the same reason floatToInt does. Widening a
	// float to double is exact, and floor() of an exact value is that same value
	// at either width, so dropping the widening cannot change the result.
	const float shifted = value + 0.5f;
	return floatToInt(std::floor(shifted));
}

inline long_t roundDouble(double value)
{
	return doubleToLong(std::floor(value + 0.5));
}

}
