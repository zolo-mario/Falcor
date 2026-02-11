# StringUtils - String Manipulation Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros.h (FALCOR_API)
- Falcor/Core/Error.h (FALCOR_THROW, FALCOR_ASSERT)
- std::algorithm (std::transform, std::min)
- std::codecvt (std::wstring_convert, std::codecvt_utf8)
- std::iomanip (std::setprecision, std::fixed)
- std::locale (std::locale)
- std::string (std::string)
- std::vector (std::vector)
- fmt/format.h (fmt::format)

### Dependent Modules

- Falcor/Utils (Internal utilities)
- Falcor/Core (Core utilities)
- Falcor/Scene (Scene utilities)
- Falcor/Rendering (Rendering utilities)
- Falcor/Plugins (Plugin utilities)

## Module Overview

StringUtils provides a comprehensive set of string manipulation utilities for the Falcor framework. These utilities include string prefix/suffix checking, string splitting/joining, whitespace removal, character replacement, substring replacement, URI decoding, array index parsing, buffer copying, byte size formatting, case conversion, Unicode conversion, and Base64 encoding/decoding.

### Key Characteristics

- **Comprehensive**: Covers common string manipulation tasks
- **Case-Sensitive Support**: Optional case-sensitive comparison
- **Unicode Support**: UTF-8 and UTF-32 conversion
- **Base64 Support**: Base64 encoding and decoding
- **URI Support**: URI decoding
- **Inline Functions**: Some functions are inline for performance
- **Header-Only**: Some functions are inline in header
- **Efficient**: Optimized for performance

### Architecture Principles

