#pragma once

#include "TileEntity.h"
#include "java/Type.h"

class NBTTagCompound;

// net.minecraft.src.TileEntityPiston
class TileEntityPiston : public TileEntity
{
public:
	TileEntityPiston();
	// Java: TileEntityPiston(int i, int j, int k, boolean flag, boolean flag1)
	// i = storedBlockID, j = storedMetadata, k = pistonOrientation (facing),
	// flag = extending, flag1 = isHead.
	TileEntityPiston(int_t storedBlockID, int_t storedMetadata, int_t orientation, bool extending, bool isHead);

	int_t getStoredBlockID();
	int_t getBlockMetadata() override;

	// Java: func_31015_b — true while the piston is in the extending phase.
	bool isExtending();
	// Java: func_31009_d — facing of the moving piston (0..5).
	int_t getOrientation();
	// Java: func_31012_k — head/source flag (true if this TE represents the head).
	bool isHead();

	// Java: func_31008_a(f) — interpolated 0..1 progress at partialTicks `f`.
	float getProgress(float partialTicks);
	// Java: func_31017_b(f) — X offset for rendering the moved block at progress `f`.
	float getOffsetX(float partialTicks);
	// Java: func_31014_c(f) — Y offset.
	float getOffsetY(float partialTicks);
	// Java: func_31013_d(f) — Z offset.
	float getOffsetZ(float partialTicks);

	// Java: func_31011_l — finalize the piston move (clears the TE, places the block).
	void clearPistonTileEntity();

	void updateEntity() override;
	void readFromNBT(NBTTagCompound *nbttagcompound) override;
	void writeToNBT(NBTTagCompound *nbttagcompound) override;

private:
	// Java: func_31010_a(f, f1) — pushes entities sitting in the path of the moving block.
	void pushEntities(float progress, float delta);

	int_t storedBlockID;
	int_t storedMetadata;
	int_t pistonOrientation;
	bool  extending;
	bool  fieldIsHead;
	float progress;
	float lastProgress;
};
