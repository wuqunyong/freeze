#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <concepts>



namespace apie
{

extern const char kWhitespaceASCII[];

enum WhitespaceHandling
{
    KEEP_WHITESPACE,
    TRIM_WHITESPACE,
};

enum SplitResult
{
    SPLIT_WANT_ALL,
    SPLIT_WANT_NONEMPTY,
};

std::vector<std::string> SplitString(const std::string &input,
                                     const std::string &delimiters,
                                     WhitespaceHandling whitespace,
                                     SplitResult resultType);

void SplitStringAlongWhitespace(const std::string &input,
                                std::vector<std::string> *tokensOut);

std::string TrimString(const std::string &input, const std::string &trimChars);

std::string StringJoin(std::vector<std::string> &data, const std::string &delimiters);

bool HexStringToUInt(const std::string &input, unsigned int *uintOut);

bool ReadFileToString(const std::string &path, std::string *stringOut);


// Check if the string str begins with the given prefix.
// The comparison is case sensitive.
bool BeginsWith(const std::string &str, const std::string &prefix);

// Check if the string str begins with the given prefix.
// Prefix may not be NULL and needs to be NULL terminated.
// The comparison is case sensitive.
bool BeginsWith(const std::string &str, const char *prefix);

// Check if the string str begins with the given prefix.
// str and prefix may not be NULL and need to be NULL terminated.
// The comparison is case sensitive.
bool BeginsWith(const char *str, const char *prefix);

// Check if the string str begins with the first prefixLength characters of the given prefix.
// The length of the prefix string should be greater than or equal to prefixLength.
// The comparison is case sensitive.
bool BeginsWith(const std::string &str, const std::string &prefix, const size_t prefixLength);

// Check if the string str ends with the given suffix.
// Suffix may not be NUL and needs to be NULL terminated.
// The comparison is case sensitive.
bool EndsWith(const std::string& str, const char* suffix);

// Convert to lower-case.
void ToLower(std::string *str);

// Replaces the substring 'substring' in 'str' with 'replacement'. Returns true if successful.
bool ReplaceSubstring(std::string *str,
                      const std::string &substring,
                      const std::string &replacement);

std::string& ReplaceStrAll(std::string& str, const std::string& old_value, const std::string& new_value);

char* URLDecode(const char *in);

std::string randomStr(int32_t iCount);


template <typename S, typename T>
struct is_streamable {
	template <typename SS, typename TT>
	static auto test(int)
		-> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

	template <typename, typename>
	static auto test(...)->std::false_type;

	static const bool value = decltype(test<S, T>(0))::value;
};

template<typename Key, bool Streamable>
struct streamable_to_string {
	static std::string impl(const Key& key) {
		std::stringstream ss;
		ss << key;
		return ss.str();
	}
};

template<typename Key>
struct streamable_to_string<Key, false> {
	static std::string impl(const Key&) {
		return "undefined";
	}
};

template<typename Key>
std::string key_to_string(const Key& key) {
	return streamable_to_string<Key, is_streamable<std::stringstream, Key>::value>().impl(key);
}

template <typename T>
constexpr bool always_false = std::false_type::value;

template <typename T>
std::string as_string(T a)
{
	constexpr bool has_to_string = requires(T x)
	{
		{ std::to_string(x) } -> std::convertible_to<std::string>;
	};
	
	constexpr bool has_stream = requires(T x, std::ostream & os)
	{
		{os << x} -> std::same_as<std::ostream&>;
	};


	if constexpr (has_to_string)
	{
		return std::to_string(a);
	}
	else if constexpr (has_stream)
	{
		std::stringstream s;
		s << a;
		return s.str();
	}
	else
	{
		static_assert(always_false<T>, "The type cannot be serialized");
	}
}


std::string DecodeBase64(const std::string& base64_str);

std::string EncodeBase64(const std::string& in);


}  // namespace APie
