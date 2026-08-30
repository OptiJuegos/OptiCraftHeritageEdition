#include "java/IOUtil.h"
#include "java/Arithmetic.h"

#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace IOUtil
{

namespace
{
int readRequiredByte(std::istream &is)
{
    const int value = is.get();
    if (value == std::char_traits<char>::eof())
        throw std::runtime_error("Unexpected end of NBT input");
    return value & 0xff;
}

void putByte(std::ostream &os, std::uint8_t value)
{
    os.put(static_cast<char>(value));
}

void appendUtf8(std::string &out, std::uint32_t cp)
{
    if (cp <= 0x7f)
    {
        out.push_back(static_cast<char>(cp));
    }
    else if (cp <= 0x7ff)
    {
        out.push_back(static_cast<char>(0xc0u | (cp >> 6)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
    }
    else if (cp <= 0xffff)
    {
        // This also intentionally supports isolated UTF-16 surrogates using
        // their WTF-8 byte representation. Java modified UTF-8 can encode an
        // isolated surrogate code unit and this preserves it losslessly.
        out.push_back(static_cast<char>(0xe0u | (cp >> 12)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
    }
    else if (cp <= 0x10ffff)
    {
        out.push_back(static_cast<char>(0xf0u | (cp >> 18)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 12) & 0x3fu)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
    }
    else
    {
        throw std::runtime_error("Invalid Unicode code point");
    }
}

std::vector<std::uint16_t> utf8ToUtf16(const std::string &str)
{
    std::vector<std::uint16_t> units;
    units.reserve(str.size());

    for (std::size_t i = 0; i < str.size();)
    {
        const std::uint8_t b0 = static_cast<std::uint8_t>(str[i]);
        std::uint32_t cp = 0;
        std::size_t count = 0;

        if (b0 < 0x80)
        {
            cp = b0;
            count = 1;
        }
        else if ((b0 & 0xe0u) == 0xc0u)
        {
            cp = b0 & 0x1fu;
            count = 2;
            if (cp == 0)
            {
                // C0/C1 are overlong in standard UTF-8. jstring is expected to
                // contain standard UTF-8; reject them rather than silently
                // changing the Java String.
                throw std::runtime_error("Invalid UTF-8 string");
            }
        }
        else if ((b0 & 0xf0u) == 0xe0u)
        {
            cp = b0 & 0x0fu;
            count = 3;
        }
        else if ((b0 & 0xf8u) == 0xf0u)
        {
            cp = b0 & 0x07u;
            count = 4;
        }
        else
        {
            throw std::runtime_error("Invalid UTF-8 string");
        }

        if (i + count > str.size())
            throw std::runtime_error("Truncated UTF-8 string");

        for (std::size_t j = 1; j < count; ++j)
        {
            const std::uint8_t bx = static_cast<std::uint8_t>(str[i + j]);
            if ((bx & 0xc0u) != 0x80u)
                throw std::runtime_error("Invalid UTF-8 continuation byte");
            cp = (cp << 6) | (bx & 0x3fu);
        }

        const std::uint32_t minCp = count == 1 ? 0u : count == 2 ? 0x80u : count == 3 ? 0x800u : 0x10000u;
        if (cp < minCp || cp > 0x10ffffu)
            throw std::runtime_error("Invalid UTF-8 code point");

        // Allow a three-byte surrogate sequence as WTF-8 so a modified-UTF
        // isolated surrogate can round-trip through jstring.
        if (cp >= 0xd800u && cp <= 0xdfffu)
        {
            if (count != 3)
                throw std::runtime_error("Invalid UTF-8 surrogate");
            units.push_back(static_cast<std::uint16_t>(cp));
        }
        else if (cp <= 0xffffu)
        {
            units.push_back(static_cast<std::uint16_t>(cp));
        }
        else
        {
            cp -= 0x10000u;
            units.push_back(static_cast<std::uint16_t>(0xd800u + (cp >> 10)));
            units.push_back(static_cast<std::uint16_t>(0xdc00u + (cp & 0x3ffu)));
        }

        i += count;
    }

    return units;
}

std::vector<std::uint8_t> encodeModifiedUtf8(const std::string &str)
{
    const std::vector<std::uint16_t> units = utf8ToUtf16(str);
    std::vector<std::uint8_t> out;
    out.reserve(str.size() + 8);

    for (std::uint16_t unit : units)
    {
        if (unit >= 0x0001u && unit <= 0x007fu)
        {
            out.push_back(static_cast<std::uint8_t>(unit));
        }
        else if (unit <= 0x07ffu)
        {
            out.push_back(static_cast<std::uint8_t>(0xc0u | (unit >> 6)));
            out.push_back(static_cast<std::uint8_t>(0x80u | (unit & 0x3fu)));
        }
        else
        {
            out.push_back(static_cast<std::uint8_t>(0xe0u | (unit >> 12)));
            out.push_back(static_cast<std::uint8_t>(0x80u | ((unit >> 6) & 0x3fu)));
            out.push_back(static_cast<std::uint8_t>(0x80u | (unit & 0x3fu)));
        }
    }

    if (out.size() > std::numeric_limits<std::uint16_t>::max())
        throw std::runtime_error("Modified UTF-8 string is longer than 65535 bytes");

    return out;
}

std::string decodeModifiedUtf8(const std::vector<std::uint8_t> &bytes)
{
    std::vector<std::uint16_t> units;
    units.reserve(bytes.size());

    for (std::size_t i = 0; i < bytes.size();)
    {
        const std::uint8_t b0 = bytes[i++];
        if (b0 != 0 && b0 <= 0x7f)
        {
            units.push_back(b0);
            continue;
        }

        if ((b0 & 0xe0u) == 0xc0u)
        {
            if (i >= bytes.size())
                throw std::runtime_error("Truncated modified UTF-8 string");
            const std::uint8_t b1 = bytes[i++];
            if ((b1 & 0xc0u) != 0x80u)
                throw std::runtime_error("Invalid modified UTF-8 continuation byte");
            const std::uint16_t unit = static_cast<std::uint16_t>(((b0 & 0x1fu) << 6) | (b1 & 0x3fu));
            // Modified UTF-8 permits C0 80 only for U+0000; other overlong
            // two-byte forms are invalid.
            if (unit != 0 && unit < 0x80u)
                throw std::runtime_error("Overlong modified UTF-8 sequence");
            units.push_back(unit);
            continue;
        }

        if ((b0 & 0xf0u) == 0xe0u)
        {
            if (i + 1 >= bytes.size())
                throw std::runtime_error("Truncated modified UTF-8 string");
            const std::uint8_t b1 = bytes[i++];
            const std::uint8_t b2 = bytes[i++];
            if ((b1 & 0xc0u) != 0x80u || (b2 & 0xc0u) != 0x80u)
                throw std::runtime_error("Invalid modified UTF-8 continuation byte");
            const std::uint16_t unit = static_cast<std::uint16_t>(((b0 & 0x0fu) << 12)
                                                               | ((b1 & 0x3fu) << 6)
                                                               |  (b2 & 0x3fu));
            if (unit < 0x800u)
                throw std::runtime_error("Overlong modified UTF-8 sequence");
            units.push_back(unit);
            continue;
        }

        // Raw NUL and four-byte standard UTF-8 are not legal in Java's
        // DataInput modified UTF-8 representation.
        throw std::runtime_error("Invalid modified UTF-8 lead byte");
    }

    std::string out;
    out.reserve(bytes.size());
    for (std::size_t i = 0; i < units.size(); ++i)
    {
        const std::uint16_t unit = units[i];
        if (unit >= 0xd800u && unit <= 0xdbffu && i + 1 < units.size()
            && units[i + 1] >= 0xdc00u && units[i + 1] <= 0xdfffu)
        {
            const std::uint32_t cp = 0x10000u
                + ((static_cast<std::uint32_t>(unit) - 0xd800u) << 10)
                +  (static_cast<std::uint32_t>(units[++i]) - 0xdc00u);
            appendUtf8(out, cp);
        }
        else
        {
            appendUtf8(out, unit);
        }
    }
    return out;
}

} // namespace

void writeBoolean(std::ostream &os, bool b)
{
    putByte(os, b ? 1u : 0u);
}

void writeByte(std::ostream &os, int_t b)
{
    putByte(os, static_cast<std::uint8_t>(b));
}

void writeChar(std::ostream &os, int_t c)
{
    const std::uint16_t value = static_cast<std::uint16_t>(c);
    putByte(os, static_cast<std::uint8_t>(value >> 8));
    putByte(os, static_cast<std::uint8_t>(value));
}

void writeShort(std::ostream &os, int_t s)
{
    const std::uint16_t value = static_cast<std::uint16_t>(s);
    putByte(os, static_cast<std::uint8_t>(value >> 8));
    putByte(os, static_cast<std::uint8_t>(value));
}

void writeInt(std::ostream &os, int_t i)
{
    const std::uint32_t value = static_cast<std::uint32_t>(i);
    putByte(os, static_cast<std::uint8_t>(value >> 24));
    putByte(os, static_cast<std::uint8_t>(value >> 16));
    putByte(os, static_cast<std::uint8_t>(value >> 8));
    putByte(os, static_cast<std::uint8_t>(value));
}

void writeLong(std::ostream &os, long_t l)
{
    const std::uint64_t value = static_cast<std::uint64_t>(l);
    putByte(os, static_cast<std::uint8_t>(value >> 56));
    putByte(os, static_cast<std::uint8_t>(value >> 48));
    putByte(os, static_cast<std::uint8_t>(value >> 40));
    putByte(os, static_cast<std::uint8_t>(value >> 32));
    putByte(os, static_cast<std::uint8_t>(value >> 24));
    putByte(os, static_cast<std::uint8_t>(value >> 16));
    putByte(os, static_cast<std::uint8_t>(value >> 8));
    putByte(os, static_cast<std::uint8_t>(value));
}

void writeFloat(std::ostream &os, float f)
{
    if (std::isnan(f)) { writeInt(os, JavaArithmetic::intFromBits(0x7fc00000u)); return; }
    int_t bits;
    static_assert(sizeof(bits) == sizeof(f), "Unexpected float size");
    std::memcpy(&bits, &f, sizeof(bits));
    writeInt(os, bits);
}

void writeDouble(std::ostream &os, double d)
{
    if (std::isnan(d)) { writeLong(os, JavaArithmetic::longFromBits(UINT64_C(0x7ff8000000000000))); return; }
    long_t bits;
    static_assert(sizeof(bits) == sizeof(d), "Unexpected double size");
    std::memcpy(&bits, &d, sizeof(bits));
    writeLong(os, bits);
}

void writeUTF(std::ostream &os, const jstring &str)
{
    const std::vector<std::uint8_t> bytes = encodeModifiedUtf8(static_cast<const std::string &>(str));
    if (bytes.size() > 65535u)
        throw std::runtime_error("Modified UTF-8 string exceeds 65535 bytes");
    writeShort(os, static_cast<int_t>(bytes.size()));
    if (!bytes.empty())
        os.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

bool readBoolean(std::istream &is)
{
    return readRequiredByte(is) != 0;
}

byte_t readByte(std::istream &is)
{
    return JavaArithmetic::byteFromBits(static_cast<ubyte_t>(readRequiredByte(is)));
}

int_t readUnsignedByte(std::istream &is)
{
    return readRequiredByte(is);
}

char_t readChar(std::istream &is)
{
    const std::uint16_t b0 = static_cast<std::uint16_t>(readRequiredByte(is));
    const std::uint16_t b1 = static_cast<std::uint16_t>(readRequiredByte(is));
    return static_cast<char_t>((b0 << 8) | b1);
}

short_t readShort(std::istream &is)
{
    const std::uint16_t b0 = static_cast<std::uint16_t>(readRequiredByte(is));
    const std::uint16_t b1 = static_cast<std::uint16_t>(readRequiredByte(is));
    return JavaArithmetic::shortFromBits(static_cast<ushort_t>((b0 << 8) | b1));
}

int_t readInt(std::istream &is)
{
    const std::uint32_t b0 = static_cast<std::uint32_t>(readRequiredByte(is));
    const std::uint32_t b1 = static_cast<std::uint32_t>(readRequiredByte(is));
    const std::uint32_t b2 = static_cast<std::uint32_t>(readRequiredByte(is));
    const std::uint32_t b3 = static_cast<std::uint32_t>(readRequiredByte(is));
    return JavaArithmetic::intFromBits((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
}

long_t readLong(std::istream &is)
{
    const std::uint64_t b0 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t b1 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t b2 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t b3 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t b4 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t b5 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t b6 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t b7 = static_cast<std::uint64_t>(readRequiredByte(is));
    const std::uint64_t bits = (b0 << 56) | (b1 << 48) | (b2 << 40) | (b3 << 32)
                             | (b4 << 24) | (b5 << 16) | (b6 << 8) | b7;
    long_t value;
    static_assert(sizeof(value) == sizeof(bits), "Unexpected Java long size");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

float readFloat(std::istream &is)
{
    int_t bits = readInt(is);
    float value;
    static_assert(sizeof(value) == sizeof(bits), "Unexpected float size");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

double readDouble(std::istream &is)
{
    long_t bits = readLong(is);
    double value;
    static_assert(sizeof(value) == sizeof(bits), "Unexpected double size");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

jstring readUTF(std::istream &is)
{
    const std::size_t len = (static_cast<std::size_t>(readRequiredByte(is)) << 8)
                          | static_cast<std::size_t>(readRequiredByte(is));
    std::vector<std::uint8_t> bytes(len);
    if (len != 0)
        is.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(len));
    if (!is)
        throw std::runtime_error("Truncated NBT string");
    return jstring(decodeModifiedUtf8(bytes));
}

std::vector<char> readAllBytes(std::istream &is)
{
    is.seekg(0, std::ios::end);
    std::streampos size = is.tellg();
    is.seekg(0, std::ios::beg);

    if (size < 0)
        throw std::runtime_error("Unable to determine input size");

    std::vector<char> buffer(static_cast<std::size_t>(size));
    if (!buffer.empty())
        is.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    return buffer;
}

} // namespace IOUtil
