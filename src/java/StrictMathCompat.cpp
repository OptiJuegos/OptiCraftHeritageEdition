#include "java/StrictMathCompat.h"

/*
 * The implementations below delegate to the bundled fdlibm 5.3 sources in
 * src/java/fdlibm.  Those files carry Sun's original permissive notices.
 * Java StrictMath is specified in terms of fdlibm-style reproducible results,
 * which avoids target-libm drift between desktop, Wii and PS2.
 */
extern "C" {
double ieee_sin(double);
double ieee_cos(double);
double __ieee754_atan2(double, double);
double __ieee754_sqrt(double);
double __ieee754_log(double);
double __ieee754_pow(double, double);
}

namespace StrictMathCompat
{
double sin(double x) { return ieee_sin(x); }
double cos(double x) { return ieee_cos(x); }
double atan2(double y, double x) { return __ieee754_atan2(y, x); }
double sqrt(double x) { return __ieee754_sqrt(x); }
double log(double x) { return __ieee754_log(x); }
double pow(double x, double y) { return __ieee754_pow(x, y); }
}
