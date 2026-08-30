#include "WorldGenBigTree.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include "MathHelper.h"
#include "World.h"

const byte_t WorldGenBigTree::field_882_a[6] = { 2, 0, 0, 1, 2, 1 };

#if PLATFORM_FLOAT_FEATURE_GENERATION
#define TREE_R(v) static_cast<BigTreeGenReal>(v##f)
static int_t treeToInt(BigTreeGenReal value) { return JavaArithmetic::floatToInt(value); }
static int_t treeFloor(BigTreeGenReal value) { return MathHelper::floor_float(value); }
static BigTreeGenReal treeSqrt(BigTreeGenReal value) { return MathHelper::sqrt_float(value); }
static BigTreeGenReal treeSquare(BigTreeGenReal value) { return value * value; }
static BigTreeGenReal treeSin(BigTreeGenReal value) { return MathHelper::sin(value); }
static BigTreeGenReal treeCos(BigTreeGenReal value) { return MathHelper::cos(value); }
#else
#define TREE_R(v) static_cast<BigTreeGenReal>(v)
static int_t treeToInt(BigTreeGenReal value) { return JavaArithmetic::doubleToInt(value); }
static int_t treeFloor(BigTreeGenReal value) { return MathHelper::floor_double(value); }
static BigTreeGenReal treeSqrt(BigTreeGenReal value) { return JavaMath::sqrt(value); }
static BigTreeGenReal treeSquare(BigTreeGenReal value) { return JavaMath::pow(value, 2.0); }
static BigTreeGenReal treeSin(BigTreeGenReal value) { return JavaMath::sin(value); }
static BigTreeGenReal treeCos(BigTreeGenReal value) { return JavaMath::cos(value); }
#endif


WorldGenBigTree::WorldGenBigTree()
    : WorldGenBigTree(false)
{
}

WorldGenBigTree::WorldGenBigTree(bool notify)
    : WorldGenerator(notify)
{
	worldObj = nullptr;
	basePos = { 0, 0, 0 };
	field_878_e = 0;
	height = 0;
	field_876_g = TREE_R(0.61799999999999999);
	field_875_h = TREE_R(1.0);
	field_874_i = TREE_R(0.38100000000000001);
	field_873_j = TREE_R(1.0);
	field_872_k = TREE_R(1.0);
	field_871_l = 1;
	field_870_m = 12;
	field_869_n = 4;
}

