#include "MeshletCulling.h"
#include <unordered_map>

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, MeshletCulling>();
}

namespace
{
    const char kCullingShader[] = "RenderPasses/MeshletCulling/MeshletCulling.cs.slang";
    const char kRasterShader[] = "RenderPasses/MeshletCulling/MeshletRaster.slang";
    const char kVisualizeShader[] = "RenderPasses/MeshletCulling/MeshletVBufferVisualize.cs.slang";

    constexpr size_t kMaxVerticesPerMeshlet = 64;
    constexpr size_t kMaxTrianglesPerMeshlet = 124;
    constexpr float kConeWeight = 0.5f;

    // RenderPass I/O
    const char kOutputColor[] = "color";
    const char kOutputVBuffer[] = "vbuffer";
    const char kOutputDepth[] = "depth";
}

MeshletCulling::MeshletCulling(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    for (const auto& [key, value] : props)
    {
        if (key == "enableCulling") mEnableCulling = value;
        else if (key == "outputVBuffer") mOutputVBuffer = value;
        else if (key == "visualizeMode") mVisualizeMode = static_cast<VisualizeMode>((uint32_t)value);
        else logWarning("MeshletCulling: Unknown property '{}'", key);
    }

    ProgramDesc cullingDesc;
    cullingDesc.addShaderLibrary(kCullingShader).csEntry("main");
    mpCullingPass = ComputePass::create(mpDevice, cullingDesc);
}

ref<MeshletCulling> MeshletCulling::create(ref<Device> pDevice, const Properties& props)
{
    return make_ref<MeshletCulling>(pDevice, props);
}

Properties MeshletCulling::getProperties() const
{
    Properties props;
    props["enableCulling"] = mEnableCulling;
    props["outputVBuffer"] = mOutputVBuffer;
    props["visualizeMode"] = static_cast<uint32_t>(mVisualizeMode);
    return props;
}

RenderPassReflection MeshletCulling::reflect(const CompileData& compileData)
{
    RenderPassReflection r;

    // Output: Visualized color
    r.addOutput(kOutputColor, "Visualized output")
        .bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::UnorderedAccess)
        .format(ResourceFormat::RGBA8UnormSrgb);

    // Output: Visibility Buffer (R32_UINT)
    r.addOutput(kOutputVBuffer, "Visibility buffer (packed MeshletID + PrimitiveID)")
        .bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource)
        .format(ResourceFormat::R32Uint);

    // Output: Depth
    r.addOutput(kOutputDepth, "Depth buffer")
        .bindFlags(ResourceBindFlags::DepthStencil)
        .format(ResourceFormat::D32Float);

    return r;
}

void MeshletCulling::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;

    mBuildResult.clear();
    mpMeshletBuffer = nullptr;
    mpMeshletVertices = nullptr;
    mpMeshletTriangles = nullptr;
    mpMeshletPositions = nullptr;
    mpFlattenedIndexBuffer = nullptr;
    mpVisibleMeshletIDs = nullptr;
    mpIndirectArgsBuffer = nullptr;
    mStats = {};

    if (!mpScene || mpScene->getMeshCount() == 0)
    {
        logInfo("MeshletCulling: No scene or empty scene");
        return;
    }

    buildMeshlets();
    createGpuBuffers();
    initRasterPass();
    initVisualizePass();
}

