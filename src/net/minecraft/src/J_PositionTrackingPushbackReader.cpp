#include "J_PositionTrackingPushbackReader.h"
#include "java/String.h"

#include <iterator>
#include <string>

J_PositionTrackingPushbackReader::J_PositionTrackingPushbackReader(std::istream* in)
    : inputIndex(0)
    , col(0)
    , line(1)
    , lastWasCR(false)
{
    // Java's parser reads characters from a Reader, not raw UTF-8 bytes. Decode
    // once at the boundary so every read below advances by one Java UTF-16 char.
    std::string bytes((std::istreambuf_iterator<char>(*in)), std::istreambuf_iterator<char>());
    if (bytes.size() >= 3 &&
        static_cast<unsigned char>(bytes[0]) == 0xEF &&
        static_cast<unsigned char>(bytes[1]) == 0xBB &&
        static_cast<unsigned char>(bytes[2]) == 0xBF)
        bytes.erase(0, 3);
    inputUnits = String::toUtf16(jstring(bytes));
}

void J_PositionTrackingPushbackReader::unreadChar(int c)
{
    if (c < 0) return;
    col--;
    if (col < 0)
        col = 0;
    pushback.push_front(static_cast<char_t>(c));
}

void J_PositionTrackingPushbackReader::untrackChars(const char* /*buf*/, int len)
{
    col -= len;
    if (col < 0)
        col = 0;
}

int J_PositionTrackingPushbackReader::readChar()
{
    int ch;
    if (!pushback.empty())
    {
        ch = pushback.front();
        pushback.pop_front();
    }
    else
    {
        if (inputIndex >= inputUnits.size()) return -1;
        ch = inputUnits[inputIndex++];
    }
    trackChar(ch);
    return ch;
}

int J_PositionTrackingPushbackReader::readChars(char* buf, int len)
{
    int count = 0;
    for (int i = 0; i < len; i++)
    {
        int ch = readChar();
        if (ch == -1)
            break;
        // This overload is only used for JSON grammar literals and hex digits,
        // all of which are ASCII. Non-ASCII content is handled by parseString.
        buf[i] = static_cast<char>(ch & 0xff);
        count++;
    }
    return count;
}

void J_PositionTrackingPushbackReader::trackChar(int ch)
{
    if (ch == '\r')
    {
        col = 0;
        line++;
        lastWasCR = true;
    }
    else if (ch == '\n' && !lastWasCR)
    {
        col = 0;
        line++;
        lastWasCR = false;
    }
    else
    {
        col++;
        lastWasCR = false;
    }
}

int J_PositionTrackingPushbackReader::getColumn() { return col; }
int J_PositionTrackingPushbackReader::getLine() { return line; }
