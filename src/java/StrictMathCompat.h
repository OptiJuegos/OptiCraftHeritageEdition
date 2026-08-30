#pragma once

namespace StrictMathCompat
{
// fdlibm-backed Java StrictMath-compatible operations. These do not fall back
// to the host libm, including large-argument trigonometric reduction and the
// general pow(x, y) path.
double sin(double x);
double cos(double x);
double atan2(double y, double x);
double sqrt(double x);
double log(double x);
double pow(double x, double y);
}
