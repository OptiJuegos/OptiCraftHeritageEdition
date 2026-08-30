#include "platform/Log.h"
#include "ChunkBlockMap.h"

#include <cstdio>
#include "Block.h"

byte_t ChunkBlockMap::blockMapping[256];

void ChunkBlockMap::initialize()
{
	try
	{
		for (int_t i = 0; i < 256; i++)
		{
			byte_t byte0 = (byte_t)i;
			if (byte0 != 0 && Block::blocksList[byte0 & 0xff] == nullptr)
				byte0 = 0;
			blockMapping[i] = byte0;
		}
	}
	catch (...)
	{
		MC_LOG_ERROR("chunk", "ChunkBlockMap::initialize – exception caught\n");
	}
}

void ChunkBlockMap::remapBlockArray(byte_t *abyte0, int_t length)
{
	for (int_t i = 0; i < length; i++)
		abyte0[i] = blockMapping[abyte0[i] & 0xff];
}
