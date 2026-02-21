#include "D3D12RaytracingHelloWorld.h"
#include "Core/API/Device.h"
#include "Core/API/RenderContext.h"
#include "Core/Program/Program.h"
#include "Core/Program/RtBindingTable.h"
#include "Core/Program/ProgramVars.h"
#include "Utils/Math/FalcorMath.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

// Match D3D12 sample: vertices in screen space (Y down), depth 1.0
struct Vertex
{
    float v1, v2, v3;
};

D3D12RaytracingHelloWorld::D3D12RaytracingHelloWorld(SampleApp* pHost) : SampleBase(pHost)
{
    mRayGenCB.viewport = {-1.0f, -1.0f, 1.0f, 1.0f};
}

D3D12RaytracingHelloWorld::~D3D12RaytracingHelloWorld() {}

void D3D12RaytracingHelloWorld::onLoad(RenderContext* pRenderContext)
{
    if (!getDevice()->isFeatureSupported(Device::SupportedFeatures::Raytracing))
    {
        FALCOR_THROW("Device does not support raytracing!");
    }

    buildAccelerationStructures(pRenderContext);

    // Create raytracing program
    ProgramDesc rtProgDesc;
    rtProgDesc.addShaderLibrary("Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld/D3D12RaytracingHelloWorld.rt.slang");
    rtProgDesc.setMaxTraceRecursionDepth(1);
    rtProgDesc.setMaxPayloadSize(16); // float4

    ref<RtBindingTable> sbt = RtBindingTable::create(1, 1, 1);
    sbt->setRayGen(rtProgDesc.addRayGen("MyRaygenShader"));
    sbt->setMiss(0, rtProgDesc.addMiss("MyMissShader"));
    sbt->setHitGroup(0, 0, rtProgDesc.addHitGroup("MyClosestHitShader", "", ""));

    mpRaytraceProgram = Program::create(getDevice(), rtProgDesc);
    mpRtVars = RtProgramVars::create(getDevice(), mpRaytraceProgram, sbt);

    // Create output texture
    uint2 size = getTargetFbo() ? uint2(getTargetFbo()->getWidth(), getTargetFbo()->getHeight()) : uint2(1280, 720);
    mpRtOut = getDevice()->createTexture2D(
        size.x, size.y, ResourceFormat::RGBA8Unorm, 1, 1, nullptr,
        ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource
    );

    updateViewportConstants(size.x, size.y);

    // Flush to ensure BLAS/TLAS build completes before first raytrace
    pRenderContext->submit(true);
}

