#pragma once

#include "java/Type.h"

// net.minecraft.src.PistonBlockTextures
//
// Lookup tables for the six piston facings. Java keeps the raw obfuscated
// names; we provide readable aliases that the rest of the port uses.
class PistonBlockTextures
{
public:
	// Java field_31057_a — opposite-face index for each facing (0..5).
	static const int_t oppositeFacing[6];
	// Java field_31056_b — unit X delta per facing.
	static const int_t deltaX[6];
	// Java field_31059_c — unit Y delta per facing.
	static const int_t deltaY[6];
	// Java field_31058_d — unit Z delta per facing.
	static const int_t deltaZ[6];
};
