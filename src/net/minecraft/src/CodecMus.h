#pragma once

#include <istream>
#include <string>

// net.minecraft.src.CodecMus
// Java: extends CodecJOrbis (paulscode audio codec).
// C++: lightweight adapter for the .mus audio codec used by Minecraft.
// Actual decoding is handled by SDL2_mixer / stb_vorbis; this class
// exists to carry the URL path so MusInputStream can derive its XOR key.
class CodecMus
{
public:
    CodecMus();
    virtual ~CodecMus() = default;

    void setURL(const std::string &path);
    const std::string &getURL() const;

protected:
    // Returns a MusInputStream wrapping the raw OGG data stream.
    // In Java this overrides CodecJOrbis::openInputStream().
    std::istream *openInputStream(std::istream *rawStream);

private:
    std::string url;
};