void WorldGenBigTree::generateLeafNodeList()
{
	height = treeToInt(static_cast<BigTreeGenReal>(field_878_e) * field_876_g);
	if (height >= field_878_e)
		height = JavaArithmetic::intSub(field_878_e, 1);
	BigTreeGenReal nodeScale = (field_872_k * static_cast<BigTreeGenReal>(field_878_e)) / TREE_R(13.0);
	int_t i = treeToInt(TREE_R(1.3819999999999999) + treeSquare(nodeScale));
	if (i < 1)
		i = 1;
	const int_t nodeCapacity = JavaArithmetic::intMul(i, field_878_e);
	if (nodeCapacity <= 0)
		throw std::length_error("Invalid big-tree node capacity");
	std::vector<std::array<int_t, 4>> ai(static_cast<std::size_t>(nodeCapacity));
	int_t j = JavaArithmetic::intSub(JavaArithmetic::intAdd(basePos[1], field_878_e), field_869_n);
	int_t k = 1;
	int_t l = JavaArithmetic::intAdd(basePos[1], height);
	int_t i1 = JavaArithmetic::intSub(j, basePos[1]);
	ai[0] = { basePos[0], j, basePos[2], l };
	j = JavaArithmetic::intSub(j, 1);
	while (i1 >= 0)
	{
		int_t j1 = 0;
		float f = getLeafSize(i1);
		if (f < 0.0f)
		{
			j = JavaArithmetic::intSub(j, 1);
			i1 = JavaArithmetic::intSub(i1, 1);
		}
		else
		{
			BigTreeGenReal d = TREE_R(0.5);
			for (; j1 < i; j1++)
			{
				BigTreeGenReal d1 = field_873_j * (static_cast<BigTreeGenReal>(f) * (static_cast<BigTreeGenReal>(field_881_b.nextFloat()) + TREE_R(0.32800000000000001)));
				BigTreeGenReal d2 = static_cast<BigTreeGenReal>(field_881_b.nextFloat()) * TREE_R(2.0) * TREE_R(3.1415899999999999);
				int_t k1 = treeFloor(d1 * treeSin(d2) + static_cast<BigTreeGenReal>(basePos[0]) + d);
				int_t l1 = treeFloor(d1 * treeCos(d2) + static_cast<BigTreeGenReal>(basePos[2]) + d);
				std::array<int_t, 3> ai1 = { k1, j, l1 };
				std::array<int_t, 3> ai2 = { k1, JavaArithmetic::intAdd(j, field_869_n), l1 };
				if (checkBlockLine(ai1, ai2) != -1)
					continue;
				std::array<int_t, 3> ai3 = { basePos[0], basePos[1], basePos[2] };
				BigTreeGenReal dx = static_cast<BigTreeGenReal>(JavaArithmetic::intAbs(JavaArithmetic::intSub(basePos[0], ai1[0])));
				BigTreeGenReal dz = static_cast<BigTreeGenReal>(JavaArithmetic::intAbs(JavaArithmetic::intSub(basePos[2], ai1[2])));
				BigTreeGenReal d3 = treeSqrt(treeSquare(dx) + treeSquare(dz));
				BigTreeGenReal d4 = d3 * field_874_i;
				if (static_cast<BigTreeGenReal>(ai1[1]) - d4 > static_cast<BigTreeGenReal>(l))
					ai3[1] = l;
				else
					ai3[1] = treeToInt(static_cast<BigTreeGenReal>(ai1[1]) - d4);
				if (checkBlockLine(ai3, ai1) == -1)
					ai[k++] = { k1, j, l1, ai3[1] };
			}
			j = JavaArithmetic::intSub(j, 1);
			i1 = JavaArithmetic::intSub(i1, 1);
		}
	}
	field_868_o.assign(ai.begin(), ai.begin() + k);
}

void WorldGenBigTree::placeLeafCircle(int_t i, int_t j, int_t k, float f, byte_t byte0, int_t l)
{
	int_t i1 = treeToInt(static_cast<BigTreeGenReal>(f) + TREE_R(0.61799999999999999));
	byte_t byte1 = field_882_a[byte0];
	byte_t byte2 = field_882_a[byte0 + 3];
	std::array<int_t, 3> ai = { i, j, k };
	std::array<int_t, 3> ai1 = { 0, 0, 0 };
	int_t j1 = -i1;
	int_t k1 = -i1;
	ai1[byte0] = ai[byte0];
	for (; j1 <= i1; j1++)
	{
		ai1[byte1] = JavaArithmetic::intAdd(ai[byte1], j1);
		for (int_t l1 = -i1; l1 <= i1;)
		{
			BigTreeGenReal dx = static_cast<BigTreeGenReal>(JavaArithmetic::intAbs(j1)) + TREE_R(0.5);
			BigTreeGenReal dz = static_cast<BigTreeGenReal>(JavaArithmetic::intAbs(l1)) + TREE_R(0.5);
			BigTreeGenReal d = treeSqrt(treeSquare(dx) + treeSquare(dz));
			if (d > static_cast<BigTreeGenReal>(f))
			{
				l1++;
			}
			else
			{
				ai1[byte2] = JavaArithmetic::intAdd(ai[byte2], l1);
				int_t i2 = worldObj->getBlockId(ai1[0], ai1[1], ai1[2]);
				if (i2 != 0 && i2 != 18)
				{
					l1++;
				}
				else
				{
					setBlockAndMetadata(worldObj, ai1[0], ai1[1], ai1[2], l, 0);
					l1++;
				}
			}
		}
	}
}

float WorldGenBigTree::getLeafSize(int_t i)
{
	if (static_cast<BigTreeGenReal>(i) < static_cast<BigTreeGenReal>(field_878_e) * TREE_R(0.29999999999999999))
		return -1.618f;
	float f = (float)field_878_e / 2.0f;
	float f1 = (float)field_878_e / 2.0f - (float)i;
	float f2;
	if (f1 == 0.0f)
		f2 = f;
	else if (std::abs(f1) >= f)
		f2 = 0.0f;
	else
		f2 = static_cast<float>(treeSqrt(treeSquare(static_cast<BigTreeGenReal>(std::abs(f))) -
		treeSquare(static_cast<BigTreeGenReal>(std::abs(f1)))));
	f2 *= 0.5f;
	return f2;
}