void MeshletCulling::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    mFrameCount++;

    // Get outputs
    auto pOutputColor = renderData.getTexture(kOutputColor);
    auto pOutputVBuffer = renderData.getTexture(kOutputVBuffer);
    auto pOutputDepth = renderData.getTexture(kOutputDepth);

    if (!pOutputColor || !mpScene || mBuildResult.meshlets.empty())
    {
        if (pOutputColor)
            pRenderContext->clearTexture(pOutputColor.get(), float4(0.1f, 0.1f, 0.15f, 1.0f));
        return;
    }

    // Clear outputs
    pRenderContext->clearUAV(pOutputVBuffer->getUAV().get(), uint4(0xFFFFFFFF));
    pRenderContext->clearDsv(pOutputDepth->getDSV().get(), 1.0f, 0);

    // Update frustum
    const auto* pCamera = mpScene->getCamera().get();
    if (pCamera && !mFreezeCulling)
    {
        updateFrustumData(pCamera);
    }

    // 1. GPU Culling
    pRenderContext->clearUAVCounter(mpVisibleMeshletIDs, 0);
    if (mEnableCulling)
    {
        runCullingPass(pRenderContext);
    }
    else
    {
        // Need to fill IndirectArgs even when culling is disabled
        runCullingPass(pRenderContext); // Shader internally skips culling based on flag
    }

    // 2. Rasterization -> VBuffer
    runRasterPass(pRenderContext, pOutputVBuffer, pOutputDepth);

    // 3. Visualize VBuffer -> Color
    runVisualizePass(pRenderContext, pOutputVBuffer, pOutputColor);

    // Statistics
    if (mReadbackStats)
    {
        pRenderContext->submit(true);
        mStats.visibleMeshlets = mpVisibleMeshletIDs->getUAVCounter()->getElement<uint32_t>(0);
    }
}

void MeshletCulling::renderUI(Gui::Widgets& widget)
{
    widget.checkbox("Enable Culling", mEnableCulling);
    widget.tooltip("Toggle frustum culling on/off");

    widget.checkbox("Freeze Culling", mFreezeCulling);
    widget.tooltip("Freeze current culling results");

    widget.checkbox("Show Stats", mShowStats);

    if (widget.checkbox("Readback Stats (Slow)", mReadbackStats))
    {
        if (mReadbackStats)
            logWarning("MeshletCulling: Readback enabled - this will cause GPU stalls!");
    }

    widget.separator();

    widget.dropdown("Visualize Mode", mVisualizeMode);

    widget.separator();

    if (mShowStats)
    {
        widget.text("Meshlet Statistics:");
        widget.text(fmt::format("  Total Meshlets: {}", mStats.totalMeshlets));
        widget.text(fmt::format("  Visible Meshlets: {}", mStats.visibleMeshlets));
        widget.text(fmt::format("  Total Triangles: {}", mStats.totalTriangles));

        if (mStats.totalMeshlets > 0)
        {
            float cullRate = 1.0f - (float)mStats.visibleMeshlets / (float)mStats.totalMeshlets;
            widget.text(fmt::format("  Cull Rate: {:.1f}%", cullRate * 100.0f));
        }
    }
}

