#pragma once

#include <istream>
#include <string>
#include <vector>

#include "J_InvalidSyntaxException.h"
#include "java/Type.h"

class J_JsonListener;
class J_PositionTrackingPushbackReader;

// net.minecraft.src.J_SajParser
// SAX-style JSON parser — fires events on a J_JsonListener as it reads.
class J_SajParser
{
public:
    J_SajParser() {}

    // func_27463_a - parse from an input stream and fire events on the listener
    void parse(std::istream& in, J_JsonListener* listener);

private:
    // func_27455_a - parse array '[' ... ']'
    void parseArray(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener);

    // func_27453_b - parse object '{' ... '}'
    void parseObject(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener);

    // func_27449_c - parse one key-value pair inside an object
    void parseField(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener);

    // func_27464_d - parse one value (string, number, true/false/null, array, object)
    void parseValue(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener);

    // func_27459_a - parse a number (possibly negative)
    std::string parseNumber(J_PositionTrackingPushbackReader& reader);

    // func_27451_b - parse digits after sign
    std::string parsePositiveNumber(J_PositionTrackingPushbackReader& reader);

    // func_27460_c - read first digit 1-9
    char parseNonZeroDigit(J_PositionTrackingPushbackReader& reader);

    // func_27458_d - read digit 0-9
    char parseDigit(J_PositionTrackingPushbackReader& reader);

    // func_27456_e - read zero or more digits 0-9
    std::string parseMoreDigits(J_PositionTrackingPushbackReader& reader);

    // func_27462_f - parse optional fractional part '.ddd'
    std::string parseFractional(J_PositionTrackingPushbackReader& reader);

    // func_27454_g - parse optional exponent part 'E...'
    std::string parseExponent(J_PositionTrackingPushbackReader& reader);

    // func_27461_h - parse optional sign '+'/'-'
    std::string parseSign(J_PositionTrackingPushbackReader& reader);

    // func_27452_i - parse a JSON string (with '"' delimiters and escape handling)
    std::string parseString(J_PositionTrackingPushbackReader& reader);

    // func_27457_j - parse one escape sequence after '\'
    void parseEscapeChar(J_PositionTrackingPushbackReader& reader, std::vector<char_t>& out);

    // func_27450_k - parse 4-digit unicode hex '\uXXXX'
    char_t parseUnicode(J_PositionTrackingPushbackReader& reader);

    // func_27448_l - skip whitespace and return next non-whitespace character
    int skipWhitespace(J_PositionTrackingPushbackReader& reader);
};