float WorldGenBigTree::getLeafNodeSize(int_t i)
{
	if (i < 0 || i >= field_869_n)
		return -1.0f;
	return i != 0 && i != field_869_n - 1 ? 3.0f : 2.0f;
}

void WorldGenBigTree::generateLeafNode(int_t i, int_t j, int_t k)
{
	int_t l = j;
	for (int_t i1 = JavaArithmetic::intAdd(j, field_869_n); l < i1; l = JavaArithmetic::intAdd(l, 1))
	{
		float f = getLeafNodeSize(l - j);
		placeLeafCircle(i, l, k, f, (byte_t)1, 18);
	}
}

void WorldGenBigTree::placeBlockLine(std::array<int_t, 3> ai, std::array<int_t, 3> ai1, int_t i)
{
	std::array<int_t, 3> ai2 = { 0, 0, 0 };
	byte_t byte0 = 0;
	int_t j = 0;
	for (; byte0 < 3; byte0++)
	{
		ai2[byte0] = JavaArithmetic::intSub(ai1[byte0], ai[byte0]);
		if (JavaArithmetic::intAbs(ai2[byte0]) > JavaArithmetic::intAbs(ai2[j]))
			j = byte0;
	}
	if (ai2[j] == 0)
		return;
	byte_t byte1 = field_882_a[j];
	byte_t byte2 = field_882_a[j + 3];
	byte_t byte3 = ai2[j] > 0 ? 1 : -1;
	BigTreeGenReal d = static_cast<BigTreeGenReal>(ai2[byte1]) / static_cast<BigTreeGenReal>(ai2[j]);
	BigTreeGenReal d1 = static_cast<BigTreeGenReal>(ai2[byte2]) / static_cast<BigTreeGenReal>(ai2[j]);
	std::array<int_t, 3> ai3 = { 0, 0, 0 };
	int_t k = 0;
	for (int_t l = JavaArithmetic::intAdd(ai2[j], byte3); k != l; k += byte3)
	{
		ai3[j] = treeFloor(static_cast<BigTreeGenReal>(JavaArithmetic::intAdd(ai[j], k)) + TREE_R(0.5));
		ai3[byte1] = treeFloor(static_cast<BigTreeGenReal>(ai[byte1]) + static_cast<BigTreeGenReal>(k) * d + TREE_R(0.5));
		ai3[byte2] = treeFloor(static_cast<BigTreeGenReal>(ai[byte2]) + static_cast<BigTreeGenReal>(k) * d1 + TREE_R(0.5));
		setBlockAndMetadata(worldObj, ai3[0], ai3[1], ai3[2], i, 0);
	}
}

void WorldGenBigTree::generateLeaves()
{
	for (int_t i = 0, j = (int_t)field_868_o.size(); i < j; i++)
		generateLeafNode(field_868_o[i][0], field_868_o[i][1], field_868_o[i][2]);
}

bool WorldGenBigTree::leafNodeNeedsBase(int_t i)
{
	return static_cast<BigTreeGenReal>(i) >= static_cast<BigTreeGenReal>(field_878_e) * TREE_R(0.20000000000000001);
}

void WorldGenBigTree::generateTrunk()
{
	int_t i = basePos[0];
	int_t j = basePos[1];
	int_t k = JavaArithmetic::intAdd(basePos[1], height);
	int_t l = basePos[2];
	std::array<int_t, 3> ai = { i, j, l };
	std::array<int_t, 3> ai1 = { i, k, l };
	placeBlockLine(ai, ai1, 17);
	if (field_871_l == 2)
	{
		ai[0] = JavaArithmetic::intAdd(ai[0], 1);
		ai1[0] = JavaArithmetic::intAdd(ai1[0], 1);
		placeBlockLine(ai, ai1, 17);
		ai[2] = JavaArithmetic::intAdd(ai[2], 1);
		ai1[2] = JavaArithmetic::intAdd(ai1[2], 1);
		placeBlockLine(ai, ai1, 17);
		ai[0] = JavaArithmetic::intSub(ai[0], 1);
		ai1[0] = JavaArithmetic::intSub(ai1[0], 1);
		placeBlockLine(ai, ai1, 17);
	}
}

