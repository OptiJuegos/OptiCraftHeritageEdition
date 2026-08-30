#include "MD5String.h"

#include <cstring>
#include <cstdint>
#include <sstream>
#include <iomanip>

// Portable MD5 implementation (RFC 1321)
namespace
{

struct MD5Context
{
    uint32_t state[4];
    uint32_t count[2];
    uint8_t  buffer[64];
};

static const uint32_t S[64] = {
    7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
    5, 9,14,20, 5, 9,14,20, 5, 9,14,20, 5, 9,14,20,
    4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
    6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
};

static const uint32_t K[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,
    0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,
    0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,
    0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,
    0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,
    0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,
    0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,
    0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,
    0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};

inline uint32_t leftRotate(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }

static void md5Transform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t M[16];
    for (int i = 0; i < 16; i++)
        M[i] = (uint32_t)block[i*4] | ((uint32_t)block[i*4+1] << 8) | ((uint32_t)block[i*4+2] << 16) | ((uint32_t)block[i*4+3] << 24);

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];

    for (int i = 0; i < 64; i++)
    {
        uint32_t F, g;
        if      (i < 16) { F = (b & c) | (~b & d); g = (uint32_t)i; }
        else if (i < 32) { F = (d & b) | (~d & c); g = (5u * i + 1u) % 16u; }
        else if (i < 48) { F = b ^ c ^ d;            g = (3u * i + 5u) % 16u; }
        else             { F = c ^ (b | ~d);          g = (7u * i)      % 16u; }

        F += a + K[i] + M[g];
        a = d; d = c; c = b;
        b += leftRotate(F, S[i]);
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}

static std::string md5Compute(const std::string &input)
{
    uint32_t state[4] = { 0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u };

    const uint8_t *data = reinterpret_cast<const uint8_t*>(input.data());
    size_t len = input.size();

    uint8_t block[64];
    size_t i = 0;

    // Process full 64-byte blocks
    for (; i + 64 <= len; i += 64)
        md5Transform(state, data + i);

    // Build final block(s) with padding
    size_t rem = len - i;
    memcpy(block, data + i, rem);
    block[rem] = 0x80;
    memset(block + rem + 1, 0, 63 - rem);

    if (rem >= 56)
    {
        md5Transform(state, block);
        memset(block, 0, 56);
    }

    uint64_t bitLen = (uint64_t)len * 8u;
    for (int j = 0; j < 8; j++)
        block[56 + j] = (uint8_t)(bitLen >> (j * 8));

    md5Transform(state, block);

    // Output hex string
    uint8_t digest[16];
    for (int j = 0; j < 4; j++)
    {
        digest[j*4+0] = (uint8_t)(state[j]);
        digest[j*4+1] = (uint8_t)(state[j] >> 8);
        digest[j*4+2] = (uint8_t)(state[j] >> 16);
        digest[j*4+3] = (uint8_t)(state[j] >> 24);
    }

    std::ostringstream oss;
    for (int j = 0; j < 16; j++)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[j];
    return oss.str();
}

} // anonymous namespace

MD5String::MD5String(const std::string &s) :
    prefix(s)
{
}

std::string MD5String::computeHash(const std::string &s)
{
    return md5Compute(prefix + s);
}

std::string MD5String::md5Hex(const std::string &input)
{
    return md5Compute(input);
}
