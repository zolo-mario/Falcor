#pragma once
#include "CpuTimer.h"
#include "Core/Macros.h"
#include "Core/API/GpuTimer.h"
#include "Core/API/Fence.h"
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#if FALCOR_ENABLE_PROFILER
#include <tracy/Tracy.hpp>
#if FALCOR_HAS_D3D12
#include <tracy/TracyD3D12.hpp>
#endif
#endif

namespace Falcor
{
class RenderContext;

/**
 * Container class for CPU/GPU profiling.
 * This class uses the most accurately available CPU and GPU timers to profile given events.
 * It automatically creates event hierarchies based on the order and nesting of the calls made.
 * This class uses a double-buffering scheme for GPU profiling to avoid GPU stalls.
 * ProfilerEvent is a wrapper class which together with scoping can simplify event profiling.
 */
class FALCOR_API Profiler
{
public:
    enum class Flags
    {
        None = 0x0,
        Internal = 0x1,
        Pix = 0x2,

        Default = Internal | Pix
    };

    struct Stats
    {
        float min;
        float max;
        float mean;
        float stdDev;

        static Stats compute(const float* data, size_t len);
    };

    class Event
    {
    public:
        const std::string getName() const { return mName; }

        float getCpuTime() const { return mCpuTime; }
        float getGpuTime() const { return mGpuTime; }

        float getCpuTimeAverage() const { return mCpuTimeAverage; }
        float getGpuTimeAverage() const { return mGpuTimeAverage; }

        Stats computeCpuTimeStats() const;
        Stats computeGpuTimeStats() const;

        void resetStats();

    private:
        Event(const std::string& name);

        void start(Profiler& profiler, uint32_t frameIndex);
        void end(uint32_t frameIndex);
        void endFrame(uint32_t frameIndex);

        std::string mName; ///< Nested event name.

        float mCpuTime = 0.0; ///< CPU time (previous frame).
        float mGpuTime = 0.0; ///< GPU time (previous frame).

        float mCpuTimeAverage = -1.f; ///< Average CPU time (negative value to signify invalid).
        float mGpuTimeAverage = -1.f; ///< Average GPU time (negative value to signify invalid).

        std::vector<float> mCpuTimeHistory; ///< CPU time history (round-robin, used for computing stats).
        std::vector<float> mGpuTimeHistory; ///< GPU time history (round-robin, used for computing stats).
        size_t mHistoryWriteIndex = 0;      ///< History write index.
        size_t mHistorySize = 0;            ///< History size.

        uint32_t mTriggered = 0; ///< Keeping track of nested calls to start().

        struct FrameData
        {
            CpuTimer::TimePoint cpuStartTime; ///< Last event CPU start time.
            float cpuTotalTime = 0.0;         ///< Total accumulated CPU time.

            std::vector<ref<GpuTimer>> pTimers; ///< Pool of GPU timers.
            size_t currentTimer = 0;            ///< Next GPU timer to use from the pool.
            GpuTimer* pActiveTimer = nullptr;   ///< Currently active GPU timer.

            bool valid = false; ///< True when frame data is valid (after begin/end cycle).
        };
        FrameData mFrameData[2]; ///< Double-buffered frame data to avoid GPU flushes.

        friend class Profiler;
    };

    class Capture
    {
    public:
        struct Lane
        {
            std::string name;
            Stats stats;
            std::vector<float> records;
        };

        Capture(size_t reservedEvents, size_t reservedFrames);

        size_t getFrameCount() const { return mFrameCount; }
        const std::vector<Lane>& getLanes() const { return mLanes; }

        std::string toJsonString() const;
        void writeToFile(const std::filesystem::path& path) const;

    private:
        void captureEvents(const std::vector<Event*>& events);
        void finalize();

        size_t mReservedFrames = 0;
        size_t mFrameCount = 0;
        std::vector<Event*> mEvents;
        std::vector<Lane> mLanes;
        bool mFinalized = false;

        friend class Profiler;
    };

    /**
     * Constructor.
     */
    Profiler(ref<Device> pDevice);

    // Delete copy constructor and copy assignment operator (non-copyable due to unique_ptr members)
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    const Device* getDevice() const { return mpDevice.get(); }

    /**
     * Check if the profiler is enabled.
     * @return Returns true if the profiler is enabled.
     */
    bool isEnabled() const { return mEnabled; }

    /**
     * Enable/disable the profiler.
     * @param[in] enabled True to enable the profiler.
     */
    void setEnabled(bool enabled) { mEnabled = enabled; }

    /**
     * Check if the profiler is paused.
     * @return Returns true if the profiler is paused.
     */
    bool isPaused() const { return mPaused; }

    /**
     * Pause/resume the profiler.
     * @param[in] paused True to pause the profiler.
     */
    void setPaused(bool paused) { mPaused = paused; }

    /**
     * Start profile capture.
     * @param[in] reservedFrames Number of frames to reserve memory for.
     */
    void startCapture(size_t reservedFrames = 1024);

    /**
     * End profile capture.
     * @return Returns the captured data.
     */
    std::shared_ptr<Capture> endCapture();

    /**
     * Check if the profiler is capturing.
     * @return Return true if the profiler is capturing.
     */
    bool isCapturing() const;

