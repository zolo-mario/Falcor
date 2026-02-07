#include "VBufferMeshletRaster.h"
#include "Scene/HitInfo.h"
#include "RenderGraph/RenderPassStandardFlags.h"
#include "RenderGraph/RenderPassHelpers.h"
#include <unordered_map>

namespace
{
    const char kRasterShader[] = "RenderPasses/GBuffer/VBuffer/MeshletRaster.cs.slang";
    const char kResolveShader[] = "RenderPasses/GBuffer/VBuffer/MeshletResolve.cs.slang";

    constexpr size_t kMaxVerticesPerMeshlet = 64;
    constexpr size_t kMaxTrianglesPerMeshlet = 124;
    constexpr float kConeWeight = 0.5f;

    // RenderPass I/O
    const std::string kVBufferName = "vbuffer";
    const std::string kVBufferDesc = "V-buffer in packed format (indices + barycentrics)";

    // Additional output channels (matching VBufferRT)
    const ChannelList kVBufferExtraChannels = {
        { "viewW",  "gViewW",        "View direction in world space", true /* optional */, ResourceFormat::RGBA32Float },
        { "depth",  "gDepth",        "Depth buffer (NDC)",  true /* optional */, ResourceFormat::R32Float  },
        { "mvec",   "gMotionVector", "Motion vector",       true /* optional */, ResourceFormat::RG32Float },
    };
}

VBufferMeshletRaster::VBufferMeshletRaster(ref<Device> pDevice, const Properties& props)
    : GBufferBase(pDevice)
{
    parseProperties(props);

    // Compute passes will be created in setScene() when we have the scene
}

Properties VBufferMeshletRaster::getProperties() const
{
    return GBufferBase::getProperties();
}

RenderPassReflection VBufferMeshletRaster::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, mFixedOutputSize, compileData.defaultTexDims);

    // Add the required output. This always exists.
    reflector.addOutput(kVBufferName, kVBufferDesc)
        .bindFlags(ResourceBindFlags::UnorderedAccess)
        .format(mVBufferFormat)
        .texture2D(sz.x, sz.y);

    // Add all the other outputs.
    addRenderPassOutputs(reflector, kVBufferExtraChannels, ResourceBindFlags::UnorderedAccess, sz);

    return reflector;
}

void VBufferMeshletRaster::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    GBufferBase::setScene(pRenderContext, pScene);

    // Clear previous meshlet data
    mBuildResult.clear();
    mpMeshletBuffer = nullptr;
    mpMeshletVertices = nullptr;
    mpMeshletTriangles = nullptr;
    mpVisBufferDepth = nullptr;
    mpVisBufferPayload = nullptr;
    mMeshletCount = 0;
    mpRasterPass = nullptr;
    mpResolvePass = nullptr;

    if (!mpScene || mpScene->getGeometryInstanceCount() == 0)
        return;

    // Create compute passes with scene shader modules
    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kRasterShader).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        DefineList defines;
        defines.add(mpScene->getSceneDefines());

        mpRasterPass = ComputePass::create(mpDevice, desc, defines, true);
    }

    {
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kResolveShader).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        DefineList defines;
        defines.add(mpScene->getSceneDefines());

        mpResolvePass = ComputePass::create(mpDevice, desc, defines, true);
    }

    buildMeshlets();
    createGpuBuffers();

    logInfo("VBufferMeshletRaster: Built {} meshlets for {} instances",
            mMeshletCount, mpScene->getGeometryInstanceCount());
}

