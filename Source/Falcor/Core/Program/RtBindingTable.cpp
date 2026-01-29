#include "RtBindingTable.h"
#include "Core/Error.h"

namespace Falcor
{
namespace
{
// Define API limitations.
// See https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
const uint32_t kMaxMissCount = (1 << 16);
const uint32_t kMaxRayTypeCount = (1 << 4);
} // namespace

ref<RtBindingTable> RtBindingTable::create(uint32_t missCount, uint32_t rayTypeCount, uint32_t geometryCount)
{
    return ref<RtBindingTable>(new RtBindingTable(missCount, rayTypeCount, geometryCount));
}

RtBindingTable::RtBindingTable(uint32_t missCount, uint32_t rayTypeCount, uint32_t geometryCount)
    : mMissCount(missCount), mRayTypeCount(rayTypeCount), mGeometryCount(geometryCount)
{
    FALCOR_CHECK(missCount <= kMaxMissCount, "'missCount' exceeds the maximum supported ({})", kMaxMissCount);
    FALCOR_CHECK(rayTypeCount <= kMaxRayTypeCount, "'rayTypeCount' exceeds the maximum supported ({})", kMaxRayTypeCount);
    size_t recordCount = 1ull + missCount + rayTypeCount * geometryCount;
    FALCOR_CHECK(recordCount <= std::numeric_limits<uint32_t>::max(), "Raytracing binding table is too large");

    // Create the binding table. All entries will be assigned a null shader initially.
    mShaderTable.resize(recordCount);
}

void RtBindingTable::setRayGen(ShaderID shaderID)
{
    mShaderTable[0] = shaderID;
}

void RtBindingTable::setMiss(uint32_t missIndex, ShaderID shaderID)
{
    FALCOR_CHECK(missIndex < mMissCount, "'missIndex' is out of range");
    mShaderTable[getMissOffset(missIndex)] = shaderID;
}

void RtBindingTable::setHitGroup(uint32_t rayType, uint32_t geometryID, ShaderID shaderID)
{
    FALCOR_CHECK(rayType < mRayTypeCount, "'rayType' is out of range");
    FALCOR_CHECK(geometryID < mGeometryCount, "'geometryID' is out of range");
    mShaderTable[getHitGroupOffset(rayType, geometryID)] = shaderID;
}

void RtBindingTable::setHitGroup(uint32_t rayType, const std::vector<uint32_t>& geometryIDs, ShaderID shaderID)
{
    for (uint32_t geometryID : geometryIDs)
    {
        setHitGroup(rayType, geometryID, shaderID);
    }
}

void RtBindingTable::setHitGroup(uint32_t rayType, const std::vector<GlobalGeometryID>& geometryIDs, ShaderID shaderID)
{
    static_assert(std::is_same_v<GlobalGeometryID::IntType, uint32_t>);
    for (GlobalGeometryID geometryID : geometryIDs)
    {
        setHitGroup(rayType, geometryID.get(), shaderID);
    }
}
} // namespace Falcor
