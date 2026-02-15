#pragma once
#include "Falcor.h"
#include <memory>
#include "Core/SampleApp.h"
#include "Scene/Camera/Camera.h"
#include "Scene/Camera/CameraController.h"

using namespace Falcor;

// Match D3D12 Bundles sample constants
static constexpr uint32_t kCityRowCount = 10;
static constexpr uint32_t kCityColumnCount = 3;
static constexpr uint32_t kCityCount = kCityRowCount * kCityColumnCount;

// occcity.bin layout (from occtity.h)
static constexpr uint32_t kVertexDataOffset = 524288;
static constexpr uint32_t kVertexDataSize = 820248;
static constexpr uint32_t kIndexDataOffset = 1344536;
static constexpr uint32_t kIndexDataSize = 74568;
static constexpr uint32_t kTextureOffset = 0;
static constexpr uint32_t kTextureSize = 524288;
static constexpr uint32_t kTextureWidth = 1024;
static constexpr uint32_t kTextureHeight = 1024;
static constexpr uint32_t kVertexStride = 44; // POSITION(12) + NORMAL(12) + TEXCOORD(8) + TANGENT(12)
static constexpr uint32_t kNumIndices = kIndexDataSize / 4; // R32_UINT

class D3D12Bundles : public SampleApp
{
public:
    D3D12Bundles(const SampleAppConfig& config);
    ~D3D12Bundles();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    std::filesystem::path findOcctityBin();
    void loadOcctityAssets(const std::filesystem::path& path);

    ref<Buffer> mpVertexBuffer;
    ref<Buffer> mpIndexBuffer;
    ref<Texture> mpTexture;
    ref<Vao> mpVao;

    ref<Program> mpProgramSimple;
    ref<Program> mpProgramAlt;
    ref<ProgramVars> mpVarsSimple;
    ref<ProgramVars> mpVarsAlt;

    ref<GraphicsState> mpStateSimple;
    ref<GraphicsState> mpStateAlt;
    ref<DepthStencilState> mpDepthStencilState;
    ref<RasterizerState> mpRasterizerState;
    ref<Sampler> mpSampler;

    std::vector<float4x4> mModelMatrices;

    ref<Camera> mpCamera;
    std::unique_ptr<OrbiterCameraController> mpCameraController;
};
