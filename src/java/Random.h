#pragma once

#include "java/Type.h"

class Random
{
private:
	long_t seed;

public:
	Random();
	Random(long_t set_seed);

	void setSeed(long_t set_seed);

	int_t next(int_t bits);

	bool nextBoolean();

	int_t nextInt();
	int_t nextInt(int_t bound);

	long_t nextLong();

	float nextFloat();
	double nextDouble();
	float nextDoubleFloat();
	int_t nextInt5();

	// C++ does not preserve Java's left-to-right evaluation order for every
	// expression. These helpers sequence common multi-draw Java expressions.
	int_t nextIntDifference(int_t bound);
	int_t nextIntOffset(int_t base, int_t bound);
	int_t nextIntSum(int_t bound);
	float nextFloatDifference();
	float nextFloatProduct();
	float nextFloatProduct3();
	float nextFloatDifferenceTimesNextFloat();
	double nextDoubleDifference();

	// java.util.Random.nextGaussian() — polar Box-Muller, state-bearing.
	double nextGaussian();

private:
	bool   haveNextNextGaussian = false;
	double nextNextGaussian = 0.0;
};
