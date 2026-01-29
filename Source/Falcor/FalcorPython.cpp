#include "Core/API/Device.h"
#include "Core/Plugin.h"
#include "Utils/Scripting/ScriptBindings.h"

#include <pybind11/pybind11.h>

/**
 * This function checks if the falcor module is loaded from a Falcor
 * application running the embedded python interpreter (e.g. Mogwai).
 */
static bool isLoadedFromEmbeddedPython()
{
    try
    {
        auto os = pybind11::module::import("os");
        std::string value = os.attr("environ")["FALCOR_EMBEDDED_PYTHON"].cast<pybind11::str>();
        return value == "1";
    }
    catch (const std::exception&)
    {}

    return false;
}

PYBIND11_MODULE(falcor_ext, m)
{
    if (!isLoadedFromEmbeddedPython())
    {
        Falcor::Logger::setOutputs(Falcor::Logger::OutputFlags::Console | Falcor::Logger::OutputFlags::DebugWindow);
        Falcor::Device::enableAgilitySDK();
        Falcor::PluginManager::instance().loadAllPlugins();
    }

    m.doc() = "Falcor python bindings";
    Falcor::ScriptBindings::initModule(m);
}