    /**
     * Finish profiling for the entire frame.
     * Note: Must be called once at the end of each frame.
     */
    void endFrame(RenderContext* pRenderContext);

    /**
     * Start profiling a new event and update the events hierarchies.
     * @param[in] pRenderContext Render context for measuring GPU time.
     * @param[in] name The event name.
     * @param[in] flags The event flags.
     */
    void startEvent(RenderContext* pRenderContext, const std::string& name, Flags flags = Flags::Default);

    /**
     * Finish profiling a new event and update the events hierarchies.
     * @param[in] pRenderContext Render context for measuring GPU time.
     * @param[in] name The event name.
     * @param[in] flags The event flags.
     */
    void endEvent(RenderContext* pRenderContext, const std::string& name, Flags flags = Flags::Default);

    /**
     * Get the event, or create a new one if the event does not yet exist.
     * This is a public interface to facilitate more complicated construction of event names and finegrained control over the profiled
     * region.
     * @param[in] name The event name.
     * @return Returns a pointer to the event.
     */
    Event* getEvent(const std::string& name);

    /**
     * Get the profiler events (previous frame).
     */
    const std::vector<Event*>& getEvents() const { return mLastFrameEvents; }

    /**
     * Reset profiler stats at the next call to endFrame().
     */
    void resetStats();

    void breakStrongReferenceToDevice();

private:
    /**
     * Create a new event.
     * @param[in] name The event name.
     * @return Returns the new event.
     */
    Event* createEvent(const std::string& name);

    /**
     * Find an event that was previously created.
     * @param[in] name The event name.
     * @return Returns the event or nullptr if none was found.
     */
    Event* findEvent(const std::string& name);

    BreakableReference<Device> mpDevice;

    bool mEnabled = true;
    bool mPaused = false;

    std::unordered_map<std::string, std::shared_ptr<Event>> mEvents; ///< Events by name.
    std::vector<Event*> mCurrentFrameEvents;                         ///< Events registered for current frame.
    std::vector<Event*> mLastFrameEvents;                            ///< Events from last frame.
    std::string mCurrentEventName;                                   ///< Current nested event name.
    uint32_t mCurrentLevel = 0;                                      ///< Current nesting level.
    uint32_t mFrameIndex = 0;                                        ///< Current frame index.
    bool mPendingReset = false;                                      ///< Reset profiler stats at the next call to endFrame().

    std::shared_ptr<Capture> mpCapture; ///< Currently active capture.

    ref<Fence> mpFence;
    uint64_t mFenceValue = uint64_t(-1);

#if FALCOR_ENABLE_PROFILER && FALCOR_HAS_D3D12
    // Stack to manage nested Tracy D3D12 GPU zones
    std::vector<std::unique_ptr<tracy::D3D12ZoneScope>> mTracyD3D12ZoneStack;
#endif
};

FALCOR_ENUM_CLASS_OPERATORS(Profiler::Flags);

/**
 * Helper class for starting and ending profiling events using RAII.
 * The constructor and destructor call Profiler::StartEvent() and Profiler::EndEvent().
 * The FALCOR_PROFILE macro wraps creation of local ProfilerEvent objects when profiling is enabled,
 * and does nothing when profiling is disabled, so should be used instead of directly creating ProfilerEvent objects.
 */
class FALCOR_API ScopedProfilerEvent
{
public:
    ScopedProfilerEvent(RenderContext* pRenderContext, const std::string& name, Profiler::Flags flags = Profiler::Flags::Default);
    ~ScopedProfilerEvent();

private:
    RenderContext* mpRenderContext;
    const std::string mName;
    Profiler::Flags mFlags;
};
} // namespace Falcor

#if FALCOR_ENABLE_PROFILER
#define FALCOR_PROFILE(_pRenderContext, _name) \
    ZoneScoped; \
    { \
        std::string __tracy_name_str = _name; \
        ZoneName(__tracy_name_str.c_str(), __tracy_name_str.size()); \
    } \
    Falcor::ScopedProfilerEvent FALCOR_CONCAT_STRINGS(_profileEvent, __LINE__)(_pRenderContext, _name)
#define FALCOR_PROFILE_DYNAMIC(_pRenderContext, _name) \
    ZoneScoped; \
    { \
        const std::string& __tracy_name_ref = _name; \
        ZoneName(__tracy_name_ref.c_str(), __tracy_name_ref.size()); \
    } \
    Falcor::ScopedProfilerEvent FALCOR_CONCAT_STRINGS(_profileEvent, __LINE__)(_pRenderContext, _name)
#define FALCOR_PROFILE_CUSTOM(_pRenderContext, _name, _flags) \
    ZoneScoped; \
    { \
        std::string __tracy_name_str = _name; \
        ZoneName(__tracy_name_str.c_str(), __tracy_name_str.size()); \
    } \
    Falcor::ScopedProfilerEvent FALCOR_CONCAT_STRINGS(_profileEvent, __LINE__)(_pRenderContext, _name, _flags)
#else
#define FALCOR_PROFILE(_pRenderContext, _name)
#define FALCOR_PROFILE_DYNAMIC(_pRenderContext, _name)
#define FALCOR_PROFILE_CUSTOM(_pRenderContext, _name, _flags)
#endif