void MeshletCulling::buildMeshlets()
{
    if (!mpScene || mpScene->getMeshCount() == 0)
        return;

    const auto& meshDesc = mpScene->getMesh(MeshID{0});
    uint32_t vertexCount = meshDesc.vertexCount;
    uint32_t triangleCount = meshDesc.getTriangleCount();

    logInfo("MeshletCulling: Building meshlets for mesh with {} vertices, {} triangles",
            vertexCount, triangleCount);

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

    mpScene->getMeshVerticesAndIndices(MeshID{0}, buffers);

    std::vector<uint3> indices(triangleCount);
    std::vector<float3> positions(vertexCount);

    buffers["triangleIndices"]->getBlob(indices.data(), 0, triangleCount * sizeof(uint3));
    buffers["positions"]->getBlob(positions.data(), 0, vertexCount * sizeof(float3));

    std::vector<uint32_t> flatIndices(triangleCount * 3);
    for (uint32_t i = 0; i < triangleCount; i++)
    {
        flatIndices[i * 3 + 0] = indices[i].x;
        flatIndices[i * 3 + 1] = indices[i].y;
        flatIndices[i * 3 + 2] = indices[i].z;
    }

    uint32_t indexCount = (uint32_t)flatIndices.size();
    std::vector<uint32_t> remap(indexCount);

    size_t optimizedVertexCount = meshopt_generateVertexRemap(
        remap.data(),
        flatIndices.data(),
        indexCount,
        positions.data(),
        vertexCount,
        sizeof(float3)
    );

    std::vector<uint32_t> remappedIndices(indexCount);
    meshopt_remapIndexBuffer(remappedIndices.data(), flatIndices.data(), indexCount, remap.data());

    mBuildResult.remappedPositions.resize(optimizedVertexCount);
    meshopt_remapVertexBuffer(
        mBuildResult.remappedPositions.data(),
        positions.data(),
        vertexCount,
        sizeof(float3),
        remap.data()
    );
    mBuildResult.remappedVertexCount = optimizedVertexCount;

    logInfo("MeshletCulling: Vertex optimization: {} -> {} vertices",
            vertexCount, optimizedVertexCount);

    size_t maxMeshlets = meshopt_buildMeshletsBound(
        remappedIndices.size(), kMaxVerticesPerMeshlet, kMaxTrianglesPerMeshlet);

    mBuildResult.meshlets.resize(maxMeshlets);
    mBuildResult.meshletVertices.resize(maxMeshlets * kMaxVerticesPerMeshlet);
    mBuildResult.meshletTriangles.resize(maxMeshlets * kMaxTrianglesPerMeshlet * 3);

    size_t meshletCount = meshopt_buildMeshlets(
        mBuildResult.meshlets.data(),
        mBuildResult.meshletVertices.data(),
        mBuildResult.meshletTriangles.data(),
        remappedIndices.data(),
        remappedIndices.size(),
        reinterpret_cast<const float*>(mBuildResult.remappedPositions.data()),
        optimizedVertexCount,
        sizeof(float3),
        kMaxVerticesPerMeshlet,
        kMaxTrianglesPerMeshlet,
        kConeWeight
    );

    mBuildResult.meshlets.resize(meshletCount);
    if (meshletCount > 0)
    {
        const auto& lastMeshlet = mBuildResult.meshlets.back();
        mBuildResult.meshletVertices.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
        mBuildResult.meshletTriangles.resize(
            lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));
    }

    mBuildResult.meshletBounds.resize(meshletCount);
    for (size_t i = 0; i < meshletCount; i++)
    {
        const auto& m = mBuildResult.meshlets[i];
        meshopt_Bounds bounds = meshopt_computeMeshletBounds(
            &mBuildResult.meshletVertices[m.vertex_offset],
            &mBuildResult.meshletTriangles[m.triangle_offset],
            m.triangle_count,
            reinterpret_cast<const float*>(mBuildResult.remappedPositions.data()),
            optimizedVertexCount,
            sizeof(float3)
        );
        mBuildResult.meshletBounds[i] = float4(
            bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius);
    }

    mBuildResult.flattenedIndices.clear();
    mBuildResult.flattenedIndices.reserve(triangleCount * 3);

    for (size_t mi = 0; mi < meshletCount; mi++)
    {
        const auto& m = mBuildResult.meshlets[mi];

        for (uint32_t t = 0; t < m.triangle_count; t++)
        {
            uint32_t triOffset = m.triangle_offset + t * 3;
            uint8_t lv0 = mBuildResult.meshletTriangles[triOffset + 0];
            uint8_t lv1 = mBuildResult.meshletTriangles[triOffset + 1];
            uint8_t lv2 = mBuildResult.meshletTriangles[triOffset + 2];

            uint32_t gv0 = mBuildResult.meshletVertices[m.vertex_offset + lv0];
            uint32_t gv1 = mBuildResult.meshletVertices[m.vertex_offset + lv1];
            uint32_t gv2 = mBuildResult.meshletVertices[m.vertex_offset + lv2];

            mBuildResult.flattenedIndices.push_back(gv0);
            mBuildResult.flattenedIndices.push_back(gv1);
            mBuildResult.flattenedIndices.push_back(gv2);
        }
    }

    mStats.totalMeshlets = (uint32_t)meshletCount;
    mStats.totalTriangles = 0;
    for (const auto& m : mBuildResult.meshlets)
        mStats.totalTriangles += m.triangle_count;

    logInfo("MeshletCulling: Built {} meshlets, {} triangles, {} flattened indices",
            meshletCount, mStats.totalTriangles, mBuildResult.flattenedIndices.size());
}

