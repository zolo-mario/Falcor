#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Core/API/RtAccelerationStructure.h"

using namespace Falcor;

// Match D3D12 RaytracingHlslCompat.h
struct Viewport
{
    float left;
    float top;
    float right;
    float bottom;
};

struct RayGenConstantBuffer
{
    Viewport viewport;
    Viewport stencil;
};

class D3D12RaytracingHelloWorld : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12RaytracingHelloWorld, "D3D12RaytracingHelloWorld", SampleBase::PluginInfo{"Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld"});

    explicit D3D12RaytracingHelloWorld(SampleApp* pHost);
    ~D3D12RaytracingHelloWorld();

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void buildAccelerationStructures(RenderContext* pRenderContext);
    void updateViewportConstants(uint32_t width, uint32_t height);

    // Geometry (match D3D12 sample: 3 vertices, 1 triangle)
    ref<Buffer> mpVertexBuffer;
    ref<Buffer> mpIndexBuffer;

    // Acceleration structures
    ref<RtAccelerationStructure> mpBlas;
    ref<RtAccelerationStructure> mpTlas;
    ref<Buffer> mpBlasBuffer;
    ref<Buffer> mpTlasBuffer;
    ref<Buffer> mpScratchBuffer;
    ref<Buffer> mpInstanceBuffer;

    // Raytracing
    ref<Program> mpRaytraceProgram;
    ref<RtProgramVars> mpRtVars;
    ref<Texture> mpRtOut;

    // Viewport constants (match D3D12 RayGenConstantBuffer)
    RayGenConstantBuffer mRayGenCB;
};
