#pragma once
#include "Clock.h"
#include "Core/Macros.h"
#include <algorithm>
#include <string>
#include <vector>

namespace Falcor
{
/**
 * Helper class for calculating framerate.
 */
class FALCOR_API FrameRate
{
public:
    FrameRate()
    {
        mFrameTimes.resize(kFrameWindow);
        reset();
    }

    /**
     * Resets the FPS.
     * After this call it will appear as if the application had just started. Useful in cases a new scene is loaded, since it will display a
     * more accurate FPS.
     */
    void reset()
    {
        mFrameCount = 0;
        mClock.setTime(0).tick();
    }

    /**
     * Tick the timer.
     * It is assumed that this is called once per frame, since this frequency is assumed when calculating FPS.
     */
    void newFrame()
    {
        mFrameCount++;
        mFrameTimes[mFrameCount % kFrameWindow] = mClock.tick().getRealTimeDelta();
        mClock.setTime(0).tick();
    }

    /**
     * Get the time in seconds it took to render a frame.
     */
    double getAverageFrameTime() const
    {
        uint64_t frames = std::min(mFrameCount, kFrameWindow);
        double time = 0;
        for (uint64_t i = 0; i < frames; i++)
            time += mFrameTimes[i];
        return time / double(frames);
    }

    /**
     * Get the time in seconds that it took to render the last frame.
     */
    double getLastFrameTime() const { return mFrameTimes[mFrameCount % kFrameWindow]; }

    /**
     * Get the frame count (= number of times newFrame() has been called).
     */
    uint64_t getFrameCount() const { return mFrameCount; }

    /**
     * Get a message with the FPS.
     */
    std::string getMsg(bool vsyncOn = false) const;

private:
    Clock mClock;
    std::vector<double> mFrameTimes;
    uint64_t mFrameCount = 0;
    static constexpr uint64_t kFrameWindow = 60;
};

inline std::string to_string(const FrameRate& fr, bool vsyncOn = false)
{
    return fr.getMsg(vsyncOn);
}
} // namespace Falcor
