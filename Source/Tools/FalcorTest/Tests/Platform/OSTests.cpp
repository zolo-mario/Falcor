#include "Testing/UnitTest.h"
#include "Core/Platform/OS.h"

namespace Falcor
{
CPU_TEST(Junction)
{
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path target = cwd / "junction_target";
    std::filesystem::path link = cwd / "junction_link";

    // Create junction_target/test
    std::filesystem::create_directories(target / "test");

    // Create junction from junction_link to junction_target
    EXPECT_EQ(createJunction(link, target), true);
    // Check that junction was successfully created by accessing junction_link/test
    EXPECT_EQ(std::filesystem::exists(link / "test"), true);
    // Delete junction
    EXPECT_EQ(deleteJunction(link), true);
    // Check that junction was deleted
    EXPECT_EQ(std::filesystem::exists(link), false);

    // Delete junction_target/test
    std::filesystem::remove_all(target);
}

CPU_TEST(HasExtension)
{
    EXPECT_EQ(hasExtension("foo.exr", "exr"), true);
    EXPECT_EQ(hasExtension("foo.exr", ".exr"), true);
    EXPECT_EQ(hasExtension("foo.Exr", "exr"), true);
    EXPECT_EQ(hasExtension("foo.Exr", ".exr"), true);
    EXPECT_EQ(hasExtension("foo.Exr", "exR"), true);
    EXPECT_EQ(hasExtension("foo.Exr", ".exR"), true);
    EXPECT_EQ(hasExtension("foo.EXR", "exr"), true);
    EXPECT_EQ(hasExtension("foo.EXR", ".exr"), true);
    EXPECT_EQ(hasExtension("foo.xr", "exr"), false);
    EXPECT_EQ(hasExtension("/foo/png", ""), true);
    EXPECT_EQ(hasExtension("/foo/png", "exr"), false);
    EXPECT_EQ(hasExtension("/foo/.profile", ""), true);
}

CPU_TEST(GetExtensionFromPath)
{
    EXPECT_EQ(getExtensionFromPath("foo.exr"), "exr");
    EXPECT_EQ(getExtensionFromPath("foo.Exr"), "exr");
    EXPECT_EQ(getExtensionFromPath("foo.EXR"), "exr");
    EXPECT_EQ(getExtensionFromPath("foo"), "");
    EXPECT_EQ(getExtensionFromPath("/foo/.profile"), "");
}

CPU_TEST(OS_HomeDirectory)
{
    std::filesystem::path homeDirectory = getHomeDirectory();
    EXPECT(!homeDirectory.empty());
}

CPU_TEST(GetEnvironmentVariable)
{
    EXPECT_EQ(getEnvironmentVariable("__ENV_VARIABLE_THAT_MOST_PROBABLY_DOES_NOT_EXIST__"), std::optional<std::string>{});
#if FALCOR_WINDOWS
    EXPECT_NE(getEnvironmentVariable("Path"), std::optional<std::string>{});
#elif FALCOR_LINUX
    EXPECT_NE(getEnvironmentVariable("PATH"), std::optional<std::string>{});
#endif
}
} // namespace Falcor
