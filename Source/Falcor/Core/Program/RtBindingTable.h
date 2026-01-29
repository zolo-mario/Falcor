#pragma once
#include "Program.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Scene/SceneIDs.h"
#include <memory>
#include <vector>

namespace Falcor
{
/**
 * This class describes the binding of ray tracing shaders for raygen/miss/hits.
 *
 * There is always exactly one raygen shader, which is the entry point for the program.
 * The class also describes the mapping from TraceRay() miss index to miss shader,
 * and the mapping from (rayType, geometryID) to which hit group to execute.
 *
 * The user is responsible for creating a binding table for use with a particular
 * Program and Scene before creating an RtProgramVars object.
 */
class FALCOR_API RtBindingTable : public Object
{
    FALCOR_OBJECT(RtBindingTable)
public:
    using ShaderID = ProgramDesc::ShaderID;

    /**
     * Create a new binding table.
     * @param[in] missCount Number of miss shaders.
     * @param[in] rayTypeCount Number of ray types.
     * @param[in] geometryCount Number of geometries.
     * @return A new object, or throws an exception on error.
     */
    static ref<RtBindingTable> create(uint32_t missCount, uint32_t rayTypeCount, uint32_t geometryCount);

    /**
     * Set the raygen shader ID.
     * @param[in] shaderID The shader ID in the program.
     */
    void setRayGen(ShaderID shaderID);

    /**
     * Set a miss shader ID.
     * @param[in] missIndex The miss index.
     * @param[in] shaderID The shader ID in the program.
     */
    void setMiss(uint32_t missIndex, ShaderID shaderID);

    /**
     * Set a hit group shader ID.
     * @param[in] rayType The ray type.
     * @param[in] geometryID The geometry ID in the scene.
     * @param[in] shaderID The shader ID in the program.
     */
    void setHitGroup(uint32_t rayType, uint32_t geometryID, ShaderID shaderID);
    void setHitGroup(uint32_t rayType, GlobalGeometryID geometryID, ShaderID shaderID)
    {
        setHitGroup(rayType, geometryID.get(), std::move(shaderID));
    }

    /**
     * Set hit group shader ID.
     * @param[in] rayType The ray type.
     * @param[in] geometryIDs The geometry IDs in the scene.
     * @param[in] shaderID The shader ID in the program.
     */
    void setHitGroup(uint32_t rayType, const std::vector<uint32_t>& geometryIDs, ShaderID shaderID);
    void setHitGroup(uint32_t rayType, const std::vector<GlobalGeometryID>& geometryIDs, ShaderID shaderID);

    /**
     * Get the raygen shader ID.
     * @return The shader ID in the program.
     */
    ShaderID getRayGen() const { return mShaderTable[0]; }

    /**
     * Get the miss shader ID.
     * @param[in] missIndex The miss index.
     * @return The shader ID in the program.
     */
    ShaderID getMiss(uint32_t missIndex) const { return mShaderTable[getMissOffset(missIndex)]; }

    /**
     * Get a hit group shader ID.
     * @param[in] rayType The ray type.
     * @param[in] geometryID The geometry ID in the scene.
     * @return The shader ID in the program.
     */
    ShaderID getHitGroup(uint32_t rayType, uint32_t geometryID) const { return mShaderTable[getHitGroupOffset(rayType, geometryID)]; }

    uint32_t getMissCount() const { return mMissCount; }
    uint32_t getRayTypeCount() const { return mRayTypeCount; }
    uint32_t getGeometryCount() const { return mGeometryCount; }

private:
    RtBindingTable() = delete;
    RtBindingTable(const RtBindingTable&) = delete;
    RtBindingTable& operator=(const RtBindingTable&) = delete;

    RtBindingTable(uint32_t missCount, uint32_t rayTypeCount, uint32_t geometryCount);

    uint32_t getMissOffset(uint32_t missIndex) const
    {
        FALCOR_ASSERT(missIndex < mMissCount);
        uint32_t offset = 1 + missIndex;
        FALCOR_ASSERT(offset < mShaderTable.size());
        return offset;
    }
    uint32_t getHitGroupOffset(uint32_t rayType, uint32_t geometryID) const
    {
        FALCOR_ASSERT(rayType < mRayTypeCount && geometryID < mGeometryCount);
        uint32_t offset = 1 + mMissCount + geometryID * mRayTypeCount + rayType;
        FALCOR_ASSERT(offset < mShaderTable.size());
        return offset;
    }

    // Internal state
    uint32_t mMissCount = 0;     ///< Number of miss shaders.
    uint32_t mRayTypeCount = 0;  ///< Number of ray types.
    uint32_t mGeometryCount = 0; ///< Number of geometries in the scene.

    std::vector<ShaderID> mShaderTable; ///< Table of all shader IDs. The default value is a null entry (no shader).
};
} // namespace Falcor