void MeshletCulling::createGpuBuffers()
{
    if (mBuildResult.meshlets.empty())
        return;

    uint32_t meshletCount = (uint32_t)mBuildResult.meshlets.size();

    std::vector<GpuMeshlet> gpuMeshlets(meshletCount);
    uint32_t currentIndexStart = 0;

    for (size_t i = 0; i < meshletCount; i++)
    {
        const auto& m = mBuildResult.meshlets[i];
        const auto& bounds = mBuildResult.meshletBounds[i];

        gpuMeshlets[i].vertexOffset = m.vertex_offset;
        gpuMeshlets[i].triangleOffset = m.triangle_offset;
        gpuMeshlets[i].vertexCount = m.vertex_count;
        gpuMeshlets[i].triangleCount = m.triangle_count;
        gpuMeshlets[i].boundCenter = float3(bounds.x, bounds.y, bounds.z);
        gpuMeshlets[i].boundRadius = bounds.w;
        gpuMeshlets[i].indexStart = currentIndexStart;
        gpuMeshlets[i].indexCount = m.triangle_count * 3;
        gpuMeshlets[i].meshID = 0;
        gpuMeshlets[i]._pad0 = 0;

        currentIndexStart += m.triangle_count * 3;
    }

    mpMeshletBuffer = mpDevice->createStructuredBuffer(
        sizeof(GpuMeshlet), meshletCount,
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        gpuMeshlets.data()
    );

    mpMeshletVertices = mpDevice->createStructuredBuffer(
        sizeof(uint32_t), (uint32_t)mBuildResult.meshletVertices.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mBuildResult.meshletVertices.data()
    );

    size_t packedSize = (mBuildResult.meshletTriangles.size() + 3) / 4;
    std::vector<uint32_t> packedTriangles(packedSize, 0);
    for (size_t i = 0; i < mBuildResult.meshletTriangles.size(); i++)
    {
        uint32_t byteIndex = i % 4;
        uint32_t uintIndex = i / 4;
        packedTriangles[uintIndex] |= (uint32_t)mBuildResult.meshletTriangles[i] << (byteIndex * 8);
    }

    mpMeshletTriangles = mpDevice->createStructuredBuffer(
        sizeof(uint32_t), (uint32_t)packedTriangles.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        packedTriangles.data()
    );

    mpMeshletPositions = mpDevice->createStructuredBuffer(
        sizeof(float3), (uint32_t)mBuildResult.remappedPositions.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mBuildResult.remappedPositions.data()
    );

    MeshletSceneData sceneData;
    sceneData.meshletCount = meshletCount;
    sceneData.totalTriangles = mStats.totalTriangles;
    sceneData.totalVertices = (uint32_t)mBuildResult.remappedVertexCount;
    sceneData.totalIndices = (uint32_t)mBuildResult.flattenedIndices.size();

    mpSceneDataBuffer = mpDevice->createStructuredBuffer(
        sizeof(MeshletSceneData), 1,
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        &sceneData
    );

    mpFlattenedIndexBuffer = mpDevice->createBuffer(
        mBuildResult.flattenedIndices.size() * sizeof(uint32_t),
        ResourceBindFlags::Index | ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mBuildResult.flattenedIndices.data()
    );

    mpVisibleMeshletIDs = mpDevice->createStructuredBuffer(
        sizeof(uint32_t), meshletCount,
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal,
        nullptr,
        true
    );

    mpIndirectArgsBuffer = mpDevice->createStructuredBuffer(
        sizeof(DrawIndexedIndirectArgs), meshletCount,
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess | ResourceBindFlags::IndirectArg,
        MemoryType::DeviceLocal
    );

    mpFrustumBuffer = mpDevice->createBuffer(
        sizeof(FrustumData),
        ResourceBindFlags::Constant,
        MemoryType::Upload
    );

    logInfo("MeshletCulling: GPU buffers created successfully");
}

