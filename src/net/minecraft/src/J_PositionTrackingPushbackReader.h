#pragma once

#include <istream>
#include <deque>
#include <vector>
#include <cstddef>
#include "java/Type.h"

#include "J_ThingWithPosition.h"

// net.minecraft.src.J_PositionTrackingPushbackReader
class J_PositionTrackingPushbackReader : public J_ThingWithPosition
{
public:
    explicit J_PositionTrackingPushbackReader(std::istream* in);

    // func_27334_a - unread one character back into the stream and decrement column
    void unreadChar(int c);

    // func_27335_a - only adjust position tracking backward (does NOT put chars back), used in error paths
    void untrackChars(const char* buf, int len);

    // func_27333_c - read one character, returns -1 on EOF
    int readChar();

    // func_27336_b - read len characters into buf, returns number of chars read
    int readChars(char* buf, int len);

    int getColumn() override;  // func_27331_a
    int getLine()   override;  // func_27330_b

private:
    void trackChar(int ch);

    std::vector<char_t> inputUnits;
    std::size_t inputIndex;
    std::deque<char_t> pushback;  // field_27338_a (unread buffer)
    int              col;       // field_27337_b
    int              line;      // field_27340_c
    bool             lastWasCR; // field_27339_d
};
