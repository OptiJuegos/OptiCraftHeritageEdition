#include "util/GLU.h"

#include <cmath>

void gluPerspective(float fovy, float aspect, float zNear, float zFar)
{
    constexpr double pi = 3.14159265358979323846;
    double const height = zNear * std::tan(fovy * pi / 360.0);
    double const width = height * aspect;
    renderFrustum(-width, width, -height, height, zNear, zFar);
}
