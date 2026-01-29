#pragma once
#include "Falcor.h"
#include "Core/Enum.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

/**
 * Simple composite pass that blends two buffers together.
 *
 * Each input A and B can be independently scaled, and the output C
 * is computed C = A <op> B, where the blend operation is configurable.
 * If the output buffer C is of integer format, floating point values
 * are converted to integers using round-to-nearest-even.
 */
class Composite : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(Composite, "Composite", "Composite pass.");

    /**
     * Composite modes.
     */
    enum class Mode
    {
        Add,
        Multiply,
    };

    FALCOR_ENUM_INFO(
        Mode,
        {
            {Mode::Add, "Add"},
            {Mode::Multiply, "Multiply"},
        }
    );

    static ref<Composite> create(ref<Device> pDevice, const Properties& props) { return make_ref<Composite>(pDevice, props); }

    Composite(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    DefineList getDefines() const;

    uint2 mFrameDim = {0, 0};
    Mode mMode = Mode::Add;
    float mScaleA = 1.f;
    float mScaleB = 1.f;
    ResourceFormat mOutputFormat = ResourceFormat::RGBA32Float;

    ref<ComputePass> mCompositePass;
};

FALCOR_ENUM_REGISTER(Composite::Mode);
