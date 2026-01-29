#pragma once
#include "Core/Macros.h"
#include <cstdint>

namespace Falcor
{
/**
 * Flags to indicate what have changed since last frame.
 * One or more flags can be OR'ed together.
 */
enum class RenderPassRefreshFlags : uint32_t
{
    None = 0x0,
    LightingChanged = 0x1,      ///< Lighting has changed.
    RenderOptionsChanged = 0x2, ///< Options that affect the rendering have changed.
};

/**
 * The refresh flags above are passed to RenderPass::execute() via a
 * field with this name in the dictionary.
 */
static const char kRenderPassRefreshFlags[] = "_refreshFlags";

/**
 * First available preudorandom number generator dimension.
 */
static const char kRenderPassPRNGDimension[] = "_prngDimension";

/**
 * Adjust shading normals on primary hits.
 */
static const char kRenderPassGBufferAdjustShadingNormals[] = "_gbufferAdjustShadingNormals";

FALCOR_ENUM_CLASS_OPERATORS(RenderPassRefreshFlags);
} // namespace Falcor
