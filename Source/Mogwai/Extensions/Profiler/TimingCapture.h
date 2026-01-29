#pragma once
#include "../../Mogwai.h"
#include <fstream>

namespace Mogwai
{
    class TimingCapture : public Extension
    {
    public:
        virtual ~TimingCapture() = default;
        static UniquePtr create(Renderer* pRenderer);

        virtual void beginFrame(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
        virtual void registerScriptBindings(pybind11::module& m) override;
        virtual std::string getScriptVar() const override;

    protected:
        TimingCapture(Renderer *pRenderer) : Extension(pRenderer, "Timing Capture") {}

        /** Start capture frame times to file, or end capture if path is empty.
        */
        void captureFrameTime(std::filesystem::path path);
        void recordPreviousFrameTime();

        std::ofstream   mFrameTimeFile;     ///< Frame times are appended to this file when it's open.
    };
}
