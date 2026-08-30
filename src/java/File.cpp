#include "java/File.h"

#include <stack>
#include <string>
#include <iostream>
#ifdef PS2_PLATFORM
#include <algorithm>
#include <cstring>
#endif

jstring File::getName() const
{
	size_t pos = path.find_last_of("/\\");
	if (pos == jstring::npos)
		return path;
	return path.substr(pos + 1);
}

jstring File::toString() const
{
	return path;
}

jstring File::toURL() const
{
	return "file://" + path;
}

bool File::mkdirs() const
{
	// Get directory up to last one that exists
	std::stack<std::unique_ptr<File>> back;

	jstring current = path;

#ifdef PS2_PLATFORM
	// PS2 safety: some callers use mkdirs() as an "ensure parent path" helper.
	// On Memory Card this accidentally created file names such as level.dat_new
	// or r.0.0.mcr as directories.  Only strip obvious file-like final names;
	// world names containing dots must still be allowed.
	auto isFileLikeLeaf = [](const jstring& leaf) -> bool
	{
		static const char* exts[] = { ".dat", ".dat_new", ".dat_old", ".mcr", ".txt", ".lock" };
		for (const char* ext : exts)
		{
			const size_t n = std::strlen(ext);
			if (leaf.size() >= n && leaf.compare(leaf.size() - n, n, ext) == 0)
				return true;
		}
		return false;
	};

	size_t leafPos = current.find_last_of("/\\");
	if (leafPos != std::string::npos)
	{
		jstring leaf = current.substr(leafPos + 1);
		if (isFileLikeLeaf(leaf))
			current = current.substr(0, leafPos);
	}
#endif

	while (!current.empty())
	{
		auto fp = File::open(current);
		if (fp->isDirectory())
			break;

		back.emplace(std::move(fp));
		size_t npos = current.find_last_of("/\\");
		if (npos == std::string::npos)
			break;
		current = current.substr(0, npos);
	}

	if (back.empty())
		return false;

	// Create directories
	while (!back.empty())
	{
		auto fp = back.top().get();
		if (!fp->mkdir())
			return false;
		back.pop();
	}

	return true;
}
