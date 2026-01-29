#pragma once
#include "../../Mogwai.h"
#include "CaptureTrigger.h"
#include "Utils/Image/ImageProcessing.h"

namespace Mogwai
{
    class FrameCapture : public CaptureTrigger
    {
    public:
        static UniquePtr create(Renderer* pRenderer);
        virtual void renderUI(Gui* pGui) override;
        virtual void registerScriptBindings(pybind11::module& m) override;
        virtual std::string getScriptVar() const override;
        virtual std::string getScript(const std::string& var) const override;
        virtual void triggerFrame(RenderContext* pRenderContext, RenderGraph* pGraph, uint64_t frameID) override;
        void capture();

    private:
        FrameCapture(Renderer* pRenderer);

        using uint64_vec = std::vector<uint64_t>;
        void addFrames(const RenderGraph* pGraph, const uint64_vec& frames);
        void addFrames(const std::string& graphName, const uint64_vec& frames);
        std::string graphFramesStr(const RenderGraph* pGraph);
        void captureOutput(RenderContext* pRenderContext, RenderGraph* pGraph, const uint32_t outputIndex);

        bool mCaptureAllOutputs = false;
        std::unique_ptr<ImageProcessing> mpImageProcessing;
    };
}
