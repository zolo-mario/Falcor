#pragma once

#include "Formats.h"
#include "Resource.h"

#include <slang-gfx.h>

namespace Falcor
{
gfx::Format getGFXFormat(ResourceFormat format);

gfx::ResourceState getGFXResourceState(Resource::State state);
void getGFXResourceState(ResourceBindFlags flags, gfx::ResourceState& defaultState, gfx::ResourceStateSet& allowedStates);
} // namespace Falcor
