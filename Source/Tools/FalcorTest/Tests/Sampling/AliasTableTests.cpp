#include "Testing/UnitTest.h"
#include "Utils/Sampling/AliasTable.h"

#include <hypothesis/hypothesis.h>

#include <iostream>

namespace Falcor
{
namespace
{
void testAliasTable(GPUUnitTestContext& ctx, uint32_t N, std::vector<float> specificWeights = {})
{
    ref<Device> pDevice = ctx.getDevice();

    std::mt19937 rng;
    std::uniform_real_distribution<float> uniform;

    // Use specificed weights or generate pseudo-random weights.
    std::vector<float> weights(N);
    for (uint32_t i = 0; i < N; ++i)
        weights[i] = i < specificWeights.size() ? specificWeights[i] : uniform(rng);

    // Add a few zero weights.
    if (N >= 100)
    {
        for (uint32_t i = 0; i < N / 100; ++i)
            weights[(size_t)(uniform(rng) * N)] = 0.f;
    }

    // Create alias table.
    AliasTable aliasTable(pDevice, weights, rng);

    // Compute weight sum.
    double weightSum = 0.0;
    for (const auto& weight : weights)
        weightSum += weight;

    EXPECT_EQ(aliasTable.getCount(), weights.size());
    EXPECT_EQ(aliasTable.getWeightSum(), weightSum);

    // Test sampling the alias table.
    {
        const uint32_t samplesPerWeight = 10000;
        uint32_t resultCount = N * samplesPerWeight;
        uint32_t randomCount = resultCount * 2;

        // Create uniform random numbers as input.
        std::vector<float> random(randomCount);
        std::generate(random.begin(), random.end(), [&uniform, &rng]() { return uniform(rng); });

        // Setup and run GPU test.
        ctx.createProgram("Tests/Sampling/AliasTableTests.cs.slang", "testAliasTableSample");
        ctx.allocateStructuredBuffer("sampleResult", resultCount);
        ctx.allocateStructuredBuffer("random", randomCount, random.data());
        aliasTable.bindShaderData(ctx["CB"]["aliasTable"]);
        ctx["CB"]["resultCount"] = resultCount;
        ctx.runProgram(resultCount);

        // Build histogram.
        std::vector<uint32_t> histogram(N, 0);
        std::vector<uint32_t> result = ctx.readBuffer<uint32_t>("sampleResult");
        for (uint32_t i = 0; i < resultCount; ++i)
        {
            uint32_t item = result[i];
            EXPECT(item >= 0u && item < N);
            histogram[item]++;
        }

        // Verify histogram using a chi-square test.
        std::vector<double> expFrequencies(N);
        std::vector<double> obsFrequencies(N);
        for (uint32_t i = 0; i < N; ++i)
        {
            expFrequencies[i] = (weights[i] / weightSum) * N * samplesPerWeight;
            obsFrequencies[i] = (double)histogram[i];
        }

        // Special case for N == 1
        if (N == 1)
        {
            EXPECT(histogram[0] == samplesPerWeight);
        }
        else
        {
            const auto& [success, report] =
                hypothesis::chi2_test(N, obsFrequencies.data(), expFrequencies.data(), N * samplesPerWeight, 5, 0.1);
            if (!success)
                std::cout << report << std::endl;
            EXPECT(success);
        }
    }

    // Test getting weights.
    {
        uint32_t resultCount = N;

        // Setup and run GPU test.
        ctx.createProgram("Tests/Sampling/AliasTableTests.cs.slang", "testAliasTableWeight");
        ctx.allocateStructuredBuffer("weightResult", resultCount);
        aliasTable.bindShaderData(ctx["CB"]["aliasTable"]);
        ctx["CB"]["resultCount"] = resultCount;
        ctx.runProgram(resultCount);

        // Verify weights.
        std::vector<float> weightResult = ctx.readBuffer<float>("weightResult");
        for (uint32_t i = 0; i < resultCount; ++i)
        {
            EXPECT_EQ(weightResult[i], weights[i]);
        }
    }
}
} // namespace

GPU_TEST(AliasTable)
{
    testAliasTable(ctx, 1, {1.f});
    testAliasTable(ctx, 2, {1.f, 2.f});
    testAliasTable(ctx, 100);
    testAliasTable(ctx, 1000);
}
} // namespace Falcor
