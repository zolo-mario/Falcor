#pragma once
#include "Falcor.h"

#include <nvsdk_ngx_defs.h>

#include <filesystem>

// Forward declarations from NGX library.
struct NVSDK_NGX_Parameter;
struct NVSDK_NGX_Handle;

namespace Falcor
{
/**
 * This is a wrapper around the NGX functionality for DLSS.
 * It is seperated to provide focus to the calls specific to NGX for code sample purposes.
 */
class NGXWrapper
{
public:
    struct OptimalSettings
    {
        float sharpness;
        uint2 optimalRenderSize;
        uint2 minRenderSize;
        uint2 maxRenderSize;
    };

    /// Constructor. Throws an exception if unable to initialize NGX.
    NGXWrapper(ref<Device> pDevice, const std::filesystem::path& applicationDataPath, const std::filesystem::path& featureSearchPath);
    ~NGXWrapper();

    /// Query optimal DLSS settings for a given resolution and performance/quality profile.
    OptimalSettings queryOptimalSettings(uint2 displaySize, NVSDK_NGX_PerfQuality_Value perfQuality) const;

    /// Initialize DLSS. Throws an exception if unable to initialize.
    void initializeDLSS(
        RenderContext* pRenderContext,
        uint2 maxRenderSize,
        uint2 displayOutSize,
        Texture* pTarget,
        bool isContentHDR,
        bool depthInverted,
        NVSDK_NGX_PerfQuality_Value perfQuality = NVSDK_NGX_PerfQuality_Value_MaxPerf
    );

    /// Release DLSS.
    void releaseDLSS();

    /// Checks if DLSS is initialized.
    bool isDLSSInitialized() const { return mpFeature != nullptr; }

    //// Evaluate DLSS.
    bool evaluateDLSS(
        RenderContext* pRenderContext,
        Texture* pUnresolvedColor,
        Texture* pResolvedColor,
        Texture* pMotionVectors,
        Texture* pDepth,
        Texture* pExposure,
        bool resetAccumulation = false,
        float sharpness = 0.0f,
        float2 jitterOffset = {0.f, 0.f},
        float2 motionVectorScale = {1.f, 1.f}
    ) const;

private:
    void initializeNGX(const std::filesystem::path& applicationDataPath, const std::filesystem::path& featureSearchPath);
    void shutdownNGX();

    ref<Device> mpDevice;
    bool mInitialized = false;

    NVSDK_NGX_Parameter* mpParameters = nullptr;
    NVSDK_NGX_Handle* mpFeature = nullptr;
};
} // namespace Falcor
