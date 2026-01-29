#include "DiffuseSpecularUtils.h"
#include "DiffuseSpecularData.slang"
#include "Utils/Logger.h"
#include "Utils/Color/ColorHelpers.slang"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace Falcor
{
    namespace
    {
        bool get_to(const json& j, float3& v)
        {
            if (!j.is_array())
                return false;
            if (j.size() != v.length())
                return false;
            j[0].get_to(v.x);
            j[1].get_to(v.y);
            j[2].get_to(v.z);
            return true;
        }
    }

    bool DiffuseSpecularUtils::loadJSONData(const std::filesystem::path& path, DiffuseSpecularData& data)
    {
        // Set default parameters for json data is not available.
        // The default is a mix between diffuse and specular with medium roughness.
        data = {};
        data.baseColor = float3(0.5f);
        data.roughness = 0.5f;
        data.metallic = 0.5f;

        // Try loading data from json file.
        std::ifstream ifs(path);
        if (!ifs.good())
        {
            logWarning("DiffuseSpecularUtils: Failed to open file '{}' for reading.", path);
            return false;
        }

        try
        {
            json doc = json::parse(ifs);

            DiffuseSpecularData d = {};
            float3 baseColorSRGB = {};
            get_to(doc["base_color_srgb"], baseColorSRGB);
            d.baseColor = sRGBToLinear(baseColorSRGB);
            d.roughness = doc["roughness"];
            d.specular = doc["specular"];
            d.metallic = doc["metallic"];
            d.lossValue = doc["loss_value"];

            data = d;
        }
        catch (const json::exception& e)
        {
            logWarning("DiffuseSpecularUtils: Error ({}) when parsing file '{}'.", std::string(e.what()), path);
            return false;
        }

        return true;
    }

    bool DiffuseSpecularUtils::renderUI(Gui::Widgets& widget, DiffuseSpecularData& data)
    {
        DiffuseSpecularData prevData = data;
        widget.rgbColor("baseColor", data.baseColor);
        widget.var("roughness", data.roughness, 0.f, 1.f);
        widget.var("metallic", data.metallic, 0.f, 1.f);
        widget.var("specular", data.specular, 0.f, 1.f);

        return data != prevData;
    }
}
