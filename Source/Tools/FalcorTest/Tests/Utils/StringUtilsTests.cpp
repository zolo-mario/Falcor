#include "Testing/UnitTest.h"
#include "Utils/StringUtils.h"
#include <random>

namespace Falcor
{
CPU_TEST(Base64)
{
    auto testEncodeDecode = [&](std::string decoded, std::string encoded)
    {
        EXPECT(encodeBase64(std::vector<uint8_t>(decoded.begin(), decoded.end())) == encoded);
        EXPECT(decodeBase64(encoded) == std::vector<uint8_t>(decoded.begin(), decoded.end()));
    };

    testEncodeDecode("", "");
    testEncodeDecode("a", "YQ==");
    testEncodeDecode("ab", "YWI=");
    testEncodeDecode("abc", "YWJj");
    testEncodeDecode("Hello World!", "SGVsbG8gV29ybGQh");
    testEncodeDecode(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.",
        "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdCwgc2VkIGRvIGVpdXNtb2QgdGVtcG9yIGluY2lkaWR1bnQgdXQgbGFib3"
        "JlIGV0IGRvbG9yZSBtYWduYSBhbGlxdWEu"
    );
}

CPU_TEST(RemoveWhitespace)
{
    const char* whitespace = " \t\n\r";
    EXPECT_EQ(removeLeadingWhitespace("  \t\t\n\n\r\rtest", whitespace), "test");
    EXPECT_EQ(removeLeadingWhitespace("test", whitespace), "test");
    EXPECT_EQ(removeLeadingWhitespace("test  \t\t\n\n\r\r", whitespace), "test  \t\t\n\n\r\r");

    EXPECT_EQ(removeTrailingWhitespace("  \t\t\n\n\r\rtest", whitespace), "  \t\t\n\n\r\rtest");
    EXPECT_EQ(removeTrailingWhitespace("test", whitespace), "test");
    EXPECT_EQ(removeTrailingWhitespace("test  \t\t\n\n\r\r", whitespace), "test");

    EXPECT_EQ(removeLeadingTrailingWhitespace("  \t\t\n\n\r\rtest", whitespace), "test");
    EXPECT_EQ(removeLeadingTrailingWhitespace("test", whitespace), "test");
    EXPECT_EQ(removeLeadingTrailingWhitespace("test  \t\t\n\n\r\r", whitespace), "test");
}

CPU_TEST(ReplaceCharacters)
{
    EXPECT_EQ(replaceCharacters("test", "", ' '), "test");
    EXPECT_EQ(replaceCharacters("test", "x", ' '), "test");
    EXPECT_EQ(replaceCharacters("test", "t", ' '), " es ");
    EXPECT_EQ(replaceCharacters("test", "te", ' '), "  s ");
    EXPECT_EQ(replaceCharacters("test", "tes", ' '), "    ");
    EXPECT_EQ(replaceCharacters("1122334455", "24", '_'), "11__33__55");
    EXPECT_EQ(replaceCharacters("some/path with/whitespace", " /", '_'), "some_path_with_whitespace");
}

CPU_TEST(FormatByteSize)
{
    const size_t kB = 1024ull;
    const size_t MB = 1048576ull;
    const size_t GB = 1073741824ull;
    const size_t TB = 1099511627776ull;

    EXPECT_EQ(formatByteSize(0), "0 B");
    EXPECT_EQ(formatByteSize(100), "100 B");
    EXPECT_EQ(formatByteSize(1023), "1023 B");
    EXPECT_EQ(formatByteSize(kB), "1.00 kB");
    EXPECT_EQ(formatByteSize(100 * kB), "100.00 kB");
    EXPECT_EQ(formatByteSize(1023 * kB), "1023.00 kB");
    EXPECT_EQ(formatByteSize(MB), "1.00 MB");
    EXPECT_EQ(formatByteSize(10 * MB), "10.00 MB");
    EXPECT_EQ(formatByteSize(1023 * MB), "1023.00 MB");
    EXPECT_EQ(formatByteSize(GB), "1.00 GB");
    EXPECT_EQ(formatByteSize(10 * GB), "10.00 GB");
    EXPECT_EQ(formatByteSize(1023 * GB), "1023.00 GB");
    EXPECT_EQ(formatByteSize(TB), "1.00 TB");
    EXPECT_EQ(formatByteSize(10 * TB), "10.00 TB");
}

CPU_TEST(DecodeURI)
{
    EXPECT_EQ(decodeURI("test"), "test");
    EXPECT_EQ(decodeURI("hello%20world"), "hello world");
    EXPECT_EQ(decodeURI("hello%20world%21"), "hello world!");
    EXPECT_EQ(decodeURI("%22hello+world%22"), "\"hello world\"");
}

} // namespace Falcor
