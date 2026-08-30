#pragma once

#include "java/Type.h"
#include "java/StrictMathCompat.h"

namespace Math
{

double random();

}

// Central boundary for java.lang.Math operations whose exact libm behaviour
// matters to Java/C++ parity tests. Keeping the calls here gives each target one
// replacement point if its libc differs from the JVM on an audited vector.
namespace JavaMath
{
inline double sin(double v) { return StrictMathCompat::sin(v); }
inline double cos(double v) { return StrictMathCompat::cos(v); }
inline double sqrt(double v) { return StrictMathCompat::sqrt(v); }
inline double pow(double a, double b) { return StrictMathCompat::pow(a, b); }
inline double atan2(double y, double x) { return StrictMathCompat::atan2(y, x); }
inline double log(double v) { return StrictMathCompat::log(v); }
}