void MeshletCulling::updateFrustumData(const Camera* pCamera)
{
    if (!pCamera)
        return;

    const auto& viewProj = pCamera->getViewProjMatrix();

    mCachedFrustum.planes[0] = float4(
        viewProj[0][3] + viewProj[0][0],
        viewProj[1][3] + viewProj[1][0],
        viewProj[2][3] + viewProj[2][0],
        viewProj[3][3] + viewProj[3][0]
    );

    mCachedFrustum.planes[1] = float4(
        viewProj[0][3] - viewProj[0][0],
        viewProj[1][3] - viewProj[1][0],
        viewProj[2][3] - viewProj[2][0],
        viewProj[3][3] - viewProj[3][0]
    );

    mCachedFrustum.planes[2] = float4(
        viewProj[0][3] + viewProj[0][1],
        viewProj[1][3] + viewProj[1][1],
        viewProj[2][3] + viewProj[2][1],
        viewProj[3][3] + viewProj[3][1]
    );

    mCachedFrustum.planes[3] = float4(
        viewProj[0][3] - viewProj[0][1],
        viewProj[1][3] - viewProj[1][1],
        viewProj[2][3] - viewProj[2][1],
        viewProj[3][3] - viewProj[3][1]
    );

    mCachedFrustum.planes[4] = float4(
        viewProj[0][2],
        viewProj[1][2],
        viewProj[2][2],
        viewProj[3][2]
    );

    mCachedFrustum.planes[5] = float4(
        viewProj[0][3] - viewProj[0][2],
        viewProj[1][3] - viewProj[1][2],
        viewProj[2][3] - viewProj[2][2],
        viewProj[3][3] - viewProj[3][2]
    );

    for (int i = 0; i < 6; i++)
    {
        float3 planeNormal = float3(mCachedFrustum.planes[i].x, mCachedFrustum.planes[i].y, mCachedFrustum.planes[i].z);
        float len = math::length(planeNormal);
        if (len > 0.0001f)
            mCachedFrustum.planes[i] /= len;
    }

    mCachedFrustum.cameraPos = pCamera->getPosition();

    mpFrustumBuffer->setBlob(&mCachedFrustum, 0, sizeof(FrustumData));
}

void MeshletCulling::runCullingPass(RenderContext* pRenderContext)
{
    FALCOR_PROFILE(pRenderContext, "MeshletCulling");

    auto vars = mpCullingPass->getRootVar();

    vars["gMeshlets"] = mpMeshletBuffer;
    vars["gSceneData"] = mpSceneDataBuffer;
    vars["gFrustum"] = mpFrustumBuffer;
    vars["gVisibleMeshletIDs"] = mpVisibleMeshletIDs;
    vars["gIndirectArgs"] = mpIndirectArgsBuffer;

    uint32_t meshletCount = mStats.totalMeshlets;
    uint32_t threadGroupSize = 64;
    uint32_t dispatchX = (meshletCount + threadGroupSize - 1) / threadGroupSize;

    mpCullingPass->execute(pRenderContext, dispatchX, 1, 1);

    pRenderContext->uavBarrier(mpVisibleMeshletIDs.get());
    pRenderContext->uavBarrier(mpIndirectArgsBuffer.get());
}

