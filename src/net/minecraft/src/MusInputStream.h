#pragma once

#include <istream>
#include <streambuf>
#include <string>
#include <vector>

#include "java/Type.h"

class CodecMus;

// net.minecraft.src.MusInputStream
// Wraps an existing input stream and XOR-decrypts the bytes using a
// rolling hash derived from the filename.  The same key stream is
// produced by the Java codec so .mus files can be decoded on-the-fly.
class MusInputStream : public std::istream
{
public:
    MusInputStream(CodecMus *codecmus, const std::string &urlPath, std::istream *inputstream);

    // Returns one byte (XOR-decoded), or -1 on EOF.
    int read();

    // Reads up to j bytes into abyte0 starting at offset i.
    // Returns number of bytes read, or -1 on EOF / error.
    int read(std::vector<byte_t> &abyte0, int_t i, int_t j);

    CodecMus *codec;

private:
    // Custom streambuf that delegates to MusInputStream::read()
    class MusStreamBuf : public std::streambuf
    {
    public:
        MusStreamBuf(MusInputStream *owner) : owner_(owner) {}
    protected:
        int_type underflow() override;
    private:
        MusInputStream *owner_;
        char buf_[1];
    };

    int_t          hash;
    std::istream  *inputStream;
    byte_t         buffer[1];
    MusStreamBuf   streamBuf;
};
