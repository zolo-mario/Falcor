#pragma once
#include "../../Mogwai.h"

namespace Mogwai
{
    class CaptureTrigger : public Extension
    {
    public:
        virtual ~CaptureTrigger() {};

        virtual void beginFrame(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override final;
        virtual void endFrame(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override final;
        virtual bool hasWindow() const override { return true; }
        virtual bool isWindowShown() const override { return mShowUI; }
        virtual void toggleWindow() override { mShowUI = !mShowUI; }
        virtual void registerScriptBindings(pybind11::module& m) override;
        virtual void activeGraphChanged(RenderGraph* pNewGraph, RenderGraph* pPrevGraph) override;
    protected:
        CaptureTrigger(Renderer* pRenderer, const std::string& name) : Extension(pRenderer, name) {}

        using Range = std::pair<uint64_t, uint64_t>; // Start frame and count

        virtual void beginRange(RenderGraph* pGraph, const Range& r) {};
        virtual void triggerFrame(RenderContext* pCtx, RenderGraph* pGraph, uint64_t frameID) {};
        virtual void endRange(RenderGraph* pGraph, const Range& r) {};

        void addRange(const RenderGraph* pGraph, uint64_t startFrame, uint64_t count);
        void reset(const RenderGraph* pGraph = nullptr);
        void renderBaseUI(Gui::Window& w);

        void setOutputDirectory(const std::filesystem::path& path);
        const std::filesystem::path& getOutputDirectory() const { return mOutputDir; }

        void setBaseFilename(const std::string& baseFilename);
        const std::string& getBaseFilename() const { return mBaseFilename; }

        std::string getScript(const std::string& var) const override;
        std::filesystem::path getOutputPath() const;
        std::string getOutputNamePrefix(const std::string& output) const;

        using range_vec = std::vector<Range>;
        std::unordered_map<const RenderGraph*, range_vec> mGraphRanges;

        std::string mBaseFilename = "Mogwai";
        std::filesystem::path mOutputDir = ".";
        bool mShowUI = false;

        struct
        {
            RenderGraph* pGraph = nullptr;
            Range range;
        } mCurrent;
    };
}
