#include "MERLMaterial.h"
#include "Core/API/Device.h"
#include "Utils/Logger.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "GlobalState.h"
#include "Scene/Material/MERLFile.h"
#include "Scene/Material/MaterialSystem.h"
#include "Scene/Material/DiffuseSpecularUtils.h"

namespace Falcor
{
    namespace
    {
        static_assert((sizeof(MaterialHeader) + sizeof(MERLMaterialData)) <= sizeof(MaterialDataBlob), "MERLMaterialData is too large");

        const char kShaderFile[] = "Rendering/Materials/MERLMaterial.slang";
    }

    MERLMaterial::MERLMaterial(ref<Device> pDevice, const std::string& name, const std::filesystem::path& path)
        : Material(pDevice, name, MaterialType::MERL)
    {
        FALCOR_CHECK(!path.empty(), "Missing path.");

        MERLFile merlFile(path);
        init(merlFile);

        // Create albedo LUT texture.
        auto lut = merlFile.prepareAlbedoLUT(mpDevice);
        FALCOR_CHECK(!lut.empty() && sizeof(lut[0]) == sizeof(float4), "Expected albedo LUT in float4 format.");
        static_assert(MERLFile::kAlbedoLUTFormat == ResourceFormat::RGBA32Float);
        mpAlbedoLUT = mpDevice->createTexture2D((uint32_t)lut.size(), 1, MERLFile::kAlbedoLUTFormat, 1, 1, lut.data(), ResourceBindFlags::ShaderResource);
    }

    MERLMaterial::MERLMaterial(ref<Device> pDevice, const MERLFile& merlFile)
        : Material(pDevice, "", MaterialType::MERL)
    {
        init(merlFile);
    }

    void MERLMaterial::init(const MERLFile& merlFile)
    {
        mPath = merlFile.getDesc().path;
        mBRDFName = merlFile.getDesc().name;
        mData.extraData = merlFile.getDesc().extraData;

        // Create GPU buffer.
        const auto& brdf = merlFile.getData();
        FALCOR_CHECK(!brdf.empty() && sizeof(brdf[0]) == sizeof(float3), "Expected BRDF data in float3 format.");
        mpBRDFData = mpDevice->createBuffer(brdf.size() * sizeof(brdf[0]), ResourceBindFlags::ShaderResource, MemoryType::DeviceLocal, brdf.data());

        // Create sampler for albedo LUT.
        Sampler::Desc desc;
        desc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Point, TextureFilteringMode::Point);
        desc.setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
        desc.setMaxAnisotropy(1);
        mpLUTSampler = mpDevice->createSampler(desc);

        markUpdates(Material::UpdateFlags::ResourcesChanged);
    }

    bool MERLMaterial::renderUI(Gui::Widgets& widget)
    {
        bool changed = false;

        widget.text("MERL BRDF " + mBRDFName);
        widget.tooltip("Full path the BRDF was loaded from:\n" + mPath.string(), true);

        if (auto g = widget.group("Approx diffuse/specular sampling"))
        {
            if (DiffuseSpecularUtils::renderUI(g, mData.extraData))
            {
                markUpdates(Material::UpdateFlags::DataChanged);
                changed = true;
            }
        }

        return changed;
    }

    Material::UpdateFlags MERLMaterial::update(MaterialSystem* pOwner)
    {
        FALCOR_ASSERT(pOwner);

        auto flags = Material::UpdateFlags::None;
        if (mUpdates != Material::UpdateFlags::None)
        {
            uint32_t bufferID = pOwner->addBuffer(mpBRDFData);
            uint32_t samplerID = pOwner->addTextureSampler(mpLUTSampler);

            if (mData.bufferID != bufferID || mData.samplerID != samplerID)
            {
                mUpdates |= Material::UpdateFlags::DataChanged;
            }
            mData.bufferID = bufferID;
            mData.samplerID = samplerID;

            updateTextureHandle(pOwner, mpAlbedoLUT, mData.texAlbedoLUT);

            flags |= mUpdates;
            mUpdates = Material::UpdateFlags::None;
        }

        return flags;
    }

    bool MERLMaterial::isEqual(const ref<Material>& pOther) const
    {
        auto other = dynamic_ref_cast<MERLMaterial>(pOther);
        if (!other) return false;

        if (!isBaseEqual(*other)) return false;
        if (mPath != other->mPath) return false;

        return true;
    }

    ProgramDesc::ShaderModuleList MERLMaterial::getShaderModules() const
    {
        return { ProgramDesc::ShaderModule::fromFile(kShaderFile) };
    }

    TypeConformanceList MERLMaterial::getTypeConformances() const
    {
        return { {{"MERLMaterial", "IMaterial"}, (uint32_t)MaterialType::MERL} };
    }

    FALCOR_SCRIPT_BINDING(MERLMaterial)
    {
        using namespace pybind11::literals;

        FALCOR_SCRIPT_BINDING_DEPENDENCY(Material)

        pybind11::class_<MERLMaterial, Material, ref<MERLMaterial>> material(m, "MERLMaterial");
        auto create = [] (const std::string& name, const std::filesystem::path& path)
        {
            return MERLMaterial::create(accessActivePythonSceneBuilder().getDevice(), name, getActiveAssetResolver().resolvePath(path));
        };
        material.def(pybind11::init(create), "name"_a, "path"_a); // PYTHONDEPRECATED
    }
}