void VBufferMeshletRaster::buildMeshlets()
{
    if (!mpScene)
        return;

    uint32_t totalPrimitiveOffset = 0;

    // Build meshlets per-instance
    for (uint32_t instanceIdx = 0; instanceIdx < mpScene->getGeometryInstanceCount(); instanceIdx++)
    {
        const auto& instance = mpScene->getGeometryInstance(instanceIdx);
        MeshID meshID{instance.geometryID};
        const auto& meshDesc = mpScene->getMesh(meshID);

        uint32_t vertexCount = meshDesc.vertexCount;
        uint32_t triangleCount = meshDesc.getTriangleCount();

        if (triangleCount == 0)
            continue;

        // Get mesh data
        std::map<std::string, ref<Buffer>> buffers;
        buffers["triangleIndices"] = mpDevice->createStructuredBuffer(
            sizeof(uint3), triangleCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal
        );
        buffers["positions"] = mpDevice->createStructuredBuffer(
            sizeof(float3), vertexCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal
        );
        buffers["texcrds"] = mpDevice->createStructuredBuffer(
            sizeof(float2), vertexCount,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal
        );

        mpScene->getMeshVerticesAndIndices(meshID, buffers);

        std::vector<uint3> indices(triangleCount);
        std::vector<float3> positions(vertexCount);

        buffers["triangleIndices"]->getBlob(indices.data(), 0, triangleCount * sizeof(uint3));
        buffers["positions"]->getBlob(positions.data(), 0, vertexCount * sizeof(float3));

        // Flatten indices
        std::vector<uint32_t> flatIndices(triangleCount * 3);
        for (uint32_t i = 0; i < triangleCount; i++)
        {
            flatIndices[i * 3 + 0] = indices[i].x;
            flatIndices[i * 3 + 1] = indices[i].y;
            flatIndices[i * 3 + 2] = indices[i].z;
        }

        // Build meshlets for this instance
        size_t maxMeshlets = meshopt_buildMeshletsBound(
            flatIndices.size(), kMaxVerticesPerMeshlet, kMaxTrianglesPerMeshlet);

        std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
        std::vector<uint32_t> meshletVertices(maxMeshlets * kMaxVerticesPerMeshlet);
        std::vector<uint8_t> meshletTriangles(maxMeshlets * kMaxTrianglesPerMeshlet * 3);

        size_t meshletCount = meshopt_buildMeshlets(
            meshlets.data(),
            meshletVertices.data(),
            meshletTriangles.data(),
            flatIndices.data(),
            flatIndices.size(),
            reinterpret_cast<const float*>(positions.data()),
            vertexCount,
            sizeof(float3),
            kMaxVerticesPerMeshlet,
            kMaxTrianglesPerMeshlet,
            kConeWeight
        );

        meshlets.resize(meshletCount);

        // Resize buffers to actual size
        if (meshletCount > 0)
        {
            const auto& lastMeshlet = meshlets.back();
            meshletVertices.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
            meshletTriangles.resize(
                lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));
        }

        // Store meshlet data with instance information
        uint32_t baseVertexOffset = (uint32_t)mBuildResult.meshletVertices.size();
        uint32_t baseTriangleOffset = (uint32_t)mBuildResult.meshletTriangles.size();

        for (size_t i = 0; i < meshletCount; i++)
        {
            const auto& m = meshlets[i];

            GpuMeshlet gpuMeshlet;
            gpuMeshlet.vertexOffset = baseVertexOffset + m.vertex_offset;
            gpuMeshlet.triangleOffset = baseTriangleOffset + m.triangle_offset;
            gpuMeshlet.vertexCount = m.vertex_count;
            gpuMeshlet.triangleCount = m.triangle_count;
            gpuMeshlet.instanceID = instanceIdx;
            gpuMeshlet.primitiveOffset = totalPrimitiveOffset;
            gpuMeshlet.meshID = meshID.get();

            // Compute bounds in world space
            meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                &meshletVertices[m.vertex_offset],
                &meshletTriangles[m.triangle_offset],
                m.triangle_count,
                reinterpret_cast<const float*>(positions.data()),
                vertexCount,
                sizeof(float3)
            );

            // Transform bounds to world space
            const auto& globalMatrices = mpScene->getAnimationController()->getGlobalMatrices();
            float4x4 worldMatrix = globalMatrices[instance.globalMatrixID];
            float3 center = float3(bounds.center[0], bounds.center[1], bounds.center[2]);
            float4 worldCenter4 = mul(worldMatrix, float4(center, 1.0f));
            float3 worldCenter = float3(worldCenter4.x, worldCenter4.y, worldCenter4.z);

            gpuMeshlet.boundCenter = worldCenter;
            gpuMeshlet.boundRadius = bounds.radius;

            mBuildResult.gpuMeshlets.push_back(gpuMeshlet);
        }

        // Append vertex and triangle data
        mBuildResult.meshletVertices.insert(
            mBuildResult.meshletVertices.end(),
            meshletVertices.begin(),
            meshletVertices.end()
        );
        mBuildResult.meshletTriangles.insert(
            mBuildResult.meshletTriangles.end(),
            meshletTriangles.begin(),
            meshletTriangles.end()
        );

        totalPrimitiveOffset += triangleCount;
    }

    mMeshletCount = (uint32_t)mBuildResult.gpuMeshlets.size();
}

