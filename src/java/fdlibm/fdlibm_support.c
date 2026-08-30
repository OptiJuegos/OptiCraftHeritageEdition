/* Minimal support functions required by the bundled fdlibm 5.3 routines.
 * fabs/floor/scalbn are exact elementary operations for finite IEEE-754
 * doubles; keeping them here avoids routing transcendental functions through
 * the platform libm. */
#include "fdlibm.h"
#include <math.h>

double ieee_fabs(double x) { return fabs(x); }
double ieee_floor(double x) { return floor(x); }
double ieee_scalbn(double x, int n) { return scalbn(x, n); }
extern double __ieee754_sqrt(double);
double ieee_sqrt(double x) { return __ieee754_sqrt(x); }