void D3D12RaytracingHelloWorld::buildAccelerationStructures(RenderContext* pRenderContext)
{
    // Match D3D12 sample geometry: single triangle in screen space
    const float depthValue = 1.0f;
    const float offset = 0.7f;
    Vertex vertices[] = {
        {0, -offset, depthValue},
        {-offset, offset, depthValue},
        {offset, offset, depthValue},
    };
    uint16_t indices[] = {0, 1, 2};

    mpVertexBuffer = getDevice()->createBuffer(
        sizeof(vertices),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        vertices
    );
    mpVertexBuffer->setName("D3D12RaytracingHelloWorld VertexBuffer");

    mpIndexBuffer = getDevice()->createBuffer(
        sizeof(indices),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        indices
    );
    mpIndexBuffer->setName("D3D12RaytracingHelloWorld IndexBuffer");

    pRenderContext->resourceBarrier(mpVertexBuffer.get(), Resource::State::NonPixelShader);
    pRenderContext->resourceBarrier(mpIndexBuffer.get(), Resource::State::NonPixelShader);

    // Build BLAS
    RtGeometryDesc geomDesc = {};
    geomDesc.type = RtGeometryType::Triangles;
    geomDesc.flags = RtGeometryFlags::Opaque;
    geomDesc.content.triangles.transform3x4 = 0;
    geomDesc.content.triangles.indexFormat = ResourceFormat::R16Uint;
    geomDesc.content.triangles.vertexFormat = ResourceFormat::RGB32Float;
    geomDesc.content.triangles.indexCount = 3;
    geomDesc.content.triangles.vertexCount = 3;
    geomDesc.content.triangles.indexData = mpIndexBuffer->getGpuAddress();
    geomDesc.content.triangles.vertexData = mpVertexBuffer->getGpuAddress();
    geomDesc.content.triangles.vertexStride = sizeof(Vertex);

    RtAccelerationStructureBuildInputs blasInputs = {};
    blasInputs.kind = RtAccelerationStructureKind::BottomLevel;
    blasInputs.flags = RtAccelerationStructureBuildFlags::PreferFastTrace;
    blasInputs.descCount = 1;
    blasInputs.geometryDescs = &geomDesc;

    auto blasPrebuild = RtAccelerationStructure::getPrebuildInfo(getDevice().get(), blasInputs);

    mpBlasBuffer = getDevice()->createBuffer(
        blasPrebuild.resultDataMaxSize,
        ResourceBindFlags::AccelerationStructure,
        MemoryType::DeviceLocal
    );
    mpBlasBuffer->setName("D3D12RaytracingHelloWorld BLAS");

    mpScratchBuffer = getDevice()->createBuffer(
        std::max(blasPrebuild.scratchDataSize, 256ull),
        ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal
    );
    mpScratchBuffer->setName("D3D12RaytracingHelloWorld Scratch");

    pRenderContext->uavBarrier(mpScratchBuffer.get());

    RtAccelerationStructure::Desc blasDesc;
    blasDesc.setKind(RtAccelerationStructureKind::BottomLevel);
    blasDesc.setBuffer(mpBlasBuffer, 0, blasPrebuild.resultDataMaxSize);
    mpBlas = RtAccelerationStructure::create(getDevice(), blasDesc);

    RtAccelerationStructure::BuildDesc blasBuildDesc = {};
    blasBuildDesc.inputs = blasInputs;
    blasBuildDesc.scratchData = mpScratchBuffer->getGpuAddress();
    blasBuildDesc.dest = mpBlas.get();

    pRenderContext->buildAccelerationStructure(blasBuildDesc, 0, nullptr);
    pRenderContext->uavBarrier(mpBlasBuffer.get());

    // Build TLAS
    RtInstanceDesc instanceDesc = {};
    instanceDesc.transform[0][0] = instanceDesc.transform[1][1] = instanceDesc.transform[2][2] = 1;
    instanceDesc.instanceMask = 1;
    instanceDesc.accelerationStructure = mpBlas->getGpuAddress();

    mpInstanceBuffer = getDevice()->createBuffer(
        sizeof(RtInstanceDesc),
        ResourceBindFlags::ShaderResource,
        MemoryType::Upload,
        &instanceDesc
    );
    mpInstanceBuffer->setName("D3D12RaytracingHelloWorld InstanceDesc");

    pRenderContext->resourceBarrier(mpInstanceBuffer.get(), Resource::State::NonPixelShader);

    RtAccelerationStructureBuildInputs tlasInputs = {};
    tlasInputs.kind = RtAccelerationStructureKind::TopLevel;
    tlasInputs.flags = RtAccelerationStructureBuildFlags::PreferFastTrace;
    tlasInputs.descCount = 1;
    tlasInputs.instanceDescs = mpInstanceBuffer->getGpuAddress();

    auto tlasPrebuild = RtAccelerationStructure::getPrebuildInfo(getDevice().get(), tlasInputs);

    mpTlasBuffer = getDevice()->createBuffer(
        tlasPrebuild.resultDataMaxSize,
        ResourceBindFlags::AccelerationStructure,
        MemoryType::DeviceLocal
    );
    mpTlasBuffer->setName("D3D12RaytracingHelloWorld TLAS");

    if (tlasPrebuild.scratchDataSize > mpScratchBuffer->getSize())
    {
        mpScratchBuffer = getDevice()->createBuffer(
            tlasPrebuild.scratchDataSize,
            ResourceBindFlags::UnorderedAccess,
            MemoryType::DeviceLocal
        );
    }
    pRenderContext->uavBarrier(mpScratchBuffer.get());

    RtAccelerationStructure::Desc tlasDesc;
    tlasDesc.setKind(RtAccelerationStructureKind::TopLevel);
    tlasDesc.setBuffer(mpTlasBuffer, 0, tlasPrebuild.resultDataMaxSize);
    mpTlas = RtAccelerationStructure::create(getDevice(), tlasDesc);

    RtAccelerationStructure::BuildDesc tlasBuildDesc = {};
    tlasBuildDesc.inputs = tlasInputs;
    tlasBuildDesc.scratchData = mpScratchBuffer->getGpuAddress();
    tlasBuildDesc.dest = mpTlas.get();

    pRenderContext->buildAccelerationStructure(tlasBuildDesc, 0, nullptr);
    pRenderContext->uavBarrier(mpTlasBuffer.get());
}

