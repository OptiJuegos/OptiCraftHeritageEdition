#include "java/String.h"
#include "java/Arithmetic.h"
#include "java/Unicode.h"

#include <limits>
#include <stdexcept>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <cstring>

namespace String
{

template <typename T>
static jstring intToStringImpl(T value, int_t base)
{
	if (base < 2 || base > 36)
		base = 10;

	using U = typename std::make_unsigned<T>::type;
	bool negative = false;
	U magnitude;
	if constexpr (std::is_signed<T>::value)
	{
		negative = value < 0;
		const U bits = static_cast<U>(value);
		magnitude = negative ? U(0) - bits : bits;
	}
	else
	{
		magnitude = value;
	}

	jstring out;
	do
	{
		const U digit = magnitude % static_cast<U>(base);
		magnitude /= static_cast<U>(base);
		out.insert(out.begin(), static_cast<char>(digit + (digit < 10 ? '0' : 'a' - 10)));
	}
	while (magnitude != 0);

	if (negative)
		out.insert(out.begin(), '-');
	return out;
}

jstring toString(int_t v, int_t base) { return intToStringImpl<int_t>(v, base); }
jstring toString(long_t v, int_t base) { return intToStringImpl<long_t>(v, base); }
jstring toString(uint_t v, int_t base) { return intToStringImpl<uint_t>(v, base); }
jstring toString(ulong_t v, int_t base) { return intToStringImpl<ulong_t>(v, base); }
namespace
{
template <typename T>
static bool sameBits(T a, T b)
{
    unsigned char aa[sizeof(T)], bb[sizeof(T)];
    std::memcpy(aa, &a, sizeof(T));
    std::memcpy(bb, &b, sizeof(T));
    return std::memcmp(aa, bb, sizeof(T)) == 0;
}

template <typename T>
static bool parseClassicExact(const std::string &text, T &out)
{
    std::istringstream in(text);
    in.imbue(std::locale::classic());
    in >> out;
    return !in.fail() && in.peek() == std::char_traits<char>::eof();
}

static std::string normalizeJavaDecimal(const std::string &token, bool negative)
{
    std::string s = token;
    if (!s.empty() && (s[0] == '+' || s[0] == '-')) s.erase(s.begin());

    int expPart = 0;
    std::size_t epos = s.find_first_of("eE");
    if (epos != std::string::npos)
    {
        expPart = std::stoi(s.substr(epos + 1));
        s.resize(epos);
    }

    std::size_t dot = s.find('.');
    int digitsBeforeDot = dot == std::string::npos ? static_cast<int>(s.size()) : static_cast<int>(dot);
    std::string digits = s;
    if (dot != std::string::npos) digits.erase(dot, 1);

    std::size_t first = digits.find_first_not_of('0');
    if (first == std::string::npos)
        return negative ? "-0.0" : "0.0";

    int scientificExp = expPart + digitsBeforeDot - 1 - static_cast<int>(first);
    digits.erase(0, first);
    while (digits.size() > 1 && digits.back() == '0') digits.pop_back();

    std::string out;
    if (negative) out.push_back('-');
    if (scientificExp >= -3 && scientificExp < 7)
    {
        int point = scientificExp + 1;
        if (point <= 0)
        {
            out += "0.";
            out.append(static_cast<std::size_t>(-point), '0');
            out += digits;
        }
        else if (point >= static_cast<int>(digits.size()))
        {
            out += digits;
            out.append(static_cast<std::size_t>(point - static_cast<int>(digits.size())), '0');
            out += ".0";
        }
        else
        {
            out.append(digits, 0, static_cast<std::size_t>(point));
            out.push_back('.');
            out.append(digits, static_cast<std::size_t>(point), std::string::npos);
        }
    }
    else
    {
        out.push_back(digits[0]);
        out.push_back('.');
        if (digits.size() == 1) out.push_back('0');
        else out.append(digits, 1, std::string::npos);
        out.push_back('E');
        out += std::to_string(scientificExp);
    }
    return out;
}

template <typename T>
static jstring javaFpToString(T value)
{
    if (std::isnan(value)) return "NaN";
    if (std::isinf(value)) return std::signbit(value) ? "-Infinity" : "Infinity";
    if (value == static_cast<T>(0)) return std::signbit(value) ? "-0.0" : "0.0";

    const bool negative = std::signbit(value);
    const T magnitude = negative ? -value : value;
    std::string best;
    for (int precision = 1; precision <= std::numeric_limits<T>::max_digits10; ++precision)
    {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << std::setprecision(precision) << std::defaultfloat << magnitude;
        std::string candidate = out.str();
        T parsed{};
        if (parseClassicExact(candidate, parsed) && sameBits(parsed, magnitude))
        {
            best = candidate;
            break;
        }
    }
    if (best.empty())
    {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << std::setprecision(std::numeric_limits<T>::max_digits10) << std::defaultfloat << magnitude;
        best = out.str();
    }
    return normalizeJavaDecimal(best, negative);
}
}

jstring toString(float v) { return javaFpToString(v); }
jstring toString(double v) { return javaFpToString(v); }

static void appendCodePointUtf16(std::vector<char_t> &out, std::uint32_t cp)
{
	if (cp <= 0xffffu)
	{
		out.push_back(static_cast<char_t>(cp));
		return;
	}
	cp -= 0x10000u;
	out.push_back(static_cast<char_t>(0xd800u + (cp >> 10)));
	out.push_back(static_cast<char_t>(0xdc00u + (cp & 0x3ffu)));
}

std::vector<char_t> toUtf16(const jstring &value)
{
	std::vector<char_t> out;
	out.reserve(value.size());
	for (std::size_t i = 0; i < value.size();)
	{
		const std::uint8_t c0 = static_cast<std::uint8_t>(value[i]);
		std::uint32_t cp = 0xfffdu;
		std::size_t count = 1;
		if (c0 < 0x80u)
		{
			cp = c0;
		}
		else if ((c0 & 0xe0u) == 0xc0u && i + 1 < value.size())
		{
			const std::uint8_t c1 = static_cast<std::uint8_t>(value[i + 1]);
			if ((c1 & 0xc0u) == 0x80u)
			{
				cp = ((c0 & 0x1fu) << 6) | (c1 & 0x3fu);
				if (cp >= 0x80u) count = 2; else cp = 0xfffdu;
			}
		}
		else if ((c0 & 0xf0u) == 0xe0u && i + 2 < value.size())
		{
			const std::uint8_t c1 = static_cast<std::uint8_t>(value[i + 1]);
			const std::uint8_t c2 = static_cast<std::uint8_t>(value[i + 2]);
			if ((c1 & 0xc0u) == 0x80u && (c2 & 0xc0u) == 0x80u)
			{
				cp = ((c0 & 0x0fu) << 12) | ((c1 & 0x3fu) << 6) | (c2 & 0x3fu);
				if (cp >= 0x800u) count = 3; else cp = 0xfffdu;
			}
		}
		else if ((c0 & 0xf8u) == 0xf0u && i + 3 < value.size())
		{
			const std::uint8_t c1 = static_cast<std::uint8_t>(value[i + 1]);
			const std::uint8_t c2 = static_cast<std::uint8_t>(value[i + 2]);
			const std::uint8_t c3 = static_cast<std::uint8_t>(value[i + 3]);
			if ((c1 & 0xc0u) == 0x80u && (c2 & 0xc0u) == 0x80u && (c3 & 0xc0u) == 0x80u)
			{
				cp = ((c0 & 0x07u) << 18) | ((c1 & 0x3fu) << 12) | ((c2 & 0x3fu) << 6) | (c3 & 0x3fu);
				if (cp >= 0x10000u && cp <= 0x10ffffu) count = 4; else cp = 0xfffdu;
			}
		}
		appendCodePointUtf16(out, cp);
		i += count;
	}
	return out;
}

static void appendUtf8(jstring &out, std::uint32_t cp)
{
	if (cp <= 0x7fu)
		out.push_back(static_cast<char>(cp));
	else if (cp <= 0x7ffu)
	{
		out.push_back(static_cast<char>(0xc0u | (cp >> 6)));
		out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
	}
	else if (cp <= 0xffffu)
	{
		out.push_back(static_cast<char>(0xe0u | (cp >> 12)));
		out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3fu)));
		out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
	}
	else
	{
		out.push_back(static_cast<char>(0xf0u | (cp >> 18)));
		out.push_back(static_cast<char>(0x80u | ((cp >> 12) & 0x3fu)));
		out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3fu)));
		out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
	}
}

