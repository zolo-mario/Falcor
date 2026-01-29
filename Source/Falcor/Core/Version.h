#pragma once

#include "Macros.h"

#include <string>

#define FALCOR_MAJOR_VERSION 8
#define FALCOR_MINOR_VERSION 0

namespace Falcor
{
FALCOR_API const std::string& getVersionString();
FALCOR_API const std::string& getLongVersionString();
} // namespace Falcor
