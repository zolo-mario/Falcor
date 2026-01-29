#pragma once
#include "ScriptBindings.h"
#include "Core/Platform/OS.h"
#include <algorithm>
#include <filesystem>
#include <string>

namespace Falcor
{
/**
 * Helper class to write Python script code including:
 * - calling functions
 * - calling member functions
 * - getting/setting properties
 *
 * Arguments are automatically converted from C++ types to Python code using `repr()`.
 */
class ScriptWriter
{
public:
    struct VariableName
    {
        std::string name;
        explicit VariableName(const std::string& name) : name(name) {}
    };

    static std::string makeFunc(const std::string& func) { return func + "()\n"; }

    template<typename T>
    static std::string getArgString(const T& arg);

    template<typename Arg, typename... Args>
    static std::string makeFunc(const std::string& func, Arg first, Args... args)
    {
        std::string s = func + "(" + getArgString(first);
        int32_t dummy[] = {0, (s += ", " + getArgString(args), 0)...};
        s += ")\n";
        return s;
    }

    static std::string makeMemberFunc(const std::string& var, const std::string& func) { return std::string(var) + "." + makeFunc(func); }

    template<typename Arg, typename... Args>
    static std::string makeMemberFunc(const std::string& var, const std::string& func, Arg first, Args... args)
    {
        std::string s(var);
        s += std::string(".") + makeFunc(func, first, args...);
        return s;
    }

    static std::string makeGetProperty(const std::string& var, const std::string& property) { return var + "." + property + "\n"; }

    template<typename Arg>
    static std::string makeSetProperty(const std::string& var, const std::string& property, Arg arg)
    {
        return var + "." + property + " = " + getArgString(arg) + "\n";
    }

    static std::string getPathString(const std::filesystem::path& path)
    {
        std::string str = path.string();
        std::replace(str.begin(), str.end(), '\\', '/');
        return str;
    }
};

template<typename T>
std::string ScriptWriter::getArgString(const T& arg)
{
    return ScriptBindings::repr(arg);
}

template<>
inline std::string ScriptWriter::getArgString(const pybind11::dict& dict)
{
    return pybind11::str(static_cast<pybind11::dict>(dict));
}

template<>
inline std::string ScriptWriter::getArgString(const ScriptWriter::VariableName& varName)
{
    return varName.name;
}

} // namespace Falcor
