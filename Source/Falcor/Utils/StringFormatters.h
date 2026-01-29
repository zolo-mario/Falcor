#pragma once
#include <fmt/format.h>
#include <filesystem>
#include <optional>
#include <string>

template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string>
{
    template<typename FormatContext>
    auto format(const std::filesystem::path& p, FormatContext& ctx)
    {
        return formatter<std::string>::format(p.string(), ctx);
    }
};

template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T>
{
    template<typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx)
    {
        if (opt)
        {
            formatter<T>::format(*opt, ctx);
            return ctx.out();
        }
        return fmt::format_to(ctx.out(), "nullopt");
    }
};
