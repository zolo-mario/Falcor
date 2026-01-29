#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Core/Pass/FullScreenPass.h"

using namespace Falcor;

class SVGFPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(SVGFPass, "SVGFPass", "SVGF denoising pass.");

    static ref<SVGFPass> create(ref<Device> pDevice, const Properties& props) { return make_ref<SVGFPass>(pDevice, props); }

    SVGFPass(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    void allocateFbos(uint2 dim, RenderContext* pRenderContext);
    void clearBuffers(RenderContext* pRenderContext, const RenderData& renderData);

    void computeLinearZAndNormal(RenderContext* pRenderContext, ref<Texture> pLinearZTexture, ref<Texture> pWorldNormalTexture);
    void computeReprojection(
        RenderContext* pRenderContext,
        ref<Texture> pAlbedoTexture,
        ref<Texture> pColorTexture,
        ref<Texture> pEmissionTexture,
        ref<Texture> pMotionVectorTexture,
        ref<Texture> pPositionNormalFwidthTexture,
        ref<Texture> pPrevLinearZAndNormalTexture
    );
    void computeFilteredMoments(RenderContext* pRenderContext);
    void computeAtrousDecomposition(RenderContext* pRenderContext, ref<Texture> pAlbedoTexture);

    bool mBuffersNeedClear = false;

    // SVGF parameters
    bool mFilterEnabled = true;
    int32_t mFilterIterations = 4;
    int32_t mFeedbackTap = 1;
    float mVarainceEpsilon = 1e-4f;
    float mPhiColor = 10.0f;
    float mPhiNormal = 128.0f;
    float mAlpha = 0.05f;
    float mMomentsAlpha = 0.2f;

    // SVGF passes
    ref<FullScreenPass> mpPackLinearZAndNormal;
    ref<FullScreenPass> mpReprojection;
    ref<FullScreenPass> mpFilterMoments;
    ref<FullScreenPass> mpAtrous;
    ref<FullScreenPass> mpFinalModulate;

    // Intermediate framebuffers
    ref<Fbo> mpPingPongFbo[2];
    ref<Fbo> mpLinearZAndNormalFbo;
    ref<Fbo> mpFilteredPastFbo;
    ref<Fbo> mpCurReprojFbo;
    ref<Fbo> mpPrevReprojFbo;
    ref<Fbo> mpFilteredIlluminationFbo;
    ref<Fbo> mpFinalFbo;
};
