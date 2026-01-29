#pragma once

#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/API/Texture.h"
#include "Core/API/Sampler.h"
#include "Utils/UI/Gui.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Falcor
{
    struct ShaderVar;

    class FALCOR_API LightProfile : public Object
    {
        FALCOR_OBJECT(LightProfile)
    public:
        static ref<LightProfile> createFromIesProfile(ref<Device> pDevice, const std::filesystem::path& path, bool normalize);

        void bake(RenderContext* pRenderContext);

        /** Set the light profile into a shader var.
        */
        void bindShaderData(const ShaderVar& var) const;

        /** Render the UI.
        */
        void renderUI(Gui::Widgets& widget) const;

    private:
        LightProfile(ref<Device> pDevice, const std::string& name, const std::vector<float>& rawData);

        ref<Device> mpDevice;
        std::string mName;
        std::vector<float> mRawData;
        ref<Texture> mpTexture;
        ref<Sampler> mpSampler;
        float mFluxFactor = 0.f;
    };
}
