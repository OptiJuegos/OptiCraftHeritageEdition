#include "java/Random.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>

static constexpr ulong_t RANDOM_MUL = 0x5DEECE66DULL;
static constexpr ulong_t RANDOM_ADD = 0xBULL;
static constexpr ulong_t RANDOM_AND = (1ULL << 48) - 1ULL;

namespace
{
	int_t highestSetBit32(uint_t value)
	{
#if defined(__GNUC__) || defined(__clang__)
		return 31 - __builtin_clz(value);
#else
		int_t bit = -1;
		while (value != 0u)
		{
			value >>= 1;
			++bit;
		}
		return bit;
#endif
	}
}

// Sun/OpenJDK java.util.Random() no-arg constructor strategy used by the
// Java 6/7 era: a process-wide seed uniquifier mixed with System.nanoTime().
//
// Wii/PS2 are 32-bit targets and do not provide lock-free 64-bit atomics.
// Using std::atomic<uint64_t> there makes GCC emit __atomic_load_8 /
// __atomic_compare_exchange_8 libcalls, which are not linked by the normal
// console toolchains. The game constructs its unseeded Random instances on
// the main/game thread on these targets, while worldgen uses explicitly
// seeded Random objects, so a plain 64-bit uniquifier is sufficient there.
#if defined(WII_PLATFORM) || defined(PS2_PLATFORM) || defined(HW_RVL) || defined(GEKKO)
static ulong_t g_seedUniquifier = 8682522807148012ULL;

static ulong_t nextSeedUniquifier()
{
	g_seedUniquifier *= 181783497276652981ULL;
	return g_seedUniquifier;
}
#else
static std::atomic<ulong_t> g_seedUniquifier{8682522807148012ULL};

static ulong_t nextSeedUniquifier()
{
	ulong_t current = g_seedUniquifier.load(std::memory_order_relaxed);
	for (;;)
	{
		const ulong_t next = current * 181783497276652981ULL;
		if (g_seedUniquifier.compare_exchange_weak(current, next,
				std::memory_order_relaxed, std::memory_order_relaxed))
			return next;
	}
}
#endif

Random::Random()
{
	const auto now = std::chrono::steady_clock::now();
	const ulong_t nanos = static_cast<ulong_t>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
	setSeed(JavaArithmetic::longFromBits(nextSeedUniquifier() ^ nanos));
}

Random::Random(long_t set_seed)
{
	setSeed(set_seed);
}

void Random::setSeed(long_t set_seed)
{
	// Java long arithmetic wraps in two's complement before the 48-bit mask.
	const ulong_t seedBits = static_cast<ulong_t>(set_seed);
	seed = static_cast<long_t>((seedBits ^ RANDOM_MUL) & RANDOM_AND);
	haveNextNextGaussian = false;
	nextNextGaussian = 0.0;
}

int_t Random::next(int_t bits)
{
	const ulong_t seedBits = static_cast<ulong_t>(seed);
	seed = static_cast<long_t>((seedBits * RANDOM_MUL + RANDOM_ADD) & RANDOM_AND);
	return JavaArithmetic::intFromBits(static_cast<uint_t>(static_cast<ulong_t>(seed) >> (48 - bits)));
}

bool Random::nextBoolean()
{
	return next(1) == 1;
}

int_t Random::nextInt()
{
	return next(32);
}

int_t Random::nextInt(int_t bound)
{
	if (bound <= 0)
		throw std::invalid_argument("bound must be positive");

	const int_t m = bound - 1;
	int_t r = next(31);
	if ((bound & m) == 0)
	{
		// next(31) is non-negative, so this multiplication/shift is defined and
		// matches java.util.Random exactly.
		return static_cast<int_t>((static_cast<long_t>(bound) * r) >> 31);
	}

	for (;;)
	{
		const int_t u = r;
		r = u % bound;

		// Java int arithmetic wraps modulo 2^32.  The source expression
		// `u - r + m < 0` relies on that overflow for the rejection test; signed
		// overflow would be undefined in C++.  Reproduce the Java sign bit
		// explicitly in uint32_t.
		const std::uint32_t wrapped = static_cast<std::uint32_t>(u)
		                            - static_cast<std::uint32_t>(r)
		                            + static_cast<std::uint32_t>(m);
		if ((wrapped & 0x80000000u) == 0)
			return r;

		r = next(31);
	}
}

long_t Random::nextLong()
{
	// Java performs two's-complement long arithmetic here.  Building the bit
	// pattern in uint64_t avoids left-shifting a negative signed value.
	const std::int32_t hiSigned = static_cast<std::int32_t>(next(32));
	const std::int32_t loSigned = static_cast<std::int32_t>(next(32));
	std::uint64_t bits = static_cast<std::uint64_t>(static_cast<std::uint32_t>(hiSigned)) << 32;
	bits += static_cast<std::uint64_t>(static_cast<std::int64_t>(loSigned));
	long_t result;
	static_assert(sizeof(result) == sizeof(bits), "Unexpected Java long size");
	std::memcpy(&result, &bits, sizeof(result));
	return result;
}

