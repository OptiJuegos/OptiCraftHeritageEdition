#include "MusInputStream.h"
#include "java/Arithmetic.h"

#include <algorithm>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include "java/String.h"

MusInputStream::MusInputStream(CodecMus *codecmus, const std::string &urlPath, std::istream *inputstream)
    : std::istream(nullptr),
      codec(codecmus),
      hash(0),
      inputStream(inputstream),
      streamBuf(this)
{
    rdbuf(&streamBuf);

    // Derive hash from filename component of the URL path (like Java's url.getPath())
    std::string s = urlPath;
    size_t slash = s.rfind('/');
    if (slash != std::string::npos)
        s = s.substr(slash + 1);

    // Java String.hashCode() runs over UTF-16 code units.
    hash = String::hashCode(jstring(s));
}

int MusInputStream::read()
{
    std::vector<byte_t> tmp(1);
    int n = read(tmp, 0, 1);
    if (n < 0)
        return n;
    return static_cast<int_t>(static_cast<std::int8_t>(tmp[0]));
}

int MusInputStream::read(std::vector<byte_t> &abyte0, int_t i, int_t j)
{
    if (i < 0 || j < 0 || static_cast<std::size_t>(i) > abyte0.size() ||
        static_cast<std::size_t>(j) > abyte0.size() - static_cast<std::size_t>(i))
    {
        throw std::out_of_range("MusInputStream read range");
    }

    // Read j bytes from the underlying stream
    inputStream->read(reinterpret_cast<char *>(abyte0.data() + i), j);
    j = (int_t)inputStream->gcount();

    if (j <= 0)
        return -1;

    // XOR each byte with the rolling hash key
    for (int_t k = 0; k < j; k++)
    {
        byte_t &b = abyte0[i + k];
        b ^= (byte_t)(hash >> 8);
        // Java byte is signed and Java int arithmetic wraps modulo 2^32.
        const std::int32_t signedByte = static_cast<std::int8_t>(b);
        const std::uint32_t wrapped = static_cast<std::uint32_t>(hash) * UINT32_C(0x1dba038f)
                                    + UINT32_C(0x14ee3) * static_cast<std::uint32_t>(signedByte);
        hash = JavaArithmetic::intFromBits(wrapped);
    }

    return j;
}

std::streambuf::int_type MusInputStream::MusStreamBuf::underflow()
{
    int c = owner_->read();
    if (c < 0)
        return traits_type::eof();
    buf_[0] = (char)c;
    setg(buf_, buf_, buf_ + 1);
    return traits_type::to_int_type(buf_[0]);
}
