#pragma once
#include "Core/Macros.h"
#include "Core/Platform/OS.h"
#include <pybind11/pybind11.h>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>

namespace Falcor
{
class FALCOR_API Scripting
{
public:
    static const FileDialogFilterVec kFileExtensionFilters;

    /**
     * Represents a context for executing scripts.
     * Wraps the globals dictionary that is passed to the script on execution.
     * The context can be used to pass/retrieve variables to/from the executing script.
     */
    class Context
    {
    public:
        Context(pybind11::dict globals) : mGlobals(globals) {}

        Context()
        {
            // Copy __builtins__ to our empty globals dictionary.
            mGlobals["__builtins__"] = pybind11::globals()["__builtins__"];
        }

        ~Context()
        {
            // We need to manually cleanup the globals dictionary to avoid keeping references to objects.
            for (const auto& it : mGlobals)
                mGlobals[it.first] = nullptr;
        }

        template<typename T>
        struct ObjectDesc
        {
            ObjectDesc(const std::string& name_, const T& obj_) : name(name_), obj(obj_) {}
            operator const T&() const { return obj; }
            std::string name;
            T obj;
        };

        template<typename T>
        std::vector<ObjectDesc<T>> getObjects()
        {
            std::vector<ObjectDesc<T>> v;
            for (const auto& l : mGlobals)
            {
                try
                {
                    if (!l.second.is_none())
                    {
                        v.push_back(ObjectDesc<T>(l.first.cast<std::string>(), l.second.cast<T>()));
                    }
                }
                catch (const std::exception&)
                {}
            }
            return v;
        }

        template<typename T>
        void setObject(const std::string& name, T obj)
        {
            mGlobals[name.c_str()] = obj;
        }

        template<typename T>
        T getObject(const std::string& name) const
        {
            return mGlobals[name.c_str()].cast<T>();
        }

        bool containsObject(const std::string& name) const { return mGlobals.contains(name.c_str()); }

    private:
        friend class Scripting;
        pybind11::dict mGlobals;
    };

    /**
     * Starts the script engine.
     * This will initialize the Python interpreter and setup the default context.
     */
    static void start();

    /**
     * Shuts the script engine down.
     */
    static void shutdown();

    /**
     * Returns true if the script engine is running.
     */
    static bool isRunning() { return sRunning; }

    /**
     * Returns the default context.
     */
    static Context& getDefaultContext();

    /**
     * Returns the context of the currently executing script.
     */
    static Context getCurrentContext();

    struct RunResult
    {
        std::string out;
        std::string err;
    };

    /**
     * Run a script.
     * @param[in] script Script to run.
     * @param[in] context Script execution context.
     * @param[in] captureOutput Enable capturing stdout/stderr and returning it in RunResult.
     * @return Returns the captured output if enabled.
     */
    static RunResult runScript(std::string_view script, Context& context = getDefaultContext(), bool captureOutput = false);

    /**
     * Run a script from a file.
     * @param[in] path Path of the script to run.
     * @param[in] context Script execution context.
     * @param[in] captureOutput Enable capturing stdout/stderr and returning it in RunResult.
     * @return Returns the captured output if enabled.
     */
    static RunResult runScriptFromFile(
        const std::filesystem::path& path,
        Context& context = getDefaultContext(),
        bool captureOutput = false
    );

    /**
     * Interpret a script and return the evaluated result.
     * @param[in] script Script to run.
     * @param[in] context Script execution context.
     * @return Returns a string representation of the evaluated result of the script.
     */
    static std::string interpretScript(const std::string& script, Context& context = getDefaultContext());

private:
    static bool sRunning;                            // TODO: REMOVEGLOBAL
    static std::unique_ptr<Context> sDefaultContext; // TODO: REMOVEGLOBAL
};
} // namespace Falcor
