// File_wii.cpp — Wii implementation of java/File.h, over libfat.
//
// This is the easiest of the three console file layers to write, and much
// shorter than src/ps2/java/File_ps2.cpp, because libfat gives newlib a real
// POSIX filesystem: open/stat/mkdir/rename/unlink/opendir all work on SD and USB
// exactly as they do on desktop. The PS2 had to fake all of that over the Memory
// Card, which is why its version carries workarounds for names like
// "level.dat_new" being created as directories.
//
// Two deliberate differences from the desktop version
// (src/pc/linux/java/File.cpp), both of which would break saving if copied:
//
//   1. NO realpath(). libfat paths carry a device prefix ("sd:/apps/..."), and
//      realpath() either fails on them or rewrites them into something the
//      libfat layer no longer recognises. The path is used verbatim.
//
//   2. No /proc/self/exe and no $HOME to discover locations from. The install
//      directory is fixed and comes from GameResources::getExeDir().
#ifdef WII_PLATFORM

#include "java/File.h"
#include "platform/Storage.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

#include <string>
#include <fstream>
#include <memory>

#include "net/minecraft/src/GameResources.h"
#include "util/Memory.h"
#include "wii/WiiEarlyInit.h"

namespace
{

class File_Impl : public File
{
private:
	std::string u8path;

public:
	explicit File_Impl(const jstring &p)
	{
		// Same reason as in Resource_wii.cpp: File objects can be built before
		// main(), so the mount cannot be taken for granted.
		wiiEnsureStorage();
		u8path = PlatformStorage::normalizeSlashes(static_cast<const std::string &>(p));
		this->path = u8path;
	}

	bool createNewFile() const override
	{
		return PlatformStorage::createFile(u8path);
	}

	bool remove() const override
	{
		return PlatformStorage::removePath(u8path);
	}

	bool renameTo(const File &dest) const override
	{
		return PlatformStorage::renamePath(u8path, dest.toString());
	}

	bool exists() const override
	{
		return PlatformStorage::exists(u8path);
	}

	bool isDirectory() const override
	{
		return PlatformStorage::isDirectory(u8path);
	}

	bool isFile() const override
	{
		return PlatformStorage::isFile(u8path);
	}

	long_t lastModified() const override
	{
		return static_cast<long_t>(PlatformStorage::lastModifiedMs(u8path));
	}

	long_t length() const override
	{
		const std::int64_t size = PlatformStorage::fileSize(u8path);
		return size >= 0 ? static_cast<long_t>(size) : 0;
	}

	std::vector<std::unique_ptr<File>> listFiles() const override
	{
		std::vector<std::unique_ptr<File>> files;
		if (!isDirectory())
			return files;

		std::vector<std::string> entries;
		if (!PlatformStorage::listEntries(u8path, entries))
			return files;

		for (const std::string &entry : entries)
			files.push_back(Util::make_unique<File_Impl>(jstring(PlatformStorage::join(u8path, entry))));
		return files;
	}

	File *getParentFile() const override
	{
		return new File_Impl(jstring(PlatformStorage::parent(u8path)));
	}

	bool mkdir() const override
	{
		if (PlatformStorage::exists(u8path))
			return false;
		return PlatformStorage::makeDirectory(u8path, 0755);
	}

	std::istream *toStreamIn() const override
	{
		auto is = Util::make_unique<std::ifstream>(u8path, std::ios::binary);
		if (!is->is_open() || !is->good())
			return nullptr;
		return is.release();
	}

	std::ostream *toStreamOut() const override
	{
		// A real writable stream, unlike the PS2's discard stream: the SD card
		// is an ordinary filesystem, so worlds actually persist here.
		auto os = Util::make_unique<std::ofstream>(u8path, std::ios::binary);
		if (!os->is_open() || !os->good())
			return nullptr;
		return os.release();
	}
};

} // namespace

File *File::open(const jstring &path)
{
	return new File_Impl(path);
}

File *File::open(const File &parent, const jstring &child)
{
	return new File_Impl(jstring(PlatformStorage::join(parent.toString(), child)));
}

File *File::openResourceDirectory()
{
	return new File_Impl(jstring(GameResources::getAssetsDir()));
}

File *File::openWorkingDirectory(const jstring &name)
{
	return new File_Impl(jstring(PlatformStorage::join(GameResources::getExeDir(), name)));
}

#endif // WII_PLATFORM
