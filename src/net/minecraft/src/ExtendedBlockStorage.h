#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"
#include "NibbleArray.h"

// net.minecraft.src.ExtendedBlockStorage
class ExtendedBlockStorage
{
public:
	static constexpr int_t BLOCK_COUNT = 16 * 16 * 16;
	static constexpr int_t NIBBLE_DEPTH_BITS = 4;

	explicit ExtendedBlockStorage(int_t yBase);

	int_t getExtBlockID(int_t x, int_t y, int_t z) const;
	void setExtBlockID(int_t x, int_t y, int_t z, int_t blockId);

	int_t getExtBlockMetadata(int_t x, int_t y, int_t z) const;
	void setExtBlockMetadata(int_t x, int_t y, int_t z, int_t metadata);

	bool getIsEmpty() const;
	bool getNeedsRandomTick() const;
	int_t getYLocation() const;

	void setExtSkylightValue(int_t x, int_t y, int_t z, int_t value);
	int_t getExtSkylightValue(int_t x, int_t y, int_t z) const;

	void setExtBlocklightValue(int_t x, int_t y, int_t z, int_t value);
	int_t getExtBlocklightValue(int_t x, int_t y, int_t z) const;

	void func_48708_d();
	void recalculateBlockCounts();
	void func_48711_e();
	int_t func_48700_f() const;

	std::vector<byte_t> &func_48692_g();
	const std::vector<byte_t> &func_48692_g() const;
	void func_48715_h();

	NibbleArray *getBlockMSBArray();
	const NibbleArray *getBlockMSBArray() const;
	NibbleArray &func_48697_j();
	const NibbleArray &func_48697_j() const;
	NibbleArray &getBlocklightArray();
	const NibbleArray &getBlocklightArray() const;
	NibbleArray &getSkylightArray();
	const NibbleArray &getSkylightArray() const;

	void setBlockLSBArray(std::vector<byte_t> bytes);
	void setBlockMSBArray(std::unique_ptr<NibbleArray> array);
	void setBlockMetadataArray(NibbleArray array);
	void setBlocklightArray(NibbleArray array);
	void setSkylightArray(NibbleArray array);
	NibbleArray *createBlockMSBArray();

private:
	static int_t getStorageIndex(int_t x, int_t y, int_t z);
	static bool isRandomTickBlock(int_t blockId);
	static bool isKnownBlockId(int_t blockId);
	static void validateBlockArraySize(const std::vector<byte_t> &bytes);
	static void validateNibbleArraySize(const NibbleArray &array);

	int_t yBase;
	int_t blockRefCount;
	int_t tickRefCount;
	std::vector<byte_t> blockLSBArray;
	std::unique_ptr<NibbleArray> blockMSBArray;
	NibbleArray blockMetadataArray;
	NibbleArray blocklightArray;
	NibbleArray skylightArray;
};
