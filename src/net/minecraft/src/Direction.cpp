#include "Direction.h"

const int Direction::offsetX[4] = {0, -1, 0, 1};
const int Direction::offsetZ[4] = {1, 0, -1, 0};
const int Direction::headInvisibleFace[4] = {3, 4, 2, 5};
const int Direction::vineGrowth[6] = {-1, -1, 2, 0, 1, 3};
const int Direction::footInvisibleFaceRemap[4] = {2, 3, 0, 1};
const int Direction::enderEyeMetaToDirection[4] = {1, 2, 3, 0};
const int Direction::field_35868_g[4] = {3, 0, 1, 2};
const int Direction::bedDirection[4][6] = {
	{1, 0, 3, 2, 5, 4},
	{1, 0, 5, 4, 2, 3},
	{1, 0, 2, 3, 4, 5},
	{1, 0, 4, 5, 3, 2}
};
