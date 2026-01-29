#pragma once
#include "Core/Macros.h"
#include "Core/Error.h"
#include "Core/Platform/OS.h"
#include <filesystem>
#include <string>

/// ImporterError split from the Importer.h file to allow using it without bringing in pybind11 types (via SceneBuilder)

namespace Falcor
{
    /** Exception thrown during scene import.
        Holds the path of the imported asset and a description of the exception.
    */
    class FALCOR_API ImporterError : public Exception
    {
    public:
        ImporterError() noexcept
        {}

        ImporterError(const std::filesystem::path& path, std::string_view what)
            : Exception(what)
            , mpPath(std::make_shared<std::filesystem::path>(path))
        {}

        template<typename... Args>
        explicit ImporterError(const std::filesystem::path& path, fmt::format_string<Args...> format, Args&&... args)
            : ImporterError(path, fmt::format(format, std::forward<Args>(args)...))
        {}

        virtual ~ImporterError() override
        {}

        ImporterError(const ImporterError& other) noexcept
        {
            mpWhat = other.mpWhat;
            mpPath = other.mpPath;
        }

        const std::filesystem::path& path() const noexcept { return *mpPath; }

    private:
        std::shared_ptr<std::filesystem::path> mpPath;
    };
}