void VBufferMeshletRaster::createGpuBuffers()
{
    if (mBuildResult.gpuMeshlets.empty())
        return;

    // Create meshlet buffer
    mpMeshletBuffer = mpDevice->createStructuredBuffer(
        sizeof(GpuMeshlet),
        (uint32_t)mBuildResult.gpuMeshlets.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mBuildResult.gpuMeshlets.data()
    );

    // Create meshlet vertices buffer
    mpMeshletVertices = mpDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (uint32_t)mBuildResult.meshletVertices.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mBuildResult.meshletVertices.data()
    );

    // Create meshlet triangles buffer (convert uint8_t to uint32_t for compatibility)
    std::vector<uint32_t> trianglesUint32(mBuildResult.meshletTriangles.size());
    for (size_t i = 0; i < mBuildResult.meshletTriangles.size(); i++)
    {
        trianglesUint32[i] = static_cast<uint32_t>(mBuildResult.meshletTriangles[i]);
    }
    mpMeshletTriangles = mpDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (uint32_t)trianglesUint32.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        trianglesUint32.data()
    );
}

void VBufferMeshletRaster::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Call base class execute
    GBufferBase::execute(pRenderContext, renderData);

    // Get outputs
    auto pVBuffer = renderData.getTexture(kVBufferName);
    FALCOR_ASSERT(pVBuffer);

    // Update frame dimension
    updateFrameDim(uint2(pVBuffer->getWidth(), pVBuffer->getHeight()));

    if (!mpScene || mMeshletCount == 0 || !mpRasterPass || !mpResolvePass)
    {
        // Clear outputs and return
        pRenderContext->clearUAV(pVBuffer->getUAV().get(), uint4(0));
        clearRenderPassChannels(pRenderContext, kVBufferExtraChannels, renderData);
        return;
    }

    // Create or resize visibility buffers if needed
    if (!mpVisBufferDepth || mpVisBufferDepth->getWidth() != mFrameDim.x || mpVisBufferDepth->getHeight() != mFrameDim.y)
    {
        mpVisBufferDepth = mpDevice->createTexture2D(
            mFrameDim.x, mFrameDim.y,
            ResourceFormat::R32Uint,
            1, 1,
            nullptr,
            ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource
        );
        mpVisBufferPayload = mpDevice->createTexture2D(
            mFrameDim.x, mFrameDim.y,
            ResourceFormat::R32Uint,
            1, 1,
            nullptr,
            ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource
        );
    }

    // Clear visibility buffers
    // Clear depth to max value (far plane) so any geometry will be closer
    pRenderContext->clearUAV(mpVisBufferDepth->getUAV().get(), uint4(0xFFFFFFFF));
    pRenderContext->clearUAV(mpVisBufferPayload->getUAV().get(), uint4(0));

    // Pass 1: Raster meshlets into visibility buffer
    {
        auto var = mpRasterPass->getRootVar();
        var["gVisBufferDepth"] = mpVisBufferDepth;
        var["gVisBufferPayload"] = mpVisBufferPayload;
        var["gMeshlets"] = mpMeshletBuffer;
        var["gMeshletVertices"] = mpMeshletVertices;
        var["gMeshletTriangles"] = mpMeshletTriangles;
        mpScene->bindShaderDataForRaytracing(pRenderContext, var["gScene"]);
        var["CB"]["gFrameDim"] = mFrameDim;
        var["CB"]["gMeshletCount"] = mMeshletCount;

        mpRasterPass->execute(pRenderContext, uint3(mMeshletCount, 1, 1));
    }

    // Pass 2: Resolve visibility buffer to PackedHitInfo
    {
        auto var = mpResolvePass->getRootVar();
        var["gVisBufferDepth"] = mpVisBufferDepth;
        var["gVisBufferPayload"] = mpVisBufferPayload;
        var["gVBuffer"] = pVBuffer;
        mpScene->bindShaderDataForRaytracing(pRenderContext, var["gScene"]);
        var["CB"]["gFrameDim"] = mFrameDim;

        // Bind optional outputs
        auto pViewW = getOutput(renderData, "viewW");
        var["gViewW"] = pViewW;

        auto pDepth = getOutput(renderData, "depth");
        var["gDepth"] = pDepth;

        auto pMotionVector = getOutput(renderData, "mvec");
        var["gMotionVector"] = pMotionVector;

        uint32_t groupsX = (mFrameDim.x + 15) / 16;
        uint32_t groupsY = (mFrameDim.y + 15) / 16;
        mpResolvePass->execute(pRenderContext, uint3(groupsX, groupsY, 1));
    }

    mFrameCount++;
}

void VBufferMeshletRaster::renderUI(Gui::Widgets& widget)
{
    GBufferBase::renderUI(widget);

    widget.separator();
    widget.text("Meshlet Statistics:");
    widget.text(fmt::format("  Total Meshlets: {}", mMeshletCount));
}