jstring fromUtf16(const std::vector<char_t> &value)
{
	jstring out;
	for (std::size_t i = 0; i < value.size(); ++i)
	{
		std::uint32_t cp = value[i];
		if (cp >= 0xd800u && cp <= 0xdbffu && i + 1 < value.size())
		{
			const std::uint32_t low = value[i + 1];
			if (low >= 0xdc00u && low <= 0xdfffu)
			{
				cp = 0x10000u + ((cp - 0xd800u) << 10) + (low - 0xdc00u);
				++i;
			}
		}
		appendUtf8(out, cp);
	}
	return out;
}

int_t utf16Length(const jstring &value)
{
	return static_cast<int_t>(toUtf16(value).size());
}

jstring substringUtf16(const jstring &value, int_t beginIndex, int_t endIndex)
{
	std::vector<char_t> units = toUtf16(value);
	if (beginIndex < 0 || endIndex < beginIndex || static_cast<std::size_t>(endIndex) > units.size())
		throw std::out_of_range("Java String substring index out of range");
	return fromUtf16(std::vector<char_t>(units.begin() + beginIndex, units.begin() + endIndex));
}

jstring truncateUtf16(const jstring &value, int_t maxLength)
{
	if (maxLength < 0)
		throw std::out_of_range("Java String maximum length is negative");
	const int_t length = utf16Length(value);
	return length <= maxLength ? value : substringUtf16(value, 0, maxLength);
}