float Random::nextFloat()
{
	return next(24) / static_cast<float>(1LL << 24);
}

double Random::nextDouble()
{
	const long_t high = next(26);
	const long_t low = next(27);
	return ((high << 27) + low) / static_cast<double>(1LL << 53);
}

float Random::nextDoubleFloat()
{
	// Java nextDouble() constructs an exact 53-bit binary fraction. Convert that
	// integer directly to IEEE-754 binary32 with round-to-nearest-even so this is
	// bit-identical to static_cast<float>(nextDouble()) without executing any
	// double-precision arithmetic on low-console targets.
	const ulong_t high = static_cast<ulong_t>(static_cast<uint_t>(next(26)));
	const ulong_t low = static_cast<ulong_t>(static_cast<uint_t>(next(27)));
	const ulong_t fraction53 = (high << 27) | low;
	if (fraction53 == 0ULL)
		return 0.0f;

	const uint_t upper = static_cast<uint_t>(fraction53 >> 32);
	const uint_t lower = static_cast<uint_t>(fraction53);
	const int_t topBit = upper != 0u
		? 32 + highestSetBit32(upper)
		: highestSetBit32(lower);
	uint_t exponent = static_cast<uint_t>(topBit + 74);
	ulong_t significand;

	if (topBit <= 23)
	{
		significand = fraction53 << (23 - topBit);
	}
	else
	{
		const int_t shift = topBit - 23;
		significand = fraction53 >> shift;
		const ulong_t remainderMask = (1ULL << shift) - 1ULL;
		const ulong_t remainder = fraction53 & remainderMask;
		const ulong_t halfway = 1ULL << (shift - 1);
		if (remainder > halfway || (remainder == halfway && (significand & 1ULL) != 0ULL))
		{
			++significand;
			if (significand == (1ULL << 24))
			{
				significand >>= 1;
				++exponent;
			}
		}
	}

	const uint_t bits = (exponent << 23) |
		static_cast<uint_t>(significand & ((1ULL << 23) - 1ULL));
	float result;
	static_assert(sizeof(result) == sizeof(bits), "Unexpected float size");
	std::memcpy(&result, &bits, sizeof(result));
	return result;
}

int_t Random::nextInt5()
{
	// Exact specialization of nextInt(5). Keeping the divisor constant lets the
	// compiler replace the hot modulo/division with its constant-divisor form.
	int_t r = next(31);
	for (;;)
	{
		const int_t u = r;
		r = u % 5;
		const std::uint32_t wrapped = static_cast<std::uint32_t>(u)
			- static_cast<std::uint32_t>(r) + 4u;
		if ((wrapped & 0x80000000u) == 0u)
			return r;
		r = next(31);
	}
}

int_t Random::nextIntDifference(int_t bound)
{
	const int_t first = nextInt(bound);
	const int_t second = nextInt(bound);
	return JavaArithmetic::intSub(first, second);
}

int_t Random::nextIntOffset(int_t base, int_t bound)
{
	const int_t first = nextInt(bound);
	const int_t second = nextInt(bound);
	return JavaArithmetic::intSub(JavaArithmetic::intAdd(base, first), second);
}

int_t Random::nextIntSum(int_t bound)
{
	const int_t first = nextInt(bound);
	const int_t second = nextInt(bound);
	return JavaArithmetic::intAdd(first, second);
}

float Random::nextFloatDifference()
{
	const float first = nextFloat();
	const float second = nextFloat();
	return first - second;
}

float Random::nextFloatProduct()
{
	const float first = nextFloat();
	const float second = nextFloat();
	return first * second;
}

float Random::nextFloatProduct3()
{
	const float first = nextFloat();
	const float second = nextFloat();
	const float third = nextFloat();
	return (first * second) * third;
}

float Random::nextFloatDifferenceTimesNextFloat()
{
	const float first = nextFloat();
	const float second = nextFloat();
	const float third = nextFloat();
	return (first - second) * third;
}

double Random::nextDoubleDifference()
{
	const double first = nextDouble();
	const double second = nextDouble();
	return first - second;
}

// Matches java.util.Random.nextGaussian (polar form of Box-Muller).
double Random::nextGaussian()
{
	if (haveNextNextGaussian)
	{
		haveNextNextGaussian = false;
		return nextNextGaussian;
	}

	double v1, v2, s;
	do
	{
		v1 = 2.0 * nextDouble() - 1.0;
		v2 = 2.0 * nextDouble() - 1.0;
		s = v1 * v1 + v2 * v2;
	} while (s >= 1.0 || s == 0.0);

	double multiplier = JavaMath::sqrt(-2.0 * JavaMath::log(s) / s);
	nextNextGaussian = v2 * multiplier;
	haveNextNextGaussian = true;
	return v1 * multiplier;
}
