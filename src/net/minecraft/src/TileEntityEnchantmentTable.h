#pragma once

#include "TileEntity.h"
#include "java/Random.h"

// net.minecraft.src.TileEntityEnchantmentTable
class TileEntityEnchantmentTable : public TileEntity
{
public:
	TileEntityEnchantmentTable();
	void updateEntity() override;

	int_t tickCount;
	float pageFlip;
	float pageFlipPrev;
	float field_40061_d;
	float field_40062_e;
	float bookSpread;
	float bookSpreadPrev;
	float bookRotation2;
	float bookRotationPrev;
	float bookRotation;

private:
	static Random rand;
};
