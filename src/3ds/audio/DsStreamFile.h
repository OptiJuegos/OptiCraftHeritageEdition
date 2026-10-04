#pragma once

// Sequential read-only file for the 3DS audio stream thread -- the ndsp
// counterpart of the PS2's Ps2StreamFile, reading the same ADP assets.
//
// stdio is fine here, where the PS2 reader goes to the raw POSIX layer
// instead: PS2 stdio holds a process-wide newlib lock across the whole
// read (an IOP RPC), but newlib's FILE lock is per-FILE, and this handle
// belongs to the stream thread alone -- the main thread's loaders never
// contend on it.
//
// A "pak://" path opens the pak itself on a handle of its own and confines
// every offset to the entry's byte range, so the stream thread never shares
// a file position with the main thread's loader.

#include <cstdio>

class DsStreamFile
{
public:
	static constexpr int kBufferBytes = 8192;

	DsStreamFile() = default;
	~DsStreamFile();
	DsStreamFile(const DsStreamFile &) = delete;
	DsStreamFile &operator=(const DsStreamFile &) = delete;

	bool open(const char *path);
	void close();
	bool isOpen() const;

	// Bytes in the file or pak entry.
	long size() const;

	// Absolute seek within the file/entry; discards buffered data.
	bool seek(long offset);

	// Reads exactly `bytes` into `dst`, refilling as needed.
	bool readExact(void *dst, int bytes);

private:
	int fill();
	const unsigned char *data() const;
	void consume(int bytes);

	std::FILE *file_ = nullptr;
	long base_ = 0;    // byte offset of the entry inside the handle (0 for a loose file)
	long size_ = 0;    // bytes readable from base_
	long filePos_ = 0; // next byte fill() reads, relative to base_
	int pos_ = 0;      // buffer cursor
	int len_ = 0;      // buffered byte count
	bool failed_ = false;
	unsigned char buffer_[kBufferBytes];
};
