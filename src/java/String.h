#pragma once

#include <string>
#include <utility>
#include <functional>
#include <vector>

#include "java/Type.h"

class jstring : public std::string
{
public:
	using std::string::string;

	jstring() = default;
	jstring(const jstring &) = default;
	jstring(jstring &&) noexcept = default;
	jstring(std::nullptr_t) : std::string(), mIsNull(true) {}
	jstring(const std::string &value) : std::string(value) {}
	jstring(std::string &&value) noexcept : std::string(std::move(value)) {}
	jstring(const char *value) : std::string(value ? value : ""), mIsNull(value == nullptr) {}

	jstring &operator=(const jstring &) = default;
	jstring &operator=(jstring &&) noexcept = default;
	jstring &operator=(const std::string &value) { std::string::operator=(value); mIsNull = false; return *this; }
	jstring &operator=(std::string &&value) noexcept { std::string::operator=(std::move(value)); mIsNull = false; return *this; }
	jstring &operator=(const char *value) { std::string::operator=(value ? value : ""); mIsNull = value == nullptr; return *this; }
	jstring &operator=(std::nullptr_t) { clear(); mIsNull = true; return *this; }

	bool isNull() const noexcept { return mIsNull; }

private:
	bool mIsNull = false;
};

inline bool operator==(const jstring &left, const jstring &right)
{
	return static_cast<const std::string &>(left) == static_cast<const std::string &>(right);
}

// Exact overloads so jstring == "literal" doesn't hit std::operator==(string, const char*)
inline bool operator==(const jstring &left, const char *right) { return static_cast<const std::string &>(left) == right; }
inline bool operator==(const char *left, const jstring &right) { return left == static_cast<const std::string &>(right); }

inline bool operator!=(const jstring &left, const jstring &right) { return !(left == right); }
inline bool operator!=(const jstring &left, const char *right)    { return !(left == right); }
inline bool operator!=(const char *left,    const jstring &right) { return !(left == right); }

inline bool operator<(const jstring &left, const jstring &right)
{
	return static_cast<const std::string &>(left) < static_cast<const std::string &>(right);
}

inline jstring operator+(const jstring &left, const jstring &right)
{
	std::string result(left);
	result.append(right);
	return result;
}

inline jstring operator+(const jstring &left, const char *right)
{
	return left + jstring(right);
}

inline jstring operator+(const char *left, const jstring &right)
{
	return jstring(left) + right;
}

inline jstring operator+(const jstring &left, char right)
{
	jstring result(left);
	result.push_back(right);
	return result;
}

inline jstring operator+(char left, const jstring &right)
{
	jstring result(1, left);
	result.append(right);
	return result;
}

inline jstring operator+(const jstring &left, const std::string &right) { return left + jstring(right); }
inline jstring operator+(const std::string &left, const jstring &right) { return jstring(left) + right; }
// rvalue overload: prevents ambiguity with std::operator+(string&&, const string&)
inline jstring operator+(std::string &&left, const jstring &right)      { return jstring(std::move(left)) + right; }

// std::hash specialization so jstring works in unordered_map/set
namespace std
{
template<>
struct hash<jstring>
{
    size_t operator()(const jstring &s) const noexcept
    {
        return hash<std::string>()(static_cast<const std::string &>(s));
    }
};
} // namespace std

namespace String
{

jstring toString(int_t v, int_t base = 10);
jstring toString(long_t v, int_t base = 10);
jstring toString(uint_t v, int_t base = 10);
jstring toString(ulong_t v, int_t base = 10);
jstring toString(float v);
jstring toString(double v);

inline jstring fromInt(int_t v) { return toString(v); }

// Java String semantics while jstring remains UTF-8 internally.
std::vector<char_t> toUtf16(const jstring &value);
jstring fromUtf16(const std::vector<char_t> &value);
int_t utf16Length(const jstring &value);
jstring substringUtf16(const jstring &value, int_t beginIndex, int_t endIndex);
jstring truncateUtf16(const jstring &value, int_t maxLength);
jstring removeLastUtf16Unit(const jstring &value);
void appendUtf16Unit(jstring &value, char_t unit);
int_t indexOfUtf16Unit(const jstring &value, char_t unit);
int_t hashCode(const jstring &value);
bool tryParseLong(const jstring &value, long_t &result);
bool tryParseInt(const jstring &value, int_t &result, int_t base = 10);
jstring trimJava(const jstring &value);
std::vector<jstring> splitJava(const jstring &value, char delimiter);
bool equalsIgnoreCaseJava(const jstring &left, const jstring &right);
jstring toLowerCaseJava(const jstring &value);

}