jstring removeLastUtf16Unit(const jstring &value)
{
	const int_t length = utf16Length(value);
	return length == 0 ? value : substringUtf16(value, 0, length - 1);
}

void appendUtf16Unit(jstring &value, char_t unit)
{
	std::vector<char_t> units = toUtf16(value);
	units.push_back(unit);
	value = fromUtf16(units);
}

int_t indexOfUtf16Unit(const jstring &value, char_t unit)
{
	const std::vector<char_t> units = toUtf16(value);
	for (std::size_t i = 0; i < units.size(); ++i)
	{
		if (units[i] == unit)
			return static_cast<int_t>(i);
	}
	return -1;
}

int_t hashCode(const jstring &value)
{
	std::uint32_t hash = 0;
	for (char_t unit : toUtf16(value))
		hash = hash * 31u + static_cast<std::uint32_t>(unit);
	return JavaArithmetic::intFromBits(hash);
}

bool tryParseLong(const jstring &value, long_t &result)
{
	if (value.empty())
		return false;

	std::size_t index = 0;
	bool negative = false;
	if (value[index] == '-' || value[index] == '+')
	{
		negative = value[index] == '-';
		if (++index == value.size())
			return false;
	}

	const std::uint64_t limit = negative
		? (UINT64_C(1) << 63)
		: static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
	std::uint64_t magnitude = 0;
	for (; index < value.size(); ++index)
	{
		const unsigned char c = static_cast<unsigned char>(value[index]);
		if (c < '0' || c > '9')
			return false;
		const std::uint64_t digit = c - '0';
		if (magnitude > (limit - digit) / 10u)
			return false;
		magnitude = magnitude * 10u + digit;
	}

	if (negative)
	{
		if (magnitude == (UINT64_C(1) << 63))
			result = std::numeric_limits<long_t>::min();
		else
			result = -static_cast<long_t>(magnitude);
	}
	else
		result = static_cast<long_t>(magnitude);
	return true;
}

bool tryParseInt(const jstring &value, int_t &result, int_t base)
{
	if (value.empty() || base < 2 || base > 36) return false;
	std::size_t index = 0;
	bool negative = false;
	if (value[index] == '-' || value[index] == '+') {
		negative = value[index] == '-';
		if (++index == value.size()) return false;
	}
	const std::uint64_t limit = negative ? UINT64_C(2147483648) : UINT64_C(2147483647);
	std::uint64_t magnitude = 0;
	for (; index < value.size(); ++index) {
		const unsigned char c = static_cast<unsigned char>(value[index]);
		int digit = -1;
		if (c >= '0' && c <= '9') digit = c - '0';
		else if (c >= 'a' && c <= 'z') digit = c - 'a' + 10;
		else if (c >= 'A' && c <= 'Z') digit = c - 'A' + 10;
		if (digit < 0 || digit >= base) return false;
		if (magnitude > (limit - static_cast<std::uint64_t>(digit)) / static_cast<std::uint64_t>(base)) return false;
		magnitude = magnitude * static_cast<std::uint64_t>(base) + static_cast<std::uint64_t>(digit);
	}
	if (negative) {
		if (magnitude == UINT64_C(2147483648)) result = std::numeric_limits<int_t>::min();
		else result = -static_cast<int_t>(magnitude);
	} else result = static_cast<int_t>(magnitude);
	return true;
}

jstring trimJava(const jstring &value)
{
	std::vector<char_t> units = toUtf16(value);
	std::size_t begin = 0, end = units.size();
	while (begin < end && units[begin] <= 0x20u) ++begin;
	while (end > begin && units[end - 1] <= 0x20u) --end;
	return fromUtf16(std::vector<char_t>(units.begin() + begin, units.begin() + end));
}

std::vector<jstring> splitJava(const jstring &value, char delimiter)
{
	std::vector<jstring> result;
	std::size_t start = 0;
	for (std::size_t i = 0; i <= value.size(); ++i) {
		if (i == value.size() || value[i] == delimiter) {
			result.emplace_back(value.substr(start, i - start));
			start = i + 1;
		}
	}
	// Java String.split(regex) with limit=0 discards trailing empty strings.
	while (result.size() > 1 && result.back().empty()) result.pop_back();
	return result;
}

bool equalsIgnoreCaseJava(const jstring &left, const jstring &right)
{
    const std::vector<char_t> a = toUtf16(left);
    const std::vector<char_t> b = toUtf16(right);
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (a[i] == b[i]) continue;
        const char_t au = JavaUnicode::toUpperCase(a[i]);
        const char_t bu = JavaUnicode::toUpperCase(b[i]);
        if (au == bu) continue;
        if (JavaUnicode::toLowerCase(au) != JavaUnicode::toLowerCase(bu)) return false;
    }
    return true;
}

jstring toLowerCaseJava(const jstring &value)
{
    std::vector<char_t> units = toUtf16(value);
    for (char_t &c : units) c = JavaUnicode::toLowerCase(c);
    return fromUtf16(units);
}

}
