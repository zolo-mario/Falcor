#pragma once
#include "Material.h"
#include "MERLMaterialData.slang"

namespace Falcor
{
    class MERLFile;

    /** Class representing a measured material from the MERL BRDF database.

        For details refer to:
        Wojciech Matusik, Hanspeter Pfister, Matt Brand and Leonard McMillan.
        "A Data-Driven Reflectance Model". ACM Transactions on Graphics,
        vol. 22(3), 2003, pages 759-769.
    */
    class FALCOR_API MERLMaterial : public Material
    {
        FALCOR_OBJECT(MERLMaterial)
    public:
        static ref<MERLMaterial> create(ref<Device> pDevice, const std::string& name, const std::filesystem::path& path) { return make_ref<MERLMaterial>(pDevice, name, path); }

        MERLMaterial(ref<Device> pDevice, const std::string& name, const std::filesystem::path& path);
        MERLMaterial(ref<Device> pDevice, const MERLFile& merlFile);

        bool renderUI(Gui::Widgets& widget) override;
        Material::UpdateFlags update(MaterialSystem* pOwner) override;
        bool isEqual(const ref<Material>& pOther) const override;
        MaterialDataBlob getDataBlob() const override { return prepareDataBlob(mData); }
        ProgramDesc::ShaderModuleList getShaderModules() const override;
        TypeConformanceList getTypeConformances() const override;

        size_t getMaxBufferCount() const override { return 1; }

    protected:
        void init(const MERLFile& merlFile);

        std::filesystem::path mPath;        ///< Full path to the BRDF loaded.
        std::string mBRDFName;              ///< This is the file basename without extension.

        MERLMaterialData mData;             ///< Material parameters.
        ref<Buffer> mpBRDFData;             ///< GPU buffer holding all BRDF data as float3 array.
        ref<Texture> mpAlbedoLUT;           ///< Precomputed albedo lookup table.
        ref<Sampler> mpLUTSampler;          ///< Sampler for accessing the LUT texture.
    };
}
