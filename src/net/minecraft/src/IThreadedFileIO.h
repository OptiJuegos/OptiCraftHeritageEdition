#pragma once

// net.minecraft.src.IThreadedFileIO
class IThreadedFileIO
{
public:
	virtual ~IThreadedFileIO() = default;
	virtual bool writeNextIO() = 0;
};
