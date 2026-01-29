#pragma once
#include "../GBufferBase.h"
#include "RenderGraph/RenderPassHelpers.h"

using namespace Falcor;

/**
 * Base class for the different G-buffer passes.
 */
class GBuffer : public GBufferBase
{
protected:
    GBuffer(ref<Device> pDevice);

    // Constants used in derived classes
    static const ChannelList kGBufferChannels;
};