void MeshletCulling::initRasterPass()
{
    if (mpGraphicsState) return;

    // Program with defines
    DefineList defines;
    defines.add("VBUFFER_OUTPUT_MODE", "1"); // Output VBuffer

    auto pProgram = Program::createGraphics(mpDevice, kRasterShader, "vsMain", "psMain", defines);

    // State
    mpGraphicsState = GraphicsState::create(mpDevice);
    mpGraphicsState->setProgram(pProgram);

    // Rasterizer
    RasterizerState::Desc rsDesc;
    rsDesc.setCullMode(RasterizerState::CullMode::Back);
    mpRasterState = RasterizerState::create(rsDesc);
    mpGraphicsState->setRasterizerState(mpRasterState);

    // Depth Stencil
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthEnabled(true);
    dsDesc.setDepthWriteMask(true);
    dsDesc.setDepthFunc(ComparisonFunc::Less);
    mpDepthStencilState = DepthStencilState::create(dsDesc);
    mpGraphicsState->setDepthStencilState(mpDepthStencilState);

    // Vars
    mpGraphicsVars = ProgramVars::create(mpDevice, pProgram->getReflector());

    // FBO
    mpFbo = Fbo::create(mpDevice);
}

void MeshletCulling::runRasterPass(
    RenderContext* pContext,
    const ref<Texture>& pVBuffer,
    const ref<Texture>& pDepth)
{
    FALCOR_PROFILE(pContext, "MeshletRaster");

    // FBO
    mpFbo->attachColorTarget(pVBuffer, 0);
    mpFbo->attachDepthStencilTarget(pDepth);
    mpGraphicsState->setFbo(mpFbo);

    // Vars
    auto var = mpGraphicsVars->getRootVar();
    var["gPositions"] = mpMeshletPositions;
    var["gVisibleMeshletIDs"] = mpVisibleMeshletIDs;
    var["gMeshlets"] = mpMeshletBuffer;

    // Camera
    const auto* pCamera = mpScene->getCamera().get();
    var["PerFrameCB"]["gViewProj"] = pCamera->getViewProjMatrix();
    var["PerFrameCB"]["gFrameCount"] = mFrameCount;

    // VAO (no VB, use IB)
    auto pVao = Vao::create(Vao::Topology::TriangleList, nullptr, Vao::BufferVec{}, mpFlattenedIndexBuffer, ResourceFormat::R32Uint);
    mpGraphicsState->setVao(pVao);

    // Barriers
    pContext->resourceBarrier(mpIndirectArgsBuffer.get(), Resource::State::IndirectArg);
    pContext->resourceBarrier(mpVisibleMeshletIDs.get(), Resource::State::ShaderResource);

    // Indirect Draw
    pContext->drawIndexedIndirect(
        mpGraphicsState.get(),
        mpGraphicsVars.get(),
        mStats.totalMeshlets,
        mpIndirectArgsBuffer.get(),
        0,
        mpVisibleMeshletIDs->getUAVCounter().get(),
        0
    );
}

void MeshletCulling::initVisualizePass()
{
    if (mpVisualizePass) return;

    ProgramDesc desc;
    desc.addShaderLibrary(kVisualizeShader).csEntry("main");
    mpVisualizePass = ComputePass::create(mpDevice, desc);
}

void MeshletCulling::runVisualizePass(
    RenderContext* pContext,
    const ref<Texture>& pVBuffer,
    const ref<Texture>& pOutput)
{
    FALCOR_PROFILE(pContext, "VBufferVisualize");

    auto var = mpVisualizePass->getRootVar();
    var["gVBuffer"] = pVBuffer;
    var["gOutput"] = pOutput;
    var["VisualizeCB"]["gFrameDim"] = uint2(pOutput->getWidth(), pOutput->getHeight());
    var["VisualizeCB"]["gVisualizeMode"] = static_cast<uint32_t>(mVisualizeMode);
    var["VisualizeCB"]["gFrameCount"] = mFrameCount;

    uint2 threadGroups = div_round_up(uint2(pOutput->getWidth(), pOutput->getHeight()), uint2(16, 16));
    mpVisualizePass->execute(pContext, threadGroups.x, threadGroups.y, 1);
}
