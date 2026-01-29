#include "GBuffer.h"

namespace Falcor
{
// Update 'mtlData' channel format if size changes.
// Note: Currently, we only store the first 8 bytes of the material header.
static_assert(sizeof(MaterialHeader) == 16);
} // namespace Falcor

// List of primary GBuffer channels. These correspond to the render targets
// used in the GBufferRaster pixel shader. Note that channel order should
// correspond to SV_TARGET index order.
const ChannelList GBuffer::kGBufferChannels = {
    // clang-format off
    { "posW",           "gPosW",            "Position in world space",                           true /* optional */, ResourceFormat::RGBA32Float },
    { "normW",          "gNormW",           "Shading normal in world space",                     true /* optional */, ResourceFormat::RGBA32Float },
    { "tangentW",       "gTangentW",        "Shading tangent in world space (xyz) and sign (w)", true /* optional */, ResourceFormat::RGBA32Float },
    { "faceNormalW",    "gFaceNormalW",     "Face normal in world space",                        true /* optional */, ResourceFormat::RGBA32Float },
    { "texC",           "gTexC",            "Texture coordinate",                                true /* optional */, ResourceFormat::RG32Float   },
    { "texGrads",       "gTexGrads",        "Texture gradients (ddx, ddy)",                      true /* optional */, ResourceFormat::RGBA16Float },
    { "mvec",           "gMotionVector",    "Motion vector",                                     true /* optional */, ResourceFormat::RG32Float   },
    { "mtlData",        "gMaterialData",    "Material data (ID, header.x, header.y, lobes)",     true /* optional */, ResourceFormat::RGBA32Uint  },
    // clang-format on
};

GBuffer::GBuffer(ref<Device> pDevice) : GBufferBase(pDevice)
{
    // The list of primary GBuffer channels should contain 8 entries, corresponding to the 8 render targets.
    FALCOR_ASSERT(kGBufferChannels.size() == 8);
}
