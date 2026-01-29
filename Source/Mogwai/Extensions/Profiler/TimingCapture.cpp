#include "Falcor.h"
#include "TimingCapture.h"

namespace Mogwai
{
    namespace
    {
        const std::string kScriptVar = "timingCapture";
        const std::string kCaptureFrameTime = "captureFrameTime";
    }

    MOGWAI_EXTENSION(TimingCapture);

    TimingCapture::UniquePtr TimingCapture::create(Renderer* pRenderer)
    {
        return UniquePtr(new TimingCapture(pRenderer));
    }

    void TimingCapture::registerScriptBindings(pybind11::module& m)
    {
        using namespace pybind11::literals;

        pybind11::class_<TimingCapture> timingCapture(m, "TimingCapture");

        // Members
        timingCapture.def(kCaptureFrameTime.c_str(), &TimingCapture::captureFrameTime, "path"_a);
    }

    std::string TimingCapture::getScriptVar() const
    {
        return kScriptVar;
    }

    void TimingCapture::beginFrame(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
    {
        recordPreviousFrameTime();
    }

    void TimingCapture::captureFrameTime(std::filesystem::path path)
    {
        if (mFrameTimeFile.is_open())
            mFrameTimeFile.close();

        if (!path.empty())
        {
            if (std::filesystem::exists(path))
            {
                logWarning("Frame times in file '{}' will be overwritten.", path);
            }

            mFrameTimeFile.open(path, std::ofstream::trunc);
            if (!mFrameTimeFile.is_open())
            {
                logError("Failed to open file '{}' for writing. Ignoring call.", path);
            }
        }
    }

    void TimingCapture::recordPreviousFrameTime()
    {
        if (!mFrameTimeFile.is_open()) return;

        // The FrameRate object is updated at the start of each frame, the first valid time is available on the second frame.
        auto& frameRate = mpRenderer->getFrameRate();
        if (frameRate.getFrameCount() > 1)
            mFrameTimeFile << frameRate.getLastFrameTime() << std::endl;
    }
}
