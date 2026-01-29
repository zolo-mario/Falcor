#pragma once
#include "Utils/UI/Gui.h"
#include <nlohmann/json_fwd.hpp>
#include <filesystem>

namespace Falcor
{
    struct DiffuseSpecularData;

    class DiffuseSpecularUtils
    {
    public:
        static bool loadJSONData(const std::filesystem::path& path, DiffuseSpecularData& data);
        static bool renderUI(Gui::Widgets& widget, DiffuseSpecularData& data);
    };
}
