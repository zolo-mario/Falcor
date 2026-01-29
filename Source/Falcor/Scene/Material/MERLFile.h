#pragma once
#include "Core/API/fwd.h"
#include "Core/API/Formats.h"
#include "Utils/Math/Vector.h"
#include "Scene/Material/DiffuseSpecularData.slang"
#include <filesystem>
#include <memory>

namespace Falcor
{
    class Device;

    /** Class for loading a measured material from the MERL BRDF database.
        Additional metadata is loaded along with the BRDF if available.
    */
    class FALCOR_API MERLFile
    {
    public:
        struct Desc
        {
            std::string name;                   ///< Name of the BRDF.
            std::filesystem::path path;         ///< Full path to the loaded BRDF.
            DiffuseSpecularData extraData = {}; ///< Parameters for a best fit BRDF approximation.
        };

        static constexpr ResourceFormat kAlbedoLUTFormat = ResourceFormat::RGBA32Float;

        MERLFile() = default;

        /** Constructs a new object and loads a MERL BRDF. Throws on error.
            \param[in] path Path to the binary MERL file.
        */
        MERLFile(const std::filesystem::path& path);

        /** Loads a MERL BRDF.
            \param[in] path Path to the binary MERL file.
            \return True if the BRDF was successfully loaded.
        */
        bool loadBRDF(const std::filesystem::path& path);

        /** Prepare an albedo lookup table.
            The table is loaded from disk or recomputed if needed.
            \param[in] pDevice The device.
            \return Albedo lookup table that can be used with `kAlbedoLUTFormat`.
        */
        const std::vector<float4>& prepareAlbedoLUT(ref<Device> pDevice);

        const Desc& getDesc() const { return mDesc; }
        const std::vector<float3>& getData() const { return mData; }

    private:
        void prepareData(const int dims[3], const std::vector<double>& data);
        void computeAlbedoLUT(ref<Device> pDevice, const size_t binCount);

        Desc mDesc;                     ///< BRDF description and sampling parameters.
        std::vector<float3> mData;      ///< BRDF data in RGB float format.
        std::vector<float4> mAlbedoLUT; ///< Precomputed albedo lookup table.
    };
}
