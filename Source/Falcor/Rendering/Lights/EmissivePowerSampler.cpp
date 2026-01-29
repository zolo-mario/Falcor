#include "EmissivePowerSampler.h"
#include "Utils/Timing/Profiler.h"
#include <algorithm>

namespace Falcor
{
    bool EmissivePowerSampler::update(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection)
    {
        FALCOR_PROFILE(pRenderContext, "EmissivePowerSampler::update");

        bool samplerChanged = false;

        if (mpLightCollection != pLightCollection)
        {
            setLightCollection(std::move(pLightCollection));
            mNeedsRebuild = true;
        }

        // Check if light collection has changed.
        if (mLightCollectionUpdateFlags != ILightCollection::UpdateFlags::None)
        {
            mNeedsRebuild = true;
            mLightCollectionUpdateFlags = ILightCollection::UpdateFlags::None;
        }

        // Rebuild if necessary
        if (mNeedsRebuild)
        {
            // Get global list of emissive triangles.
            FALCOR_ASSERT(mpLightCollection);
            const auto& triangles = mpLightCollection->getMeshLightTriangles(pRenderContext);

            const size_t numTris = triangles.size();
            std::vector<float> weights(numTris);
            for (size_t i = 0; i < numTris; i++) weights[i] = triangles[i].flux;

            mTriangleTable = generateAliasTable(std::move(weights));

            mNeedsRebuild = false;
            samplerChanged = true;
        }

        return samplerChanged;
    }

    void EmissivePowerSampler::bindShaderData(const ShaderVar& var) const
    {
        FALCOR_ASSERT(var.isValid());

        var["_emissivePower"]["invWeightsSum"] = 1.0f / mTriangleTable.weightSum;
        var["_emissivePower"]["triangleAliasTable"] = mTriangleTable.fullTable;
    }

    EmissivePowerSampler::EmissivePowerSampler(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection)
        : EmissiveLightSampler(EmissiveLightSamplerType::Power, std::move(pLightCollection))
    {
    }

    EmissivePowerSampler::AliasTable EmissivePowerSampler::generateAliasTable(std::vector<float> weights)
    {
        uint32_t N = uint32_t(weights.size());
        std::uniform_int_distribution<uint32_t> rngDist;

        double sum = 0.0f;
        for (float f : weights)
        {
            sum += f;
        }
        for (float& f : weights)
        {
            f *= N / float(sum);
        }

        std::vector<uint32_t> permutation(N);
        for (uint32_t i = 0; i < N; ++i)
        {
            permutation[i] = i;
        }
        std::sort(permutation.begin(), permutation.end(), [&](uint32_t a, uint32_t b) { return weights[a] < weights[b]; });

        std::vector<float> thresholds(N);
        std::vector<uint32_t> redirect(N);
        std::vector<uint2> merged(N);
        std::vector<uint2> fullTable(N);

        uint32_t head = 0;
        uint32_t tail = N - 1;

        while (head != tail)
        {
            int i = permutation[head];
            int j = permutation[tail];

            thresholds[i] = weights[i];
            redirect[i] = j;
            weights[j] -= 1.0f - weights[i];

            if (head == tail - 1)
            {
                thresholds[j] = 1.0f;
                redirect[j] = j;
                break;
            }
            else if (weights[j] < 1.0f)
            {
                std::swap(permutation[head], permutation[tail]);
                tail--;
            }
            else
            {
                head++;
            }
        }

        for (uint32_t i = 0; i < N; ++i)
        {
            permutation[i] = i;
        }

        for (uint32_t i = 0; i < N; ++i)
        {
            uint32_t dst = i + (rngDist(mAliasTableRng) % (N - i));
            std::swap(thresholds[i], thresholds[dst]);
            std::swap(redirect[i], redirect[dst]);
            std::swap(permutation[i], permutation[dst]);
        }

        for (uint32_t i = 0; i < N; ++i)
        {
            merged[i] = uint2(redirect[i], permutation[i]);

            // Pack 16-bit threshold (i.e., a half float) plus 2x 24-bit table entries
            uint32_t prob = (uint32_t(f32tof16(thresholds[i])) << 16u);
            uint2 lowPrec = uint2(redirect[i] & 0xFFFFFFu, permutation[i] & 0xFFFFFFu);
            uint2 mergedEntry = uint2(prob | ((lowPrec.x >> 8u) & 0xFFFFu), ((lowPrec.x & 0xFFu) << 24u) | lowPrec.y);
            fullTable[i] = mergedEntry;
        }

        AliasTable result
        {
            float(sum),
            N,
            mpDevice->createTypedBuffer<uint2>(N),
        };

        result.fullTable->setBlob(&fullTable[0], 0, N * sizeof(uint2));

        return result;
    }
}
