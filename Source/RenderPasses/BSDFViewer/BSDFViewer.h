#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Utils/Sampling/SampleGenerator.h"
#include "Utils/Debug/PixelDebug.h"
#include "Scene/Lights/EnvMap.h"
#include "BSDFViewerParams.slang"

using namespace Falcor;

class BSDFViewer : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(BSDFViewer, "BSDFViewer", "BSDF inspection utility.");

    static ref<BSDFViewer> create(ref<Device> pDevice, const Properties& props) { return make_ref<BSDFViewer>(pDevice, props); }

    BSDFViewer(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override;

private:
    void parseProperties(const Properties& props);
    bool loadEnvMap(const std::filesystem::path& path);
    void readPixelData();

    // Internal state

    /// Loaded scene if any, nullptr otherwise.
    ref<Scene> mpScene;
    /// Environment map if loaded, nullptr otherwise.
    ref<EnvMap> mpEnvMap;
    /// Use environment map if available.
    bool mUseEnvMap = true;

    /// Parameters shared with the shaders.
    BSDFViewerParams mParams;
    /// Random number generator for the integrator.
    ref<SampleGenerator> mpSampleGenerator;
    bool mOptionsChanged = false;

    /// GPU fence for synchronizing readback.
    ref<Fence> mpFence;
    /// Buffer for data for the selected pixel.
    ref<Buffer> mpPixelDataBuffer;
    /// Staging buffer for readback of pixel data.
    ref<Buffer> mpPixelStagingBuffer;
    /// Pixel data for the selected pixel (if valid).
    PixelData mPixelData;
    bool mPixelDataValid = false;
    bool mPixelDataAvailable = false;

    /// Utility class for pixel debugging (print in shaders).
    std::unique_ptr<PixelDebug> mpPixelDebug;

    ref<ComputePass> mpViewerPass;

    // UI variables
    Gui::DropdownList mMaterialList;
};