1. **Function-Based**: Function-based API
2. **Immutable**: Functions return new strings (don't modify input)
3. **Type-Safe**: Compile-time type checking
4. **Efficient**: Optimized for performance
5. **Flexible**: Supports various string operations

## Component Specifications

### hasPrefix() Function

**Files**: `StringUtils.h` (line 47), `StringUtils.cpp` (lines 37-55)

```cpp
FALCOR_API bool hasPrefix(const std::string& str, const std::string& prefix, bool caseSensitive = true);
```

**Purpose**: Check if a string starts with another string

**Parameters**:
- str: String to check in
- prefix: Prefix to check for
- caseSensitive: Whether comparison should be case-sensitive (default: true)

**Returns**: true if string starts with specified prefix

**Behavior**:
1. Check if str size >= prefix size
2. If not case-sensitive:
   - Convert str and prefix to lowercase
   - Compare lowercase strings
3. If case-sensitive:
   - Compare strings directly
4. Return result

**Characteristics**:
- Case-sensitive by default
- Optional case-insensitive comparison
- Returns false if prefix longer than string
- Efficient string comparison

### hasSuffix() Function

**Files**: `StringUtils.h` (line 56), `StringUtils.cpp` (lines 57-75)

```cpp
FALCOR_API bool hasSuffix(const std::string& str, const std::string& suffix, bool caseSensitive = true);
```

**Purpose**: Check if a string ends with another string

**Parameters**:
- str: String to check in
- suffix: Suffix to check for
- caseSensitive: Whether comparison should be case-sensitive (default: true)

**Returns**: true if string ends with specified suffix

**Behavior**:
1. Check if str size >= suffix size
2. Extract suffix from str
3. If not case-sensitive:
   - Convert extracted suffix and input suffix to lowercase
   - Compare lowercase strings
4. If case-sensitive:
   - Compare strings directly
5. Return result

**Characteristics**:
- Case-sensitive by default
- Optional case-insensitive comparison
- Returns false if suffix longer than string
- Efficient string comparison

### splitString() Function

**Files**: `StringUtils.h` (line 64), `StringUtils.cpp` (lines 77-101)

```cpp
FALCOR_API std::vector<std::string> splitString(const std::string& str, const std::string& delim);
```

**Purpose**: Split a string into a vector of strings based on a delimiter

**Parameters**:
- str: String to split
- delim: Delimiter to split strings by

**Returns**: Array of split strings excluding delimiters

**Behavior**:
1. Iterate through each character in string
2. If character is in delimiter:
   - If current token not empty, add to result
   - Clear current token
3. If character not in delimiter:
   - Add character to current token
4. After loop, if token not empty, add to result
5. Return result

**Characteristics**:
- Supports multi-character delimiters
- Excludes delimiters from result
- Handles consecutive delimiters
- Returns empty vector for empty string

### joinStrings() Function

**Files**: `StringUtils.h` (line 72), `StringUtils.cpp` (lines 103-116)

```cpp
FALCOR_API std::string joinStrings(const std::vector<std::string>& strings, const std::string& separator);
```

**Purpose**: Join an array of strings separated by another string

**Parameters**:
- strings: Array of strings to join
- separator: String placed between each string to be joined

**Returns**: Joined string

**Behavior**:
1. Iterate through strings
2. Add each string to result
3. If not last string, add separator
4. Return result

**Characteristics**:
- Supports multi-character separators
- No separator after last string
- Returns empty string for empty vector
- Efficient string concatenation

### removeLeadingWhitespace() Function

**Files**: `StringUtils.h` (line 80), `StringUtils.cpp` (lines 118-123)

```cpp
FALCOR_API std::string removeLeadingWhitespace(const std::string& str, const char* whitespace = " \n\r\t");
```

**Purpose**: Remove leading whitespace

**Parameters**:
- str: String to operate on
- whitespace: Whitespace characters (default: " \n\r\t")

**Returns**: String with leading whitespace removed

**Behavior**:
1. Copy input string
2. Find first non-whitespace character
3. Erase characters before first non-whitespace
4. Return result

**Characteristics**:
- Customizable whitespace characters
- Returns empty string if all whitespace
- Efficient string manipulation
- Immutable (doesn't modify input)

### removeTrailingWhitespace() Function

**Files**: `StringUtils.h` (line 88), `StringUtils.cpp` (lines 125-130)

```cpp
FALCOR_API std::string removeTrailingWhitespace(const std::string& str, const char* whitespace = " \n\r\t");
```

**Purpose**: Remove trailing whitespace

**Parameters**:
- str: String to operate on
- whitespace: Whitespace characters (default: " \n\r\t")

**Returns**: String with trailing whitespace removed

**Behavior**:
1. Copy input string
2. Find last non-whitespace character
3. Erase characters after last non-whitespace
4. Return result

**Characteristics**:
- Customizable whitespace characters
- Returns empty string if all whitespace
- Efficient string manipulation
- Immutable (doesn't modify input)

### removeLeadingTrailingWhitespace() Function

**Files**: `StringUtils.h` (line 96), `StringUtils.cpp` (lines 132-135)

```cpp
FALCOR_API std::string removeLeadingTrailingWhitespace(const std::string& str, const char* whitespace = " \n\r\t");
```

**Purpose**: Remove leading and trailing whitespace

**Parameters**:
- str: String to operate on
- whitespace: Whitespace characters (default: " \n\r\t")

**Returns**: String with leading and trailing whitespace removed

**Behavior**:
1. Remove leading whitespace
2. Remove trailing whitespace from result
3. Return result

**Characteristics**:
- Customizable whitespace characters
- Returns empty string if all whitespace
- Efficient string manipulation
- Immutable (doesn't modify input)

### replaceCharacters() Function

**Files**: `StringUtils.h` (line 106), `StringUtils.cpp` (lines 137-147)

```cpp
FALCOR_API std::string replaceCharacters(const std::string& str, const char* characters, const char replacement);
```

**Purpose**: Replace a set of characters

**Parameters**:
- str: String to operate on
- characters: Set of characters to replace
- replacement: Character to use as a replacement

**Returns**: String with characters replaced

**Behavior**:
1. Copy input string
2. Find first character to replace
3. While character found:
   - Replace character
   - Find next character to replace
4. Return result

**Characteristics**:
- Replaces all occurrences
- Supports multiple characters
- Efficient character replacement
- Immutable (doesn't modify input)

### padStringToLength() Function

**Files**: `StringUtils.h` (line 111), `StringUtils.cpp` (lines 149-155)

```cpp
FALCOR_API std::string padStringToLength(const std::string& str, size_t length, char padding = ' ');
```

**Purpose**: Pad string to minimum length

**Parameters**:
- str: String to operate on
- length: Minimum length
- padding: Padding character (default: ' ')

**Returns**: Padded string

**Behavior**:
1. Copy input string
2. If string length < target length:
   - Resize string to target length with padding character
3. Return result

**Characteristics**:
- Pads with specified character
- Only pads if string is shorter
- Efficient string manipulation
- Immutable (doesn't modify input)

### replaceSubstring() Function

**Files**: `StringUtils.h` (line 119), `StringUtils.cpp` (lines 157-168)

```cpp
FALCOR_API std::string replaceSubstring(const std::string& input, const std::string& src, const std::string& dst);
```

**Purpose**: Replace all occurrences of a substring in a string

**Parameters**:
- input: The input string
- src: The substring to replace
- dst: The substring to replace src with

**Returns**: String with all occurrences replaced

**Behavior**:
1. Copy input string
2. Find first occurrence of src
3. While occurrence found:
   - Replace src with dst
   - Move past replacement
   - Find next occurrence
4. Return result

**Characteristics**:
- Replaces all occurrences
- Supports empty src (no replacement)
- Supports empty dst (deletion)
- Efficient substring replacement
- Immutable (doesn't modify input)

### decodeURI() Function

**Files**: `StringUtils.h` (line 124), `StringUtils.cpp` (lines 170-195)

```cpp
FALCOR_API std::string decodeURI(const std::string& input);
```

**Purpose**: Decode a URI string

**Parameters**:
- input: The input URI string

**Returns**: Decoded URI string

**Behavior**:
1. Iterate through each character
2. If character is '%':
   - Extract next 2 characters as hex
   - Convert hex to character
   - Add to result
   - Skip 2 characters
3. If character is '+':
   - Add space to result
4. Otherwise:
   - Add character to result
5. Return result

**Characteristics**:
- Decodes percent-encoded characters
- Converts '+' to space
- Handles incomplete sequences gracefully
- Efficient URI decoding

### parseArrayIndex() Function

**Files**: `StringUtils.h` (line 133), `StringUtils.cpp` (lines 197-218)

```cpp
FALCOR_API bool parseArrayIndex(const std::string& name, std::string& nonArray, uint32_t& index);
```

**Purpose**: Parse a string in format <name>[<index>]

**Parameters**:
- name: String to parse
- nonArray: Output for non-array index portion of the string
- index: Output for index value parsed from the string

**Returns**: Whether string was successfully parsed

**Behavior**:
1. Find last '.' and '[' characters
2. If '[' found:
   - Check if '[' is after last '.' (ignore struct array index)
   - Extract index string between '[' and ']'
   - Parse index string to integer
   - Assert that next character is ']'
   - Extract non-array portion
   - Return true
3. Return false

**Characteristics**:
- Parses array index notation
- Ignores struct array indices
- Validates format with assertion
- Returns false if no array index

### copyStringToBuffer() Function

**Files**: `StringUtils.h` (line 138), `StringUtils.cpp` (lines 220-225)

```cpp
FALCOR_API void copyStringToBuffer(char* buffer, uint32_t bufferSize, const std::string& s);
```

**Purpose**: Copy text from a std::string to a char buffer, ensures null termination

**Parameters**:
- buffer: Destination buffer
- bufferSize: Size of destination buffer
- s: Source string

**Returns**: None

**Behavior**:
1. Calculate copy length (min of buffer size - 1 and string length)
2. Copy string to buffer
3. Null-terminate buffer

**Characteristics**:
- Ensures null termination
- Handles buffer overflow
- Efficient string copying
- Safe buffer operation

### formatByteSize() Function

**Files**: `StringUtils.h` (line 150), `StringUtils.cpp` (lines 227-239)

```cpp
FALCOR_API std::string formatByteSize(size_t size);
```

**Purpose**: Convert a size in bytes to a human readable string

**Parameters**:
- size: Size in bytes

**Returns**: Human readable string

**Behavior**:
1. If size < 1024: return "{size} B"
2. If size < 1048576: return "{size/1024.0:.2f} kB"
3. If size < 1073741824: return "{size/1048576.0:.2f} MB"
4. If size < 1099511627776: return "{size/1073741824.0:.2f} GB"
5. Otherwise: return "{size/1099511627776.0:.2f} TB"

**Characteristics**:
- Uses 1024-based units
- Formats with 2 decimal places
- Supports B, kB, MB, GB, TB units
- Efficient formatting

### toLowerCase() Function

**File**: `StringUtils.h` (lines 155-160)

```cpp
inline std::string toLowerCase(const std::string& str)
{
    std::string s = str;
    std::transform(str.begin(), str.end(), s.begin(), ::tolower);
    return s;
}
```

**Purpose**: Convert an ASCII string to lower case

**Parameters**:
- str: String to convert

**Returns**: Lowercase string

**Behavior**:
1. Copy input string
2. Transform each character to lowercase
3. Return result

**Characteristics**:
- Inline function for performance
- ASCII only (not Unicode)
- Immutable (doesn't modify input)
- Efficient character transformation

### string_2_wstring() Function

**File**: `StringUtils.h` (lines 165-170)

```cpp
inline std::wstring string_2_wstring(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    std::wstring ws = cvt.from_bytes(s);
    return ws;
}
```

**Purpose**: Convert an ASCII string to a UTF-8 wstring

**Parameters**:
- s: String to convert

**Returns**: UTF-8 wstring

**Behavior**:
1. Create UTF-8 converter
2. Convert string to wstring
3. Return result

**Characteristics**:
- Inline function for performance
- UTF-8 encoding
- Uses std::wstring_convert
- Efficient conversion

### wstring_2_string() Function

**File**: `StringUtils.h` (lines 175-180)

```cpp
inline std::string wstring_2_string(const std::wstring& ws)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    std::string s = cvt.to_bytes(ws);
    return s;
}
```

**Purpose**: Convert a UTF-8 wstring to an ASCII string

**Parameters**:
- ws: wstring to convert

**Returns**: ASCII string

**Behavior**:
1. Create UTF-8 converter
2. Convert wstring to string
3. Return result

**Characteristics**:
- Inline function for performance
- UTF-8 encoding
- Uses std::wstring_convert
- Efficient conversion

### utf32ToUtf8() Function

**File**: `StringUtils.h` (lines 185-189)

```cpp
inline std::string utf32ToUtf8(uint32_t codepoint)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    return cvt.to_bytes(codepoint);
}
```

**Purpose**: Convert a UTF-32 codepoint to a UTF-8 string

**Parameters**:
- codepoint: UTF-32 codepoint

**Returns**: UTF-8 string

**Behavior**:
1. Create UTF-32 to UTF-8 converter
2. Convert codepoint to UTF-8 string
3. Return result

**Characteristics**:
- Inline function for performance
- UTF-32 to UTF-8 conversion
- Uses std::wstring_convert
- Efficient conversion

### encodeBase64() Function

**Files**: `StringUtils.h` (lines 194, 199-202), `StringUtils.cpp` (lines 241-288)

```cpp
FALCOR_API std::string encodeBase64(const void* data, size_t len);
inline std::string encodeBase64(const std::vector<uint8_t>& in)
{
    return encodeBase64(in.data(), in.size());
}
```

**Purpose**: Encode data into base 64 encoding

**Parameters**:
- data: Pointer to data to encode
- len: Length of data to encode
- in: Vector of bytes to encode (inline overload)

**Returns**: Base64 encoded string

**Behavior**:
1. Calculate output length (4 * ((len + 2) / 3))
2. Create output string
3. For each 3 bytes:
   - Encode 3 bytes into 4 characters
   - Use encoding table
4. Handle remaining 1 or 2 bytes
   - Pad with '=' as needed
5. Return result

**Characteristics**:
- Standard Base64 encoding
- Handles arbitrary data
- Pads with '=' as needed
- Efficient encoding

### decodeBase64() Function

**Files**: `StringUtils.h` (line 207), `StringUtils.cpp` (lines 290-346)

```cpp
FALCOR_API std::vector<uint8_t> decodeBase64(const std::string& in);
```

**Purpose**: Decode data from base 64 encoding

**Parameters**:
- in: Base64 encoded string

**Returns**: Decoded data as vector of bytes

**Behavior**:
1. Validate input length (must be multiple of 4)
2. Calculate output length
3. Adjust for padding characters
4. Create output vector
5. For each 4 characters:
   - Decode 4 characters into 3 bytes
   - Use decoding table
   - Handle padding characters
6. Return result

**Characteristics**:
- Standard Base64 decoding
- Handles padding characters
- Validates input format
- Efficient decoding

## Technical Details

### Case-Insensitive Comparison

Case-insensitive string comparison:

```cpp
bool hasPrefix(const std::string& str, const std::string& prefix, bool caseSensitive)
{
    if (str.size() >= prefix.size())
    {
        if (caseSensitive == false)
        {
            std::string s = str;
            std::string pfx = prefix;
            std::transform(str.begin(), str.end(), s.begin(), ::tolower);
            std::transform(prefix.begin(), prefix.end(), pfx.begin(), ::tolower);
            return s.compare(0, pfx.length(), pfx) == 0;
        }
        else
        {
            return str.compare(0, prefix.length(), prefix) == 0;
        }
    }
    return false;
}
```

**Characteristics**:
- Converts both strings to lowercase
- Compares lowercase strings
- ASCII only (not Unicode)
- Efficient transformation

### String Splitting Algorithm

Splits string by delimiter:

```cpp
std::vector<std::string> splitString(const std::string& str, const std::string& delim)
{
    std::string s;
    std::vector<std::string> vec;
    for (char c : str)
    {
        if (delim.find(c) != std::string::npos)
        {
            if (s.length())
            {
                vec.push_back(s);
                s.clear();
            }
        }
        else
        {
            s += c;
        }
    }
    if (s.length())
    {
        vec.push_back(s);
    }
    return vec;
}
```

**Algorithm**:
1. Iterate through each character
2. If character is delimiter:
   - If current token not empty, add to result
   - Clear current token
3. If character not delimiter:
   - Add character to current token
4. After loop, if token not empty, add to result
5. Return result

**Characteristics**:
- Supports multi-character delimiters
- Excludes delimiters from result
- Handles consecutive delimiters
- Returns empty vector for empty string

### Whitespace Removal Algorithm

Removes leading/trailing whitespace:

```cpp
std::string removeLeadingWhitespace(const std::string& str, const char* whitespace)
{
    std::string result(str);
    result.erase(0, result.find_first_not_of(whitespace));
    return result;
}

std::string removeTrailingWhitespace(const std::string& str, const char* whitespace)
{
    std::string result(str);
    result.erase(result.find_last_not_of(whitespace) + 1);
    return result;
}
```

**Algorithm**:
1. Copy input string
2. Find first/last non-whitespace character
3. Erase characters before/after non-whitespace
4. Return result

**Characteristics**:
- Customizable whitespace characters
- Returns empty string if all whitespace
- Efficient string manipulation
- Immutable (doesn't modify input)

### Substring Replacement Algorithm

Replaces all occurrences of substring:

```cpp
std::string replaceSubstring(const std::string& input, const std::string& src, const std::string& dst)
{
    std::string res = input;
    size_t offset = res.find(src);
    while (offset != std::string::npos)
    {
        res.replace(offset, src.length(), dst);
        offset += dst.length();
        offset = res.find(src, offset);
    }
    return res;
}
```

**Algorithm**:
1. Copy input string
2. Find first occurrence of src
3. While occurrence found:
   - Replace src with dst
   - Move past replacement
   - Find next occurrence
4. Return result

**Characteristics**:
- Replaces all occurrences
- Supports empty src (no replacement)
- Supports empty dst (deletion)
- Efficient substring replacement

### URI Decoding Algorithm

Decodes percent-encoded URI:

```cpp
std::string decodeURI(const std::string& input)
{
    std::string result;
    for (size_t i = 0; i < input.length(); i++)
    {
        if (input[i] == '%')
        {
            if (i + 2 < input.length())
            {
                std::string hex = input.substr(i + 1, 2);
                char c = static_cast<char>(strtol(hex.c_str(), nullptr, 16));
                result += c;
                i += 2;
            }
        }
        else if (input[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += input[i];
        }
    }
    return result;
}
```

**Algorithm**:
1. Iterate through each character
2. If character is '%':
   - Extract next 2 characters as hex
   - Convert hex to character
   - Add to result
   - Skip 2 characters
3. If character is '+':
   - Add space to result
4. Otherwise:
   - Add character to result
5. Return result

**Characteristics**:
- Decodes percent-encoded characters
- Converts '+' to space
- Handles incomplete sequences gracefully
- Efficient URI decoding

### Array Index Parsing Algorithm

Parses array index notation:

```cpp
bool parseArrayIndex(const std::string& name, std::string& nonArray, uint32_t& index)
{
    size_t dot = name.find_last_of('.');
    size_t bracket = name.find_last_of('[');

    if (bracket != std::string::npos)
    {
        // Ignore cases where last index is an array of struct index (SomeStruct[1].v should be ignored)
        if ((dot == std::string::npos) || (bracket > dot))
        {
            // We know we have an array index. Make sure it's in range
            std::string indexStr = name.substr(bracket + 1);
            char* pEndPtr;
            index = strtol(indexStr.c_str(), &pEndPtr, 0);
            FALCOR_ASSERT(*pEndPtr == ']');
            nonArray = name.substr(0, bracket);
            return true;
        }
    }

    return false;
}
```

**Algorithm**:
1. Find last '.' and '[' characters
2. If '[' found:
   - Check if '[' is after last '.' (ignore struct array index)
   - Extract index string between '[' and ']'
   - Parse index string to integer
   - Assert that next character is ']'
   - Extract non-array portion
   - Return true
3. Return false

**Characteristics**:
- Parses array index notation
- Ignores struct array indices
- Validates format with assertion
- Returns false if no array index

### Base64 Encoding Algorithm

Encodes data to Base64:

```cpp
std::string encodeBase64(const void* data, size_t len)
{
    static constexpr char kEncodingTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
    };

    size_t outLen = 4 * ((len + 2) / 3);
    std::string out(outLen, '\0');

    const uint8_t* pIn = reinterpret_cast<const uint8_t*>(data);
    auto pOut = out.data();

    size_t i;
    for (i = 0; i + 2 < len; i += 3)
    {
        *pOut++ = kEncodingTable[(pIn[i] >> 2) & 0x3f];
        *pOut++ = kEncodingTable[((pIn[i] & 0x3) << 4) | ((pIn[i + 1] & 0xf0) >> 4)];
        *pOut++ = kEncodingTable[((pIn[i + 1] & 0xf) << 2) | ((pIn[i + 2] & 0xc0) >> 6)];
        *pOut++ = kEncodingTable[pIn[i + 2] & 0x3f];
    }
    if (i < len)
    {
        *pOut++ = kEncodingTable[(pIn[i] >> 2) & 0x3f];
        if (i == (len - 1))
        {
            *pOut++ = kEncodingTable[((pIn[i] & 0x3) << 4)];
            *pOut++ = '=';
        }
        else
        {
            *pOut++ = kEncodingTable[((pIn[i] & 0x3) << 4) | ((pIn[i + 1] & 0xf0) >> 4)];
            *pOut++ = kEncodingTable[((pIn[i + 1] & 0xf) << 2)];
        }
        *pOut++ = '=';
    }

    return out;
}
```

**Algorithm**:
1. Calculate output length (4 * ((len + 2) / 3))
2. Create output string
3. For each 3 bytes:
   - Encode 3 bytes into 4 characters
   - Use encoding table
4. Handle remaining 1 or 2 bytes
   - Pad with '=' as needed
5. Return result

**Characteristics**:
- Standard Base64 encoding
- Handles arbitrary data
- Pads with '=' as needed
- Efficient encoding

### Base64 Decoding Algorithm

Decodes Base64 to data:

```cpp
std::vector<uint8_t> decodeBase64(const std::string& in)
{
    static constexpr uint8_t kDecodingTable[] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    size_t inLen = in.size();
    if (inLen == 0)
        return {};
    if (inLen % 4 != 0)
        FALCOR_THROW("Input data size is not a multiple of 4");

    size_t outLen = inLen / 4 * 3;
    if (in[inLen - 1] == '=')
        outLen--;
    if (in[inLen - 2] == '=')
        outLen--;

    std::vector<uint8_t> out(outLen, 0);

    for (size_t i = 0, j = 0; i < inLen;)
    {
        uint32_t a = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t b = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t c = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t d = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];

        uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        if (j < outLen)
            out[j++] = (triple >> 2 * 8) & 0xff;
        if (j < outLen)
            out[j++] = (triple >> 1 * 8) & 0xff;
        if (j < outLen)
            out[j++] = (triple >> 0 * 8) & 0xff;
    }

    return out;
}
```

**Algorithm**:
1. Validate input length (must be multiple of 4)
2. Calculate output length
3. Adjust for padding characters
4. Create output vector
5. For each 4 characters:
   - Decode 4 characters into 3 bytes
   - Use decoding table
   - Handle padding characters
6. Return result

**Characteristics**:
- Standard Base64 decoding
- Handles padding characters
- Validates input format
- Efficient decoding

## Integration Points

### Falcor Core Integration

- **Falcor/Core/Macros.h**: FALCOR_API
- **Falcor/Core/Error.h**: FALCOR_THROW, FALCOR_ASSERT

### STL Integration

- **std::algorithm**: std::transform, std::min
- **std::codecvt**: std::wstring_convert, std::codecvt_utf8
- **std::iomanip**: std::setprecision, std::fixed
- **std::locale**: std::locale
- **std::string**: std::string
- **std::vector**: std::vector

### fmt Integration

- **fmt/format.h**: fmt::format

### Internal Falcor Usage

- **Falcor/Utils**: Internal utilities
- **Falcor/Core**: Core utilities
- **Falcor/Scene**: Scene utilities
- **Falcor/Rendering**: Rendering utilities
- **Falcor/Plugins**: Plugin utilities

## Architecture Patterns

### Function-Based Pattern

Function-based API:

```cpp
FALCOR_API bool hasPrefix(const std::string& str, const std::string& prefix, bool caseSensitive = true);
FALCOR_API bool hasSuffix(const std::string& str, const std::string& suffix, bool caseSensitive = true);
FALCOR_API std::vector<std::string> splitString(const std::string& str, const std::string& delim);
```

Benefits:
- Simple API
- Easy to use
- No state management
- Thread-safe

### Immutable Pattern

Functions return new strings:

```cpp
FALCOR_API std::string removeLeadingWhitespace(const std::string& str, const char* whitespace = " \n\r\t");
FALCOR_API std::string replaceSubstring(const std::string& input, const std::string& src, const std::string& dst);
```

Benefits:
- No side effects
- Predictable behavior
- Thread-safe
- Easy to reason about

### Inline Function Pattern

Some functions are inline for performance:

```cpp
inline std::string toLowerCase(const std::string& str)
{
    std::string s = str;
    std::transform(str.begin(), str.end(), s.begin(), ::tolower);
    return s;
}
```

Benefits:
- Zero function call overhead
- Better performance
- Compiler optimizations
- Small functions only

### Overload Pattern

Multiple overloads for convenience:

```cpp
FALCOR_API std::string encodeBase64(const void* data, size_t len);
inline std::string encodeBase64(const std::vector<uint8_t>& in)
{
    return encodeBase64(in.data(), in.size());
}
```

Benefits:
- Flexible API
- Convenience overloads
- Type safety
- Easy to use

## Code Patterns

### String Comparison Pattern

Compare strings with optional case sensitivity:

```cpp
bool hasPrefix(const std::string& str, const std::string& prefix, bool caseSensitive)
{
    if (str.size() >= prefix.size())
    {
        if (caseSensitive == false)
        {
            std::string s = str;
            std::string pfx = prefix;
            std::transform(str.begin(), str.end(), s.begin(), ::tolower);
            std::transform(prefix.begin(), prefix.end(), pfx.begin(), ::tolower);
            return s.compare(0, pfx.length(), pfx) == 0;
        }
        else
        {
            return str.compare(0, prefix.length(), prefix) == 0;
        }
    }
    return false;
}
```

Pattern:
- Check size constraint
- Transform to lowercase if needed
- Compare strings
- Return result

### String Iteration Pattern

Iterate through string characters:

```cpp
std::vector<std::string> splitString(const std::string& str, const std::string& delim)
{
    std::string s;
    std::vector<std::string> vec;
    for (char c : str)
    {
        if (delim.find(c) != std::string::npos)
        {
            if (s.length())
            {
                vec.push_back(s);
                s.clear();
            }
        }
        else
        {
            s += c;
        }
    }
    if (s.length())
    {
        vec.push_back(s);
    }
    return vec;
}
```

Pattern:
- Iterate through characters
- Build token
- Add token to result when delimiter found
- Add remaining token after loop

### String Modification Pattern

Modify string copy:

```cpp
std::string removeLeadingWhitespace(const std::string& str, const char* whitespace)
{
    std::string result(str);
    result.erase(0, result.find_first_not_of(whitespace));
    return result;
}
```

Pattern:
- Copy input string
- Find position to erase
- Erase characters
- Return result

### Substring Replacement Pattern

Replace all occurrences:

```cpp
std::string replaceSubstring(const std::string& input, const std::string& src, const std::string& dst)
{
    std::string res = input;
    size_t offset = res.find(src);
    while (offset != std::string::npos)
    {
        res.replace(offset, src.length(), dst);
        offset += dst.length();
        offset = res.find(src, offset);
    }
    return res;
}
```

Pattern:
- Copy input string
- Find first occurrence
- While occurrence found:
  - Replace substring
  - Move past replacement
  - Find next occurrence
- Return result

### Encoding Pattern

Encode data using table:

```cpp
std::string encodeBase64(const void* data, size_t len)
{
    static constexpr char kEncodingTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        // ...
    };

    size_t outLen = 4 * ((len + 2) / 3);
    std::string out(outLen, '\0');

    const uint8_t* pIn = reinterpret_cast<const uint8_t*>(data);
    auto pOut = out.data();

    size_t i;
    for (i = 0; i + 2 < len; i += 3)
    {
        *pOut++ = kEncodingTable[(pIn[i] >> 2) & 0x3f];
        *pOut++ = kEncodingTable[((pIn[i] & 0x3) << 4) | ((pIn[i + 1] & 0xf0) >> 4)];
        *pOut++ = kEncodingTable[((pIn[i + 1] & 0xf) << 2) | ((pIn[i + 2] & 0xc0) >> 6)];
        *pOut++ = kEncodingTable[pIn[i + 2] & 0x3f];
    }
    // Handle remaining bytes
    return out;
}
```

Pattern:
- Define encoding table
- Calculate output length
- Iterate through input
- Encode using table
- Handle remaining bytes
- Return result

### Decoding Pattern

Decode data using table:

```cpp
std::vector<uint8_t> decodeBase64(const std::string& in)
{
    static constexpr uint8_t kDecodingTable[] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        // ...
    };

    size_t inLen = in.size();
    if (inLen == 0)
        return {};
    if (inLen % 4 != 0)
        FALCOR_THROW("Input data size is not a multiple of 4");

    size_t outLen = inLen / 4 * 3;
    if (in[inLen - 1] == '=')
        outLen--;
    if (in[inLen - 2] == '=')
        outLen--;

    std::vector<uint8_t> out(outLen, 0);

    for (size_t i = 0, j = 0; i < inLen;)
    {
        uint32_t a = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t b = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t c = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t d = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];

        uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        if (j < outLen)
            out[j++] = (triple >> 2 * 8) & 0xff;
        if (j < outLen)
            out[j++] = (triple >> 1 * 8) & 0xff;
        if (j < outLen)
            out[j++] = (triple >> 0 * 8) & 0xff;
    }

    return out;
}
```

Pattern:
- Define decoding table
- Validate input
- Calculate output length
- Adjust for padding
- Iterate through input
- Decode using table
- Handle padding
- Return result

## Use Cases

### Basic String Operations

```cpp
#include "Utils/StringUtils.h"

// Check prefix
bool hasPrefix = hasPrefix("Hello World", "Hello");
// Returns: true

bool hasPrefix2 = hasPrefix("Hello World", "hello", false);
// Returns: true (case-insensitive)

// Check suffix
bool hasSuffix = hasSuffix("Hello World", "World");
// Returns: true

bool hasSuffix2 = hasSuffix("Hello World", "world", false);
// Returns: true (case-insensitive)
```

### String Splitting and Joining

```cpp
#include "Utils/StringUtils.h"

// Split string
std::vector<std::string> parts = splitString("a,b,c", ",");
// Returns: {"a", "b", "c"}

// Join strings
std::string joined = joinStrings({"a", "b", "c"}, ",");
// Returns: "a,b,c"
```

### Whitespace Removal

```cpp
#include "Utils/StringUtils.h"

// Remove leading whitespace
std::string trimmed = removeLeadingWhitespace("  Hello World  ");
// Returns: "Hello World  "

// Remove trailing whitespace
std::string trimmed2 = removeTrailingWhitespace("  Hello World  ");
// Returns: "  Hello World"

// Remove leading and trailing whitespace
std::string trimmed3 = removeLeadingTrailingWhitespace("  Hello World  ");
// Returns: "Hello World"
```

### Character Replacement

```cpp
#include "Utils/StringUtils.h"

// Replace characters
std::string replaced = replaceCharacters("some/path with/whitespace", "/ ", '_');
// Returns: "some_path_with_whitespace"
```

### String Padding

```cpp
#include "Utils/StringUtils.h"

// Pad string
std::string padded = padStringToLength("Hello", 10, ' ');
// Returns: "Hello     "

std::string padded2 = padStringToLength("Hello", 10, '0');
// Returns: "Hello00000"
```

### Substring Replacement

```cpp
#include "Utils/StringUtils.h"

// Replace substring
std::string replaced = replaceSubstring("Hello World", "World", "Universe");
// Returns: "Hello Universe"

std::string replaced2 = replaceSubstring("aaa", "a", "b");
// Returns: "bbb"
```

### URI Decoding

```cpp
#include "Utils/StringUtils.h"

// Decode URI
std::string decoded = decodeURI("Hello%20World");
// Returns: "Hello World"

std::string decoded2 = decodeURI("Hello+World");
// Returns: "Hello World"
```

### Array Index Parsing

```cpp
#include "Utils/StringUtils.h"

// Parse array index
std::string nonArray;
uint32_t index;
bool parsed = parseArrayIndex("array[5]", nonArray, index);
// Returns: true, nonArray = "array", index = 5

bool parsed2 = parseArrayIndex("struct.array[5]", nonArray, index);
// Returns: false (struct array index ignored)
```

### Buffer Copying

```cpp
#include "Utils/StringUtils.h"

// Copy string to buffer
char buffer[256];
copyStringToBuffer(buffer, 256, "Hello World");
// buffer = "Hello World\0"
```

### Byte Size Formatting

```cpp
#include "Utils/StringUtils.h"

// Format byte size
std::string formatted = formatByteSize(1024);
// Returns: "1.00 kB"

std::string formatted2 = formatByteSize(1048576);
// Returns: "1.00 MB"

std::string formatted3 = formatByteSize(1073741824);
// Returns: "1.00 GB"
```

### Case Conversion

```cpp
#include "Utils/StringUtils.h"

// Convert to lowercase
std::string lower = toLowerCase("HELLO WORLD");
// Returns: "hello world"
```

### Unicode Conversion

```cpp
#include "Utils/StringUtils.h"

// Convert string to wstring
std::wstring ws = string_2_wstring("Hello");
// Returns: L"Hello"

// Convert wstring to string
std::string s = wstring_2_string(L"Hello");
// Returns: "Hello"

// Convert UTF-32 codepoint to UTF-8
std::string utf8 = utf32ToUtf8(0x1F600);
// Returns: "😀"
```

### Base64 Encoding

```cpp
#include "Utils/StringUtils.h"

// Encode data to Base64
std::vector<uint8_t> data = {0x48, 0x65, 0x6c, 0x6c, 0x6f};
std::string encoded = encodeBase64(data.data(), data.size());
// Returns: "SGVsbG8="

// Encode vector to Base64
std::string encoded2 = encodeBase64(data);
// Returns: "SGVsbG8="
```

### Base64 Decoding

```cpp
#include "Utils/StringUtils.h"

// Decode Base64 to data
std::vector<uint8_t> decoded = decodeBase64("SGVsbG8=");
// Returns: {0x48, 0x65, 0x6c, 0x6c, 0x6f}
```

## Performance Considerations

### Memory Overhead

- **String Copying**: Functions copy strings (immutable pattern)
- **Vector Allocation**: splitString() allocates vector
- **Base64 Encoding**: Allocates output string
- **Base64 Decoding**: Allocates output vector

### Algorithmic Complexity

- **hasPrefix/hasSuffix**: O(n) where n is prefix/suffix length
- **splitString**: O(n) where n is string length
- **joinStrings**: O(n) where n is total string length
- **removeLeadingWhitespace**: O(n) where n is string length
- **removeTrailingWhitespace**: O(n) where n is string length
- **replaceCharacters**: O(n*m) where n is string length, m is character count
- **replaceSubstring**: O(n*m) where n is string length, m is replacement count
- **decodeURI**: O(n) where n is string length
- **parseArrayIndex**: O(n) where n is string length
- **copyStringToBuffer**: O(n) where n is string length
- **formatByteSize**: O(1)
- **toLowerCase**: O(n) where n is string length
- **string_2_wstring**: O(n) where n is string length
- **wstring_2_string**: O(n) where n is wstring length
- **utf32ToUtf8**: O(1)
- **encodeBase64**: O(n) where n is data length
- **decodeBase64**: O(n) where n is string length

### Compiler Optimizations

- **Inline Expansion**: Inline functions can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead

### Comparison with Alternatives

**StringUtils vs std::string**:
- StringUtils: More functions, higher-level API
- std::string: Fewer functions, lower-level API

**StringUtils vs Boost.String**:
- StringUtils: Simpler API, fewer dependencies
- Boost.String: More functions, more features

### Optimization Tips

1. **Use Inline Functions**: Use inline functions for performance
2. **Avoid Unnecessary Copies**: Avoid unnecessary string copies
3. **Use Efficient Algorithms**: Use efficient algorithms
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

## Limitations

### Feature Limitations

- **No Regular Expressions**: No regex support
- **No Pattern Matching**: No pattern matching
- **No Validation**: No string validation
- **No Localization**: No localization support
- **No Unicode Case Conversion**: toLowerCase is ASCII only

### API Limitations

- **No In-Place Operations**: All operations create new strings
- **No String Views**: No string view support
- **No String Builder**: No string builder pattern
- **No String Pooling**: No string pooling

### Type Limitations

- **ASCII Only**: toLowerCase is ASCII only
- **UTF-8 Only**: string_2_wstring and wstring_2_string are UTF-8 only
- **std::string Only**: All functions use std::string

### Performance Limitations

- **String Copying**: All operations create new strings
- **Vector Allocation**: splitString() allocates vector
- **Base64 Encoding**: Allocates output string
- **Base64 Decoding**: Allocates output vector

### Platform Limitations

- **C++11 Only**: Requires C++11 or later
- **std::wstring_convert**: Deprecated in C++17
- **Locale**: Depends on system locale

## Best Practices

### When to Use

**Appropriate Use Cases**:
- String prefix/suffix checking
- String splitting/joining
- Whitespace removal
- Character replacement
- Substring replacement
- URI decoding
- Array index parsing
- Buffer copying
- Byte size formatting
- Case conversion
- Unicode conversion
- Base64 encoding/decoding

**Inappropriate Use Cases**:
- Regular expressions (use regex library)
- Pattern matching (use regex library)
- String validation (use validation library)
- Localization (use localization library)
- Unicode case conversion (use Unicode library)

### Usage Patterns

**Basic String Operations**:
```cpp
bool hasPrefix = hasPrefix("Hello World", "Hello");
bool hasSuffix = hasSuffix("Hello World", "World");
```

**String Splitting and Joining**:
```cpp
std::vector<std::string> parts = splitString("a,b,c", ",");
std::string joined = joinStrings({"a", "b", "c"}, ",");
```

**Whitespace Removal**:
```cpp
std::string trimmed = removeLeadingTrailingWhitespace("  Hello World  ");
```

**Base64 Encoding/Decoding**:
```cpp
std::string encoded = encodeBase64(data);
std::vector<uint8_t> decoded = decodeBase64(encoded);
```

### Performance Tips

1. **Use Inline Functions**: Use inline functions for performance
2. **Avoid Unnecessary Copies**: Avoid unnecessary string copies
3. **Use Efficient Algorithms**: Use efficient algorithms
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

### Type Safety

- **Compile-Time Type Checking**: Compile-time type checking
- **Type-Safe Operations**: Type-safe operations
- **No Runtime Type Information**: No runtime type information

### Error Handling

- **Empty Strings**: Handle empty strings gracefully
- **Invalid Input**: Handle invalid input gracefully
- **Buffer Overflow**: Handle buffer overflow safely
- **Invalid Base64**: Validate Base64 input

## Implementation Notes

### String Comparison Implementation

Case-insensitive string comparison:

```cpp
bool hasPrefix(const std::string& str, const std::string& prefix, bool caseSensitive)
{
    if (str.size() >= prefix.size())
    {
        if (caseSensitive == false)
        {
            std::string s = str;
            std::string pfx = prefix;
            std::transform(str.begin(), str.end(), s.begin(), ::tolower);
            std::transform(prefix.begin(), prefix.end(), pfx.begin(), ::tolower);
            return s.compare(0, pfx.length(), pfx) == 0;
        }
        else
        {
            return str.compare(0, prefix.length(), prefix) == 0;
        }
    }
    return false;
}
```

**Characteristics**:
- Converts both strings to lowercase
- Compares lowercase strings
- ASCII only (not Unicode)
- Efficient transformation

### String Splitting Implementation

Splits string by delimiter:

```cpp
std::vector<std::string> splitString(const std::string& str, const std::string& delim)
{
    std::string s;
    std::vector<std::string> vec;
    for (char c : str)
    {
        if (delim.find(c) != std::string::npos)
        {
            if (s.length())
            {
                vec.push_back(s);
                s.clear();
            }
        }
        else
        {
            s += c;
        }
    }
    if (s.length())
    {
        vec.push_back(s);
    }
    return vec;
}
```

**Characteristics**:
- Supports multi-character delimiters
- Excludes delimiters from result
- Handles consecutive delimiters
- Returns empty vector for empty string

### Whitespace Removal Implementation

Removes leading/trailing whitespace:

```cpp
std::string removeLeadingWhitespace(const std::string& str, const char* whitespace)
{
    std::string result(str);
    result.erase(0, result.find_first_not_of(whitespace));
    return result;
}
```

**Characteristics**:
- Customizable whitespace characters
- Returns empty string if all whitespace
- Efficient string manipulation
- Immutable (doesn't modify input)

### Base64 Encoding Implementation

Encodes data to Base64:

```cpp
std::string encodeBase64(const void* data, size_t len)
{
    static constexpr char kEncodingTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        // ...
    };

    size_t outLen = 4 * ((len + 2) / 3);
    std::string out(outLen, '\0');

    const uint8_t* pIn = reinterpret_cast<const uint8_t*>(data);
    auto pOut = out.data();

    size_t i;
    for (i = 0; i + 2 < len; i += 3)
    {
        *pOut++ = kEncodingTable[(pIn[i] >> 2) & 0x3f];
        *pOut++ = kEncodingTable[((pIn[i] & 0x3) << 4) | ((pIn[i + 1] & 0xf0) >> 4)];
        *pOut++ = kEncodingTable[((pIn[i + 1] & 0xf) << 2) | ((pIn[i + 2] & 0xc0) >> 6)];
        *pOut++ = kEncodingTable[pIn[i + 2] & 0x3f];
    }
    // Handle remaining bytes
    return out;
}
```

**Characteristics**:
- Standard Base64 encoding
- Handles arbitrary data
- Pads with '=' as needed
- Efficient encoding

### Base64 Decoding Implementation

Decodes Base64 to data:

```cpp
std::vector<uint8_t> decodeBase64(const std::string& in)
{
    static constexpr uint8_t kDecodingTable[] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        // ...
    };

    size_t inLen = in.size();
    if (inLen == 0)
        return {};
    if (inLen % 4 != 0)
        FALCOR_THROW("Input data size is not a multiple of 4");

    size_t outLen = inLen / 4 * 3;
    if (in[inLen - 1] == '=')
        outLen--;
    if (in[inLen - 2] == '=')
        outLen--;

    std::vector<uint8_t> out(outLen, 0);

    for (size_t i = 0, j = 0; i < inLen;)
    {
        uint32_t a = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t b = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t c = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];
        uint32_t d = in[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<uint32_t>(in[i++])];

        uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        if (j < outLen)
            out[j++] = (triple >> 2 * 8) & 0xff;
        if (j < outLen)
            out[j++] = (triple >> 1 * 8) & 0xff;
        if (j < outLen)
            out[j++] = (triple >> 0 * 8) & 0xff;
    }

    return out;
}
```

**Characteristics**:
- Standard Base64 decoding
- Handles padding characters
- Validates input format
- Efficient decoding

## Future Enhancements

### Potential Improvements

1. **Regular Expressions**: Add regex support
2. **Pattern Matching**: Add pattern matching
3. **String Validation**: Add string validation
4. **Localization**: Add localization support
5. **Unicode Case Conversion**: Add Unicode case conversion
6. **String Views**: Add string view support
7. **String Builder**: Add string builder pattern
8. **String Pooling**: Add string pooling
9. **More Encodings**: Add more encodings (UTF-16, etc.)
10. **More Algorithms**: Add more string algorithms

### API Extensions

1. **In-Place Operations**: Add in-place operations
2. **String Views**: Add string view support
3. **String Builder**: Add string builder pattern
4. **String Pooling**: Add string pooling
5. **More Encodings**: Add more encodings
6. **More Algorithms**: Add more string algorithms
7. **Validation Functions**: Add validation functions
8. **Pattern Matching**: Add pattern matching
9. **Regular Expressions**: Add regex support
10. **Localization**: Add localization support

### Performance Enhancements

1. **String Views**: Use string views to avoid copies
2. **String Builder**: Add string builder pattern
3. **String Pooling**: Add string pooling
4. **SIMD Optimizations**: Add SIMD optimizations
5. **Compiler Optimizations**: Better compiler optimizations

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from std::string
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::string**: C++ string library
- **std::vector**: C++ vector container
- **std::algorithm**: C++ algorithms
- **std::codecvt**: C++ codecvt library
- **std::locale**: C++ locale library

### fmt Library

- **fmt::format**: fmt format function

### Falcor Documentation

- **Falcor Utils**: Utils module documentation
- **Falcor Core**: Core module documentation

### Related Technologies

- **String Manipulation**: String manipulation techniques
- **Base64 Encoding**: Base64 encoding techniques
- **URI Encoding**: URI encoding techniques
- **Unicode**: Unicode encoding techniques
- **String Algorithms**: String algorithms
