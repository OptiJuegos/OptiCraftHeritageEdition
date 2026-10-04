#ifdef CTR_PLATFORM

#include "3ds/audio/DsStreamFile.h"

#include "platform/storage/AssetPak.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>

DsStreamFile::~DsStreamFile()
{
	close();
}

bool DsStreamFile::open(const char *path)
{
	close();
	if (path == nullptr)
		return false;

	const std::string spelled(path);
	if (AssetPak::isPakPath(spelled))
	{
		std::uint32_t dataOffset = 0;
		std::uint32_t entrySize = 0;
		if (!AssetPak::locate(AssetPak::keyOf(spelled), &dataOffset, &entrySize))
			return false;
		file_ = std::fopen(AssetPak::archivePath().c_str(), "rb");
		if (file_ == nullptr)
			return false;
		base_ = static_cast<long>(dataOffset);
		size_ = static_cast<long>(entrySize);
	}
	else
	{
		file_ = std::fopen(path, "rb");
		if (file_ == nullptr)
			return false;
		base_ = 0;
		if (std::fseek(file_, 0, SEEK_END) != 0)
		{
			close();
			return false;
		}
		const long end = std::ftell(file_);
		if (end < 0)
		{
			close();
			return false;
		}
		size_ = end;
	}
	return seek(0);
}

void DsStreamFile::close()
{
	if (file_ != nullptr)
		std::fclose(file_);
	file_ = nullptr;
	pos_ = 0;
	len_ = 0;
	failed_ = false;
	base_ = 0;
	size_ = 0;
	filePos_ = 0;
}

bool DsStreamFile::isOpen() const
{
	return file_ != nullptr;
}

long DsStreamFile::size() const
{
	return file_ != nullptr ? size_ : -1L;
}

bool DsStreamFile::seek(long offset)
{
	if (file_ == nullptr || offset < 0 || offset > size_)
		return false;
	pos_ = 0;
	len_ = 0;
	if (std::fseek(file_, base_ + offset, SEEK_SET) != 0)
		return false;
	filePos_ = offset;
	return true;
}

int DsStreamFile::fill()
{
	if (file_ == nullptr || failed_)
		return -1;
	if (pos_ < len_)
		return len_ - pos_;

	pos_ = 0;
	len_ = 0;
	const long remaining = size_ - filePos_;
	if (remaining <= 0)
		return 0;
	const int want = static_cast<int>(std::min<long>(remaining, kBufferBytes));
	const std::size_t got = std::fread(buffer_, 1, static_cast<std::size_t>(want), file_);
	if (got == 0)
	{
		failed_ = true;
		return -1;
	}
	len_ = static_cast<int>(got);
	filePos_ += static_cast<long>(got);
	return len_;
}

const unsigned char *DsStreamFile::data() const
{
	return buffer_ + pos_;
}

void DsStreamFile::consume(int bytes)
{
	pos_ = std::min(len_, pos_ + std::max(0, bytes));
}

bool DsStreamFile::readExact(void *dst, int bytes)
{
	unsigned char *out = static_cast<unsigned char *>(dst);
	int remaining = bytes;
	while (remaining > 0)
	{
		const int available = fill();
		if (available <= 0)
			return false;
		const int piece = std::min(remaining, available);
		std::memcpy(out, data(), static_cast<std::size_t>(piece));
		consume(piece);
		out += piece;
		remaining -= piece;
	}
	return true;
}

#endif // CTR_PLATFORM
