#include "WorldGenMinable.h"

#include "Block.h"
#include "MathHelper.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"

WorldGenMinable::WorldGenMinable(int_t i, int_t j)
{
	minableBlockId = i;
	numberOfBlocks = j;
}

// Vein placement is the dominant cost of chunk decoration on console, and it is
// dominated in turn by the arithmetic type rather than by the block writes: the
// EE has no hardware double FPU, so every `double` below is software-emulated.
// One populate() runs ~92 veins (10 clay + 20 dirt + 10 gravel + 20 coal +
// 20 iron + 2 gold + 8 redstone + 1 diamond + 1 lapis), each sweeping a ~5^3
// box for 33 steps with several doubles per cell — a few million emulated ops,
// which is the 400-500 ms "chunkUnload" (= drainPendingPopulate -> populate)
// spike in the [PS2][FRAME] log.
//
// Every one of those values feeds either a comparison against 1.0 (the ellipsoid
// test) or a floor(), so float carries ample precision. This is the same trade
// PS2_FLOAT_BIOME_NOISE already makes for the biome field. The RNG call sequence
// is preserved exactly — same calls, same order, same consumed state — so vein
// *placement* still follows the seed; only the vein's rounded shape can differ
// by a block against the double path. Set PLATFORM_FLOAT_ORE_VEINS to 0 to fall
// back to the exact double arithmetic.
#if PLATFORM_FLOAT_ORE_VEINS
typedef float  gen_real_t;
#define GEN_FLOOR(v) MathHelper::floor_float(v)
#else
typedef double gen_real_t;
#define GEN_FLOOR(v) MathHelper::floor_double(v)
#endif

// Literals must carry the working type. Writing a bare `1.0` next to a float
// promotes the whole comparison back to double, which is exactly the emulated
// arithmetic this is trying to avoid.
#define GEN_R(lit) ((gen_real_t)(lit))

bool WorldGenMinable::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	float f = random.nextFloat() * 3.1415927f;
	gen_real_t d = (float)JavaArithmetic::intAdd(i, 8) + (MathHelper::sin(f) * (float)numberOfBlocks) / 8.0f;
	gen_real_t d1 = (float)JavaArithmetic::intAdd(i, 8) - (MathHelper::sin(f) * (float)numberOfBlocks) / 8.0f;
	gen_real_t d2 = (float)JavaArithmetic::intAdd(k, 8) + (MathHelper::cos(f) * (float)numberOfBlocks) / 8.0f;
	gen_real_t d3 = (float)JavaArithmetic::intAdd(k, 8) - (MathHelper::cos(f) * (float)numberOfBlocks) / 8.0f;
	gen_real_t d4 = JavaArithmetic::intSub(JavaArithmetic::intAdd(j, random.nextInt(3)), 2);
	gen_real_t d5 = JavaArithmetic::intSub(JavaArithmetic::intAdd(j, random.nextInt(3)), 2);
	// Block::stone is a fixed pointer; the original re-read blockID through it on
	// every voxel test.
	const int_t stoneId = Block::stone->blockID;
#if PLATFORM_FLOAT_ORE_VEINS
	const gen_real_t inverseBlockCount = GEN_R(1.0) / static_cast<gen_real_t>(numberOfBlocks);
#endif
	for (int_t l = 0; l <= numberOfBlocks; l++)
	{
#if PLATFORM_FLOAT_ORE_VEINS
		const gen_real_t blockFraction = static_cast<gen_real_t>(l) * inverseBlockCount;
		gen_real_t d6 = d + (d1 - d) * blockFraction;
		gen_real_t d7 = d4 + (d5 - d4) * blockFraction;
		gen_real_t d8 = d2 + (d3 - d2) * blockFraction;
#else
		gen_real_t d6 = d + ((d1 - d) * (gen_real_t)l) / (gen_real_t)numberOfBlocks;
		gen_real_t d7 = d4 + ((d5 - d4) * (gen_real_t)l) / (gen_real_t)numberOfBlocks;
		gen_real_t d8 = d2 + ((d3 - d2) * (gen_real_t)l) / (gen_real_t)numberOfBlocks;
#endif
#if PLATFORM_FLOAT_ORE_VEINS
		const gen_real_t d9 = (static_cast<gen_real_t>(random.nextDoubleFloat()) *
			static_cast<gen_real_t>(numberOfBlocks)) / GEN_R(16.0);
#else
		const gen_real_t d9 = (static_cast<gen_real_t>(random.nextDouble()) *
			static_cast<gen_real_t>(numberOfBlocks)) / GEN_R(16.0);
#endif
		// d10 and d11 were two spellings of one expression, so they were always
		// equal; evaluating it once drops a sin() and a multiply per step.
		gen_real_t d10 = (gen_real_t)(MathHelper::sin(((float)l * 3.1415927f) / (float)numberOfBlocks) + 1.0f) * d9 + GEN_R(1.0);
		gen_real_t d11 = d10;
		// The half-extents are loop-invariant across the three nested voxel
		// loops, which recomputed `d10 / 2.0` for every single cell. Hoisting is
		// bit-identical (same expression, same operands), it just stops the
		// divide from running ~4000 times per vein instead of once per step.
		const gen_real_t r10 = d10 / GEN_R(2.0);
		const gen_real_t r11 = d11 / GEN_R(2.0);
#if PLATFORM_FLOAT_ORE_VEINS
		const gen_real_t inverseR10 = GEN_R(1.0) / r10;
		const gen_real_t inverseR11 = GEN_R(1.0) / r11;
#endif
		int_t i1 = GEN_FLOOR(d6 - r10);
		int_t j1 = GEN_FLOOR(d7 - r11);
		int_t k1 = GEN_FLOOR(d8 - r10);
		int_t l1 = GEN_FLOOR(d6 + r10);
		int_t i2 = GEN_FLOOR(d7 + r11);
		int_t j2 = GEN_FLOOR(d8 + r10);
		for (int_t k2 = i1; k2 <= l1; k2++)
		{
#if PLATFORM_FLOAT_ORE_VEINS
			gen_real_t d12 = (((gen_real_t)k2 + GEN_R(0.5)) - d6) * inverseR10;
#else
			gen_real_t d12 = (((gen_real_t)k2 + GEN_R(0.5)) - d6) / r10;
#endif
			// The original recomputed d12*d12 up to three times per voxel and
			// d13*d13 twice; the partial sums are the same value each time.
			gen_real_t sx = d12 * d12;
			if (sx >= GEN_R(1.0))
				continue;
			for (int_t l2 = j1; l2 <= i2; l2++)
			{
#if PLATFORM_FLOAT_ORE_VEINS
				gen_real_t d13 = (((gen_real_t)l2 + GEN_R(0.5)) - d7) * inverseR11;
#else
				gen_real_t d13 = (((gen_real_t)l2 + GEN_R(0.5)) - d7) / r11;
#endif
				gen_real_t sxy = sx + d13 * d13;
				if (sxy >= GEN_R(1.0))
					continue;
				for (int_t i3 = k1; i3 <= j2; i3++)
				{
#if PLATFORM_FLOAT_ORE_VEINS
					gen_real_t d14 = (((gen_real_t)i3 + GEN_R(0.5)) - d8) * inverseR10;
#else
					gen_real_t d14 = (((gen_real_t)i3 + GEN_R(0.5)) - d8) / r10;
#endif
					if (sxy + d14 * d14 < GEN_R(1.0))
						world->replaceBlockForPopulation(k2, l2, i3,
						                                 stoneId, minableBlockId);
				}
			}
		}
	}
	return true;
}
