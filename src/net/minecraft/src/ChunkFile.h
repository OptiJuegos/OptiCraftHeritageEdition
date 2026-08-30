#pragma once

#include <stdexcept>

#include <memory>
#include <string>
#include "java/Type.h"
#include "java/Arithmetic.h"
#include "java/File.h"
#include "ChunkFilePattern.h"
#include "java/String.h"

// net.minecraft.src.ChunkFile
// Wraps a chunk save file; parses x/z coords from the filename.
class ChunkFile
{
public:
	explicit ChunkFile(File *file)
		: field_22326_a(file)
	{
		// Move-only: field_22326_a owns the File*. std::vector (push_back,
		// sort, reallocation) will move ChunkFile instead of copying them,
		// so there's never more than one owner of the underlying File.
		std::string name = file->getName();
		std::string xs, zs;
		if (ChunkFilePattern::parse(name, &xs, &zs))
		{
			int_t x = 0, z = 0;
			if (!String::tryParseInt(xs, x, 36) || !String::tryParseInt(zs, z, 36))
				throw std::invalid_argument("Invalid Java base36 chunk coordinate");
			field_22325_b = x;
			field_22327_c = z;
		}
		else
		{
			field_22325_b = 0;
			field_22327_c = 0;
		}
	}

	// Compare by region (>> 5) so nearby chunks sort together.
	int_t compareTo(const ChunkFile &other) const
	{
		int_t i = JavaArithmetic::intShr(field_22325_b, 5);
		int_t j = JavaArithmetic::intShr(other.field_22325_b, 5);
		if (i == j)
		{
			int_t k = JavaArithmetic::intShr(field_22327_c, 5);
			int_t l = JavaArithmetic::intShr(other.field_22327_c, 5);
			return k - l;
		}
		return i - j;
	}

	bool operator<(const ChunkFile &other) const { return compareTo(other) < 0; }

	File *getFile() const          { return field_22326_a.get(); }  // func_22324_a
	int_t getChunkX() const        { return field_22325_b; }        // func_22323_b
	int_t getChunkZ() const        { return field_22327_c; }        // func_22321_c

private:
	std::unique_ptr<File> field_22326_a;
	int_t field_22325_b;
	int_t field_22327_c;
};
