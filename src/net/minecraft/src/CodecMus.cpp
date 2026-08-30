#include "CodecMus.h"

#include "MusInputStream.h"

CodecMus::CodecMus()
{
}

void CodecMus::setURL(const std::string &path)
{
    url = path;
}

const std::string &CodecMus::getURL() const
{
    return url;
}

std::istream *CodecMus::openInputStream(std::istream *rawStream)
{
    return new MusInputStream(this, url, rawStream);
}
