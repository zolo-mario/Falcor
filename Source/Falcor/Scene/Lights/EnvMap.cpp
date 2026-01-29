#include "EnvMap.h"
#include "Core/API/Device.h"
#include "Core/Program/ShaderVar.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "GlobalState.h"

namespace Falcor
{
    ref<EnvMap> EnvMap::create(ref<Device> pDevice, const ref<Texture>& pTexture)
    {
        return ref<EnvMap>(new EnvMap(pDevice, pTexture));
    }

    ref<EnvMap> EnvMap::createFromFile(ref<Device> pDevice, const std::filesystem::path& path)
    {
        // Load environment map from file. Set it to generate mips and use linear color.
        auto pTexture = Texture::createFromFile(pDevice, path, true, false);
        if (!pTexture) return nullptr;
        return create(pDevice, pTexture);
    }

    void EnvMap::renderUI(Gui::Widgets& widgets)
    {
        auto rotation = getRotation();
        if (widgets.var("Rotation XYZ", rotation, -360.f, 360.f, 0.5f)) setRotation(rotation);
        widgets.var("Intensity", mData.intensity, 0.f, 1000000.f);
        widgets.var("Color tint", mData.tint, 0.f, 1.f);
        widgets.text("EnvMap: " + mpEnvMap->getSourcePath().string());

        widgets.text(fmt::format("Resolution: {}x{}", mpEnvMap->getWidth(), mpEnvMap->getHeight()));
        widgets.text(fmt::format("Mip levels: {}", mpEnvMap->getMipCount()));
        widgets.text(fmt::format("Format: {}", to_string(mpEnvMap->getFormat())));
    }

    void EnvMap::setRotation(float3 degreesXYZ)
    {
        if (any(degreesXYZ != mRotation))
        {
            mRotation = degreesXYZ;

            float4x4 transform = math::matrixFromRotationXYZ(math::radians(mRotation.x), math::radians(mRotation.y), math::radians(mRotation.z));

            mData.transform = transform;
            mData.invTransform = inverse(transform);
        }
    }

    void EnvMap::setTransform(const float4x4& xform)
    {
        float3 rotation;
        // Extract rotation from the computed transform
        math::extractEulerAngleXYZ(xform, rotation.x, rotation.y, rotation.z);
        setRotation(math::degrees(rotation));
    }

    void EnvMap::setIntensity(float intensity)
    {
        mData.intensity = intensity;
    }

    void EnvMap::setTint(const float3& tint)
    {
        mData.tint = tint;
    }

    void EnvMap::bindShaderData(const ShaderVar& var) const
    {
        FALCOR_ASSERT(var.isValid());

        // Set variables.
        var["data"].setBlob(mData);

        // Bind resources.
        var["envMap"].setTexture(mpEnvMap);
        var["envSampler"].setSampler(mpEnvSampler);
    }

    EnvMap::Changes EnvMap::beginFrame()
    {
        mChanges = Changes::None;

        if (mData.transform != mPrevData.transform) mChanges |= Changes::Transform;
        if (mData.intensity != mPrevData.intensity) mChanges |= Changes::Intensity;
        if (any(mData.tint != mPrevData.tint)) mChanges |= Changes::Intensity;

        mPrevData = mData;

        return getChanges();
    }

    uint64_t EnvMap::getMemoryUsageInBytes() const
    {
        return mpEnvMap ? mpEnvMap->getTextureSizeInBytes() : 0;
    }

    EnvMap::EnvMap(ref<Device> pDevice, const ref<Texture>& pTexture)
        : mpDevice(pDevice)
    {
        FALCOR_CHECK(mpDevice != nullptr, "'pDevice' must be a valid device");
        FALCOR_CHECK(pTexture != nullptr, "'pTexture' must be a valid texture");

        mpEnvMap = pTexture;

        // Create sampler.
        // The lat-long map wraps around horizontally, but not vertically. Set the sampler to only wrap in U.
        Sampler::Desc samplerDesc;
        samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear);
        samplerDesc.setAddressingMode(TextureAddressingMode::Wrap, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
        mpEnvSampler = mpDevice->createSampler(samplerDesc);
    }

    FALCOR_SCRIPT_BINDING(EnvMap)
    {
        using namespace pybind11::literals;

        pybind11::class_<EnvMap, ref<EnvMap>> envMap(m, "EnvMap");
        auto createFromFile = [](const std::filesystem::path &path) {
            ref<EnvMap> envMap = EnvMap::createFromFile(accessActivePythonSceneBuilder().getDevice(), getActiveAssetResolver().resolvePath(path));
            if (!envMap)
                FALCOR_THROW("Failed to load environment map from '{}'.", path);
            return envMap;
        };
        envMap.def(pybind11::init(createFromFile), "path"_a); // PYTHONDEPRECATED
        envMap.def_static("createFromFile", createFromFile, "path"_a);
        envMap.def_property_readonly("path", &EnvMap::getPath);
        envMap.def_property("rotation", &EnvMap::getRotation, &EnvMap::setRotation);
        envMap.def_property("intensity", &EnvMap::getIntensity, &EnvMap::setIntensity);
        envMap.def_property("tint", &EnvMap::getTint, &EnvMap::setTint);
    }
}