void D3D12RaytracingHelloWorld::updateViewportConstants(uint32_t width, uint32_t height)
{
    mRayGenCB.viewport = {-1.0f, -1.0f, 1.0f, 1.0f};

    const float border = 0.1f;
    const float aspectRatio = (height > 0) ? (float)width / height : (1280.f / 720.f);

    if (width <= height)
    {
        mRayGenCB.stencil = {
            -1.f + border,
            -1.f + border * aspectRatio,
            1.f - border,
            1.f - border * aspectRatio,
        };
    }
    else
    {
        mRayGenCB.stencil = {
            -1.f + border / aspectRatio,
            -1.f + border,
            1.f - border / aspectRatio,
            1.f - border,
        };
    }
}

void D3D12RaytracingHelloWorld::onShutdown() {}

void D3D12RaytracingHelloWorld::onResize(uint32_t width, uint32_t height)
{
    updateViewportConstants(width, height);

    if (mpRtOut && (mpRtOut->getWidth() != width || mpRtOut->getHeight() != height))
    {
        mpRtOut = getDevice()->createTexture2D(
            width, height, ResourceFormat::RGBA8Unorm, 1, 1, nullptr,
            ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource
        );
    }
}

void D3D12RaytracingHelloWorld::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 sample: clear not used for final output (we blit raytrace result)
    const float4 clearColor(0, 0, 0, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    auto var = mpRtVars->getRootVar();
    var["gRtScene"]["rtAccel"].setAccelerationStructure(mpTlas);
    var["g_rayGenCB"]["viewport"]["left"] = mRayGenCB.viewport.left;
    var["g_rayGenCB"]["viewport"]["top"] = mRayGenCB.viewport.top;
    var["g_rayGenCB"]["viewport"]["right"] = mRayGenCB.viewport.right;
    var["g_rayGenCB"]["viewport"]["bottom"] = mRayGenCB.viewport.bottom;
    var["g_rayGenCB"]["stencil"]["left"] = mRayGenCB.stencil.left;
    var["g_rayGenCB"]["stencil"]["top"] = mRayGenCB.stencil.top;
    var["g_rayGenCB"]["stencil"]["right"] = mRayGenCB.stencil.right;
    var["g_rayGenCB"]["stencil"]["bottom"] = mRayGenCB.stencil.bottom;
    var["gOutput"] = mpRtOut;

    pRenderContext->clearUAV(mpRtOut->getUAV().get(), float4(0, 0, 0, 1));
    pRenderContext->raytrace(
        mpRaytraceProgram.get(),
        mpRtVars.get(),
        pTargetFbo->getWidth(),
        pTargetFbo->getHeight(),
        1
    );
    pRenderContext->blit(mpRtOut->getSRV(), pTargetFbo->getRenderTargetView(0));
}

void D3D12RaytracingHelloWorld::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Raytracing Hello World", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Raytracing Hello World - migrated from DirectX-Graphics-Samples");
}

bool D3D12RaytracingHelloWorld::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12RaytracingHelloWorld::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12RaytracingHelloWorld::onHotReload(HotReloadFlags reloaded)
{
    //
}

SampleBase* D3D12RaytracingHelloWorld::create(SampleApp* pHost)
{
    return new D3D12RaytracingHelloWorld(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12RaytracingHelloWorld>();
}
