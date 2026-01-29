#pragma once
#include "Macros.h"

namespace Falcor
{
/**
 * Flags indicating what hot-reloadable resources have changed
 */
enum class HotReloadFlags
{
    None = 0,    ///< Nothing. Here just for completeness
    Program = 1, ///< Programs (shaders)
};

FALCOR_ENUM_CLASS_OPERATORS(HotReloadFlags);
} // namespace Falcor
