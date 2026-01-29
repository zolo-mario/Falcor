#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Utils/Scripting/ndarray.h"
#if FALCOR_HAS_CUDA
#include "Utils/CudaUtils.h"
#endif

using namespace Falcor;

class TestPyTorchPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(TestPyTorchPass, "TestPyTorchPass", "Test pass for PyTorch tensor interop.");

    using PyTorchTensor = pybind11::ndarray<pybind11::pytorch, float>;

    static ref<TestPyTorchPass> create(ref<Device> pDevice, const Properties& props) { return make_ref<TestPyTorchPass>(pDevice, props); }

    TestPyTorchPass(ref<Device> pDevice, const Properties& props);
    virtual ~TestPyTorchPass();

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override {}
    virtual void renderUI(Gui::Widgets& widget) override {}
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    static void registerScriptBindings(pybind11::module& m);

    PyTorchTensor generateData(const uint3 dim, const uint32_t offset);
    bool verifyData(const uint3 dim, const uint32_t offset, PyTorchTensor data);

private:
    /// GPU buffer for generated data.
    ref<Buffer> mpBuffer;
    ref<Buffer> mpCounterBuffer;
    ref<Buffer> mpCounterStagingBuffer;
#if FALCOR_HAS_CUDA
    /// Shared CUDA/Falcor buffer for passing data from Falcor to PyTorch asynchronously.
    InteropBuffer mSharedWriteBuffer;
    /// Shared CUDA/Falcor buffer for passing data from PyTorch to Falcor asynchronously.
    InteropBuffer mSharedReadBuffer;
#endif
    ref<ComputePass> mpWritePass;
    ref<ComputePass> mpReadPass;
    ref<Fence> mpFence;
};
