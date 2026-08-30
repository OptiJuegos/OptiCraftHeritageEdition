#include "NibbleArrayReader.h"

#include "java/Arithmetic.h"

NibbleArrayReader::NibbleArrayReader(const std::vector<byte_t> &values, int_t bits) :
	data(values), depthBits(bits), depthBitsPlusFour(JavaArithmetic::intAdd(bits, 4))
{
}

int_t NibbleArrayReader::get(int_t x, int_t y, int_t z) const
{
	const int_t index = JavaArithmetic::intShl(x, depthBitsPlusFour) |
	                    JavaArithmetic::intShl(z, depthBits) | y;
	const int_t byteIndex = JavaArithmetic::intShr(index, 1);
	if (byteIndex < 0 || byteIndex >= (int_t)data.size())
		return 0;
	const int_t half = index & 1;
	const int_t value = data[(std::size_t)byteIndex] & 0xff;
	return half == 0 ? value & 15 : value >> 4 & 15;
}
