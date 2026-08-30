#pragma once

#include "java/Type.h"

// net.minecraft.src.ChunkBlockMap
class ChunkBlockMap
{
public:
	static void initialize();

	// Replaces each block ID in the array with a validated mapping.
	// func_26002_a
	static void remapBlockArray(byte_t *abyte0, int_t length);

private:
	static byte_t blockMapping[256]; // field_26003_a
};
