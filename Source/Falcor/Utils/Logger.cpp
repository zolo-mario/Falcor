#include "Logger.h"
#include "Core/Error.h"
#include "Core/Platform/OS.h"
#include "Utils/Scripting/ScriptBindings.h"
#include <iostream>
#include <string>
#include <mutex>
#include <set>

namespace Falcor
{
namespace
{
std::mutex sMutex;
Logger::Level sVerbosity = Logger::Level::Info;
const Logger::OutputFlags sOutputs = Logger::OutputFlags::Console | Logger::OutputFlags::File;
std::filesystem::path sLogFilePath;

bool sInitialized = false;
FILE* sLogFile = nullptr;

std::filesystem::path generateLogFilePath()
{
    std::string prefix = getExecutableName();
    std::filesystem::path directory = getRuntimeDirectory();
    return findAvailableFilename(prefix, directory, "log");
}

FILE* openLogFile()
{
    FILE* pFile = nullptr;

    if (sLogFilePath.empty())
    {
        sLogFilePath = generateLogFilePath();
    }

    pFile = std::fopen(sLogFilePath.string().c_str(), "w");
    if (pFile != nullptr)
    {
        // Success
        return pFile;
    }

    // If we got here, we couldn't create a log file
    FALCOR_UNREACHABLE();
    return pFile;
}

void printToLogFile(const std::string& s)
{
    if (!sInitialized)
    {
        sLogFile = openLogFile();
        sInitialized = true;
    }

    if (sLogFile)
    {
        std::fprintf(sLogFile, "%s", s.c_str());
        std::fflush(sLogFile);
    }
}
} // namespace

void Logger::shutdown()
{
    if (sLogFile)
    {
        fclose(sLogFile);
        sLogFile = nullptr;
        sInitialized = false;
    }
}

inline const char* getLogLevelString(Logger::Level level)
{
    switch (level)
    {
    case Logger::Level::Fatal:
        return "(Fatal)";
    case Logger::Level::Error:
        return "(Error)";
    case Logger::Level::Warning:
        return "(Warning)";
    case Logger::Level::Info:
        return "(Info)";
    case Logger::Level::Debug:
        return "(Debug)";
    default:
        FALCOR_UNREACHABLE();
        return nullptr;
    }
}

class MessageDeduplicator
{
public:
    static MessageDeduplicator& instance()
    {
        static MessageDeduplicator sInstance;
        return sInstance;
    }

    bool isDuplicate(std::string_view msg)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        auto it = mStrings.find(msg);
        if (it != mStrings.end())
            return true;
        mStrings.insert(std::string(msg));
        return false;
    }

private:
    MessageDeduplicator() = default;

    std::mutex mMutex;
    std::set<std::string, std::less<>> mStrings;
};

void Logger::log(Level level, const std::string_view msg, Frequency frequency)
{
    std::lock_guard<std::mutex> lock(sMutex);
    if (level <= sVerbosity)
    {
        std::string s = fmt::format("{} {}\n", getLogLevelString(level), msg);

        if (frequency == Frequency::Once && MessageDeduplicator::instance().isDuplicate(s))
            return;

        // Write to console.
        if (is_set(sOutputs, OutputFlags::Console))
        {
            auto& os = level > Logger::Level::Error ? std::cout : std::cerr;
            os << s;
            os.flush();
        }

        // Write to file.
        if (is_set(sOutputs, OutputFlags::File))
        {
            printToLogFile(s);
        }

        // Write to debug window if debugger is attached.
        if (is_set(sOutputs, OutputFlags::DebugWindow) && isDebuggerPresent())
        {
            printToDebugWindow(s);
        }
    }
}

void Logger::setVerbosity(Level level)
{
    std::lock_guard<std::mutex> lock(sMutex);
    sVerbosity = level;
}

Logger::Level Logger::getVerbosity()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sVerbosity;
}

void Logger::setOutputs(OutputFlags outputs)
{
    std::lock_guard<std::mutex> lock(sMutex);
    // sOutputs = outputs;
}

Logger::OutputFlags Logger::getOutputs()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sOutputs;
}

void Logger::setLogFilePath(const std::filesystem::path& path)
{
    std::lock_guard<std::mutex> lock(sMutex);
    if (sLogFile)
    {
        fclose(sLogFile);
        sLogFile = nullptr;
        sInitialized = false;
    }
    sLogFilePath = path;
}

std::filesystem::path Logger::getLogFilePath()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sLogFilePath;
}

FALCOR_SCRIPT_BINDING(Logger)
{
    using namespace pybind11::literals;

    pybind11::class_<Logger> logger(m, "Logger");

    pybind11::enum_<Logger::Level> level(logger, "Level");
    level.value("Disabled", Logger::Level::Disabled);
    level.value("Fatal", Logger::Level::Fatal);
    level.value("Error", Logger::Level::Error);
    level.value("Warning", Logger::Level::Warning);
    level.value("Info", Logger::Level::Info);
    level.value("Debug", Logger::Level::Debug);

    pybind11::enum_<Logger::OutputFlags> outputFlags(logger, "OutputFlags");
    outputFlags.value("None_", Logger::OutputFlags::None);
    outputFlags.value("Console", Logger::OutputFlags::Console);
    outputFlags.value("File", Logger::OutputFlags::File);
    outputFlags.value("DebugWindow", Logger::OutputFlags::DebugWindow);

    logger.def_property_static(
        "verbosity",
        [](pybind11::object) { return Logger::getVerbosity(); },
        [](pybind11::object, Logger::Level verbosity) { Logger::setVerbosity(verbosity); }
    );
    logger.def_property_static(
        "outputs",
        [](pybind11::object) { return Logger::getOutputs(); },
        [](pybind11::object, Logger::OutputFlags outputs) { Logger::setOutputs(outputs); }
    );
    logger.def_property_static(
        "log_file_path",
        [](pybind11::object) { return Logger::getLogFilePath(); },
        [](pybind11::object, std::filesystem::path path) { Logger::setLogFilePath(path); }
    );

    logger.def_static(
        "log",
        [](Logger::Level level, const std::string_view msg) { Logger::log(level, msg, Logger::Frequency::Always); },
        "level"_a,
        "msg"_a
    );
}

} // namespace Falcor