void WorldGenBigTree::generateLeafNodeBases()
{
	std::array<int_t, 3> ai = { basePos[0], basePos[1], basePos[2] };
	for (int_t i = 0, j = (int_t)field_868_o.size(); i < j; i++)
	{
		std::array<int_t, 4> ai1 = field_868_o[i];
		std::array<int_t, 3> ai2 = { ai1[0], ai1[1], ai1[2] };
		ai[1] = ai1[3];
		int_t k = JavaArithmetic::intSub(ai[1], basePos[1]);
		if (leafNodeNeedsBase(k))
			placeBlockLine(ai, ai2, 17);
	}
}

int_t WorldGenBigTree::checkBlockLine(std::array<int_t, 3> ai, std::array<int_t, 3> ai1)
{
	std::array<int_t, 3> ai2 = { 0, 0, 0 };
	byte_t byte0 = 0;
	int_t i = 0;
	for (; byte0 < 3; byte0++)
	{
		ai2[byte0] = JavaArithmetic::intSub(ai1[byte0], ai[byte0]);
		if (JavaArithmetic::intAbs(ai2[byte0]) > JavaArithmetic::intAbs(ai2[i]))
			i = byte0;
	}
	if (ai2[i] == 0)
		return -1;
	byte_t byte1 = field_882_a[i];
	byte_t byte2 = field_882_a[i + 3];
	byte_t byte3 = ai2[i] > 0 ? 1 : -1;
	BigTreeGenReal d = static_cast<BigTreeGenReal>(ai2[byte1]) / static_cast<BigTreeGenReal>(ai2[i]);
	BigTreeGenReal d1 = static_cast<BigTreeGenReal>(ai2[byte2]) / static_cast<BigTreeGenReal>(ai2[i]);
	std::array<int_t, 3> ai3 = { 0, 0, 0 };
	int_t j = 0;
	int_t k = JavaArithmetic::intAdd(ai2[i], byte3);
	do
	{
		if (j == k)
			break;
		ai3[i] = JavaArithmetic::intAdd(ai[i], j);
		ai3[byte1] = treeFloor(static_cast<BigTreeGenReal>(ai[byte1]) + static_cast<BigTreeGenReal>(j) * d);
		ai3[byte2] = treeFloor(static_cast<BigTreeGenReal>(ai[byte2]) + static_cast<BigTreeGenReal>(j) * d1);
		int_t l = worldObj->getBlockId(ai3[0], ai3[1], ai3[2]);
		if (l != 0 && l != 18)
			break;
		j = JavaArithmetic::intAdd(j, byte3);
	} while (true);
	if (j == k)
		return -1;
	return JavaArithmetic::intAbs(j);
}

bool WorldGenBigTree::validTreeLocation()
{
	std::array<int_t, 3> ai = { basePos[0], basePos[1], basePos[2] };
	std::array<int_t, 3> ai1 = { basePos[0], JavaArithmetic::intSub(JavaArithmetic::intAdd(basePos[1], field_878_e), 1), basePos[2] };
	int_t i = worldObj->getBlockId(basePos[0], JavaArithmetic::intSub(basePos[1], 1), basePos[2]);
	if (i != 2 && i != 3)
		return false;
	int_t j = checkBlockLine(ai, ai1);
	if (j == -1)
		return true;
	if (j < 6)
		return false;
	field_878_e = j;
	return true;
}

void WorldGenBigTree::setScale(double d, double d1, double d2)
{
	field_870_m = JavaArithmetic::doubleToInt(d * 12.0);
	if (d > 0.5)
		field_869_n = 5;
	field_873_j = static_cast<BigTreeGenReal>(d1);
	field_872_k = static_cast<BigTreeGenReal>(d2);
}

bool WorldGenBigTree::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	worldObj = world;
	long_t l = random.nextLong();
	field_881_b.setSeed(l);
	basePos[0] = i;
	basePos[1] = j;
	basePos[2] = k;
	if (field_878_e == 0)
		field_878_e = JavaArithmetic::intAdd(5, field_881_b.nextInt(field_870_m));
	if (!validTreeLocation())
		return false;
	generateLeafNodeList();
	generateLeaves();
	generateTrunk();
	generateLeafNodeBases();
	return true;
}
