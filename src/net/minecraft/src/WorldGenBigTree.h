#pragma once

#include <array>
#include <vector>
#include "WorldGenerator.h"
#include "platform/PlatformTuning.h"

#if PLATFORM_FLOAT_FEATURE_GENERATION
using BigTreeGenReal = float;
#else
using BigTreeGenReal = double;
#endif

// net.minecraft.src.WorldGenBigTree
class WorldGenBigTree : public WorldGenerator
{
public:
	WorldGenBigTree();
	explicit WorldGenBigTree(bool notify);

	void setScale(double d, double d1, double d2) override;
	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	void generateLeafNodeList();
	void placeLeafCircle(int_t i, int_t j, int_t k, float f, byte_t byte0, int_t l);
	float getLeafSize(int_t i);
	float getLeafNodeSize(int_t i);
	void generateLeafNode(int_t i, int_t j, int_t k);
	void placeBlockLine(std::array<int_t, 3> ai, std::array<int_t, 3> ai1, int_t i);
	void generateLeaves();
	bool leafNodeNeedsBase(int_t i);
	void generateTrunk();
	void generateLeafNodeBases();
	int_t checkBlockLine(std::array<int_t, 3> ai, std::array<int_t, 3> ai1);
	bool validTreeLocation();

	static const byte_t field_882_a[6];
	Random field_881_b;
	World *worldObj;
	std::array<int_t, 3> basePos;
	int_t field_878_e;
	int_t height;
	BigTreeGenReal field_876_g;
	BigTreeGenReal field_875_h;
	BigTreeGenReal field_874_i;
	BigTreeGenReal field_873_j;
	BigTreeGenReal field_872_k;
	int_t field_871_l;
	int_t field_870_m;
	int_t field_869_n;
	std::vector<std::array<int_t, 4>> field_868_o;
};
