#pragma once
#include <vector>
#include <cstdint>

namespace Falcor
{

struct MaterialParamLayoutEntry
{
    const char* name;
    const char* pythonName;
    uint32_t size;
    uint32_t offset;
};

using MaterialParamLayout = std::vector<MaterialParamLayoutEntry>;

} // namespace Falcor
