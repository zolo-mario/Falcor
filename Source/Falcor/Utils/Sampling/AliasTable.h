#pragma once
#include "Core/Macros.h"
#include "Core/API/Buffer.h"
#include "Core/Program/ShaderVar.h"
#include <memory>
#include <random>

namespace Falcor
{
/**
 * Implements the alias method for sampling from a discrete probability distribution.
 */
class FALCOR_API AliasTable
{
public:
    /**
     * Create an alias table.
     * The weights don't need to be normalized to sum up to 1.
     * @param[in] pDevice GPU device.
     * @param[in] weights The weights we'd like to sample each entry proportional to.
     * @param[in] rng The random number generator to use when creating the table.
     */
    AliasTable(ref<Device> pDevice, std::vector<float> weights, std::mt19937& rng);

    /**
     * Bind the alias table data to a given shader var.
     * @param[in] var The shader variable to set the data into.
     */
    void bindShaderData(const ShaderVar& var) const;

    /**
     * Get the number of weights in the table.
     */
    uint32_t getCount() const { return mCount; }

    /**
     * Get the total sum of all weights in the table.
     */
    double getWeightSum() const { return mWeightSum; }

private:
    // Item structure for the mpItems buffer.
    struct Item
    {
        float threshold; ///< If rand() < threshold, pick indexB (else pick indexA)
        uint32_t indexA; ///< The "redirect" index, if uniform sampling would overweight indexB.
        uint32_t indexB; ///< The original / permutation index, sampled uniformly in [0...mCount-1]
        uint32_t _pad;
    };

    uint32_t mCount;       ///< Number of items in the alias table.
    double mWeightSum;     ///< Total weight of all elements used to create the alias table.
    ref<Buffer> mpItems;   ///< Buffer containing table items.
    ref<Buffer> mpWeights; ///< Buffer containing item weights.
};
} // namespace Falcor
