#include "SceneDebugger.h"


#include <sstream>
#include <unordered_map>

namespace
{

const char kShaderFile[] = "RenderPasses/SceneDebugger/SceneDebugger.cs.slang";

const std::string kOutput = "output";

std::string getModeDesc(SceneDebuggerMode mode)
{
    switch (mode)
    {
    case SceneDebuggerMode::FlatShaded:
        return "Flat shaded";
    case SceneDebuggerMode::TriangleDensity:
        return "Triangle density";
    // Geometry
    case SceneDebuggerMode::HitType:
        return "Hit type in pseudocolor";
    case SceneDebuggerMode::InstanceID:
        return "Instance ID in pseudocolor";
    case SceneDebuggerMode::MaterialID:
        return "Material ID in pseudocolor";
    case SceneDebuggerMode::PrimitiveID:
        return "Primitive ID in pseudocolor";
    case SceneDebuggerMode::GeometryID:
        return "Geometry ID in pseudocolor";
    case SceneDebuggerMode::BlasID:
        return "Raytracing bottom-level acceleration structure (BLAS) ID in pseudocolor";
    case SceneDebuggerMode::InstancedGeometry:
        return "Green = instanced geometry\n"
               "Red = non-instanced geometry";
    case SceneDebuggerMode::MaterialType:
        return "Material type in pseudocolor";
    case SceneDebuggerMode::MeshletID:
        return "Meshlet ID in pseudocolor";
    // Shading data
    case SceneDebuggerMode::FaceNormal:
        return "Face normal in RGB color";
    case SceneDebuggerMode::ShadingNormal:
        return "Shading normal in RGB color";
    case SceneDebuggerMode::ShadingTangent:
        return "Shading tangent in RGB color";
    case SceneDebuggerMode::ShadingBitangent:
        return "Shading bitangent in RGB color";
    case SceneDebuggerMode::FrontFacingFlag:
        return "Green = front-facing\n"
               "Red = back-facing";
    case SceneDebuggerMode::BackfacingShadingNormal:
        return "Pixels where the shading normal is back-facing with respect to view vector are highlighted";
    case SceneDebuggerMode::TexCoords:
        return "Texture coordinates in RG color wrapped to [0,1]";
    // Material properties
    case SceneDebuggerMode::BSDFProperties:
        return "BSDF properties";
    default:
        FALCOR_UNREACHABLE();
        return "";
    }
}

// Scripting
const char kMode[] = "mode";
const char kShowVolumes[] = "showVolumes";
const char kUseVBuffer[] = "useVBuffer";

void registerBindings(pybind11::module& m)
{
    pybind11::class_<SceneDebugger, RenderPass, ref<SceneDebugger>> pass(m, "SceneDebugger");
    pass.def_property(
        kMode,
        [](const SceneDebugger& self) { return enumToString(self.getMode()); },
        [](SceneDebugger& self, const std::string& value) { self.setMode(stringToEnum<SceneDebuggerMode>(value)); }
    );
}
} // namespace

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, SceneDebugger>();
    Falcor::ScriptBindings::registerBinding(registerBindings);
}

SceneDebugger::SceneDebugger(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    if (!mpDevice->isShaderModelSupported(ShaderModel::SM6_5))
        FALCOR_THROW("SceneDebugger requires Shader Model 6.5 support.");
    if (!mpDevice->isFeatureSupported(Device::SupportedFeatures::RaytracingTier1_1))
        FALCOR_THROW("SceneDebugger requires Raytracing Tier 1.1 support.");

    // Parse dictionary.
    for (const auto& [key, value] : props)
    {
        if (key == kMode)
            mParams.mode = (uint32_t)value.operator SceneDebuggerMode();
        else if (key == kShowVolumes)
            mParams.showVolumes = value;
        else if (key == kUseVBuffer)
            mParams.useVBuffer = static_cast<bool>(value);
        else
            logWarning("Unknown property '{}' in a SceneDebugger properties.", key);
    }

    mpFence = mpDevice->createFence();

    mpPixelDebug = std::make_unique<PixelDebug>(mpDevice);
    mpSampleGenerator = SampleGenerator::create(mpDevice, SAMPLE_GENERATOR_TINY_UNIFORM);
}

Properties SceneDebugger::getProperties() const
{
    Properties props;
    props[kMode] = SceneDebuggerMode(mParams.mode);
    props[kShowVolumes] = mParams.showVolumes;
    props[kUseVBuffer] = mParams.useVBuffer;
    return props;
}

RenderPassReflection SceneDebugger::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput("vbuffer", "Visibility buffer in packed format")
        .texture2D()
        .format(ResourceFormat::RGBA32Uint)
        .flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addOutput(kOutput, "Scene debugger output").bindFlags(ResourceBindFlags::UnorderedAccess).format(ResourceFormat::RGBA32Float);

    return reflector;
}

void SceneDebugger::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mParams.frameDim = compileData.defaultTexDims;
    mVBufferAvailable = compileData.connectedResources.getField("vbuffer");
}

void SceneDebugger::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mUpdateFlagsConnection = {};
    mUpdateFlags = IScene::UpdateFlags::None;

    mpScene = pScene;
    mpMeshToBlasID = nullptr;
    mpDebugPass = nullptr;
    mUpdateFlags = IScene::UpdateFlags::None;

    if (mpScene)
    {
        mUpdateFlagsConnection = mpScene->getUpdateFlagsSignal().connect([&](IScene::UpdateFlags flags) { mUpdateFlags |= flags; });

        // Prepare our programs for the scene.
        ProgramDesc desc;
        desc.addShaderModules(mpScene->getShaderModules());
        desc.addShaderLibrary(kShaderFile).csEntry("main");
        desc.addTypeConformances(mpScene->getTypeConformances());

        DefineList defines = mpScene->getSceneDefines();
        defines.add(mpSampleGenerator->getDefines());
        mpDebugPass = ComputePass::create(mpDevice, desc, defines);

        // Create lookup table for mesh to BLAS ID.
        auto blasIDs = mpScene->getMeshBlasIDs();
        if (!blasIDs.empty())
        {
            mpMeshToBlasID = mpDevice->createStructuredBuffer(
                sizeof(uint32_t),
                (uint32_t)blasIDs.size(),
                ResourceBindFlags::ShaderResource,
                MemoryType::DeviceLocal,
                blasIDs.data(),
                false
            );
        }

        // Create instance metadata.
        initInstanceInfo();

        // Build meshlets for the first mesh
        buildMeshlets();

        // Bind variables.
        auto var = mpDebugPass->getRootVar()["CB"]["gSceneDebugger"];
        if (!mpPixelData)
        {
            mpPixelData = mpDevice->createStructuredBuffer(
                var["pixelData"],
                1,
                ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
                MemoryType::DeviceLocal,
                nullptr,
                false
            );
            mpPixelDataStaging =
                mpDevice->createStructuredBuffer(var["pixelData"], 1, ResourceBindFlags::None, MemoryType::ReadBack, nullptr, false);
        }
        var["pixelData"] = mpPixelData;
        var["meshToBlasID"] = mpMeshToBlasID;
        var["instanceInfo"] = mpInstanceInfo;
        var["meshletData"] = mpMeshletData;
        var["meshlets"] = mpMeshletBuffer;
        var["meshletVertices"] = mpMeshletVertices;
        var["meshletTriangles"] = mpMeshletTriangles;
        var["meshletGlobalPositions"] = mpMeshletGlobalPositions;
        var["triangleToMeshlet"] = mpTriangleToMeshlet;
    }
}


void SceneDebugger::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    mPixelDataAvailable = false;
    const auto& pOutput = renderData.getTexture(kOutput);
    const auto& pVBuffer = renderData.getTexture("vbuffer");

    if (mParams.useVBuffer && pVBuffer == nullptr)
    {
        logWarningOnce("SceneDebugger cannot use vbuffer as none is connected");
        mParams.useVBuffer = false;
    }

    if (mpScene == nullptr)
    {
        pRenderContext->clearUAV(pOutput->getUAV().get(), float4(0.f));
        return;
    }

    // DEMO21:
    // mpScene->getCamera()->setJitter(0.f, 0.f);

    if (is_set(mUpdateFlags, IScene::UpdateFlags::RecompileNeeded) || is_set(mUpdateFlags, IScene::UpdateFlags::GeometryChanged))
    {
        FALCOR_THROW("This render pass does not support scene changes that require shader recompilation.");
    }

    if (mpScene)
        mpScene->bindShaderDataForRaytracing(pRenderContext, mpDebugPass->getRootVar()["gScene"]);


    ShaderVar var = mpDebugPass->getRootVar()["CB"]["gSceneDebugger"];
    var["params"].setBlob(mParams);
    var["output"] = pOutput;
    var["vbuffer"] = pVBuffer;

    mpPixelDebug->beginFrame(pRenderContext, renderData.getDefaultTextureDims());
    mpPixelDebug->prepareProgram(mpDebugPass->getProgram(), mpDebugPass->getRootVar());
    mpSampleGenerator->bindShaderData(mpDebugPass->getRootVar());

    mpDebugPass->execute(pRenderContext, uint3(mParams.frameDim, 1));

    pRenderContext->copyResource(mpPixelDataStaging.get(), mpPixelData.get());
    pRenderContext->submit(false);
    pRenderContext->signal(mpFence.get());

    mPixelDataAvailable = true;
    mParams.frameCount++;

    mpPixelDebug->endFrame(pRenderContext);

    mUpdateFlags = IScene::UpdateFlags::None;
}

void SceneDebugger::renderUI(Gui::Widgets& widget)
{
    if (mVBufferAvailable)
        widget.checkbox("Use VBuffer", mParams.useVBuffer);

    widget.dropdown("Mode", reinterpret_cast<SceneDebuggerMode&>(mParams.mode));
    widget.tooltip("Selects visualization mode");

    if (mParams.mode == (uint32_t)SceneDebuggerMode::TriangleDensity)
    {
        widget.var("Triangle density range (log2)", mParams.triangleDensityLogRange);
    }

    if (mParams.mode == (uint32_t)SceneDebuggerMode::BSDFProperties)
    {
        widget.dropdown("BSDF property", reinterpret_cast<SceneDebuggerBSDFProperty&>(mParams.bsdfProperty));
        widget.var("BSDF index", mParams.bsdfIndex, 0u, 15u, 1u);
    }

    if (mParams.mode == (uint32_t)SceneDebuggerMode::MeshletID)
    {
        widget.text("Meshlet Statistics:");
        widget.text("Meshlet count: " + std::to_string(mMeshletBuildResult.meshlets.size()));
        uint32_t totalTriangles = 0;
        for (const auto& m : mMeshletBuildResult.meshlets)
            totalTriangles += m.triangle_count;
        widget.text("Total triangles: " + std::to_string(totalTriangles));
    }

    widget.checkbox("Clamp to [0,1]", mParams.clamp);
    widget.tooltip("Clamp pixel values to [0,1] before output.");

    if ((SceneDebuggerMode)mParams.mode == SceneDebuggerMode::FaceNormal ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::ShadingNormal ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::ShadingTangent ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::ShadingBitangent ||
        (SceneDebuggerMode)mParams.mode == SceneDebuggerMode::TexCoords)
    {
        widget.checkbox("Flip sign", mParams.flipSign);
        widget.checkbox("Remap to [0,1]", mParams.remapRange);
        widget.tooltip("Remap range from [-1,1] to [0,1] before output.");
    }

    widget.checkbox("Show volumes", mParams.showVolumes);
    if (mParams.showVolumes)
    {
        widget.var("Volume density scale", mParams.volumeDensityScale, 0.f, 1000.f, 0.1f);
    }

    widget.textWrapped("Description:\n" + getModeDesc((SceneDebuggerMode)mParams.mode));

    // Show data for the currently selected pixel.
    widget.dummy("#spacer0", {1, 20});
    widget.var("Selected pixel", mParams.selectedPixel);

    if (mpScene)
        renderPixelDataUI(widget);

    widget.dummy("#spacer1", {1, 20});
    widget.text("Scene: " + (mpScene ? mpScene->getPath().string() : "No scene loaded"));

    if (auto loggingGroup = widget.group("Logging", false))
    {
        mpPixelDebug->renderUI(widget);
    }

    if (auto g = widget.group("Profiling", false))
    {
        widget.checkbox("Trace secondary rays", mParams.profileSecondaryRays);
        if (mParams.profileSecondaryRays)
        {
            widget.checkbox("Load hit info", mParams.profileSecondaryLoadHit);

            widget.var("Cone angle (deg)", mParams.profileSecondaryConeAngle, 0.f, 90.f, 1.f);
            widget.tooltip(
                "Traces secondary rays from the primary hits. The secondary rays have directions that are randomly distributed in a cone "
                "around the face normal."
            );
        }
    }
}

void SceneDebugger::renderPixelDataUI(Gui::Widgets& widget)
{
    if (!mPixelDataAvailable)
        return;

    FALCOR_ASSERT(mpPixelDataStaging);
    mpFence->wait();
    const PixelData data = mpPixelDataStaging->getElement<PixelData>(0);

    switch ((HitType)data.hitType)
    {
    case HitType::Triangle:
    {
        {
            std::string text;
            text += fmt::format("Mesh ID: {}\n", data.geometryID);
            text += fmt::format("Mesh name: {}\n", mpScene->hasMesh(data.geometryID) ? mpScene->getMeshName(data.geometryID) : "unknown");
            text += fmt::format("Instance ID: {}\n", data.instanceID);
            text += fmt::format("Material ID: {}\n", data.materialID);
            text += fmt::format("BLAS ID: {}\n", data.blasID);
            widget.text(text);
            widget.dummy("#spacer2", {1, 10});
        }

        // Show mesh details.
        if (auto g = widget.group("Mesh info"); g.open())
        {
            FALCOR_ASSERT(data.geometryID < mpScene->getMeshCount());
            const auto& mesh = mpScene->getMesh(MeshID{data.geometryID});
            std::string text;
            text += fmt::format("flags: 0x{:08x}\n", mesh.flags);
            text += fmt::format("materialID: {}\n", mesh.materialID);
            text += fmt::format("vertexCount: {}\n", mesh.vertexCount);
            text += fmt::format("indexCount: {}\n", mesh.indexCount);
            text += fmt::format("triangleCount: {}\n", mesh.getTriangleCount());
            text += fmt::format("vbOffset: {}\n", mesh.vbOffset);
            text += fmt::format("ibOffset: {}\n", mesh.ibOffset);
            text += fmt::format("skinningVbOffset: {}\n", mesh.skinningVbOffset);
            text += fmt::format("prevVbOffset: {}\n", mesh.prevVbOffset);
            text += fmt::format("use16BitIndices: {}\n", mesh.use16BitIndices());
            text += fmt::format("isFrontFaceCW: {}\n", mesh.isFrontFaceCW());
            g.text(text);
        }

        // Show mesh instance info.
        if (auto g = widget.group("Mesh instance info"); g.open())
        {
            FALCOR_ASSERT(data.instanceID < mpScene->getGeometryInstanceCount());
            const auto& instance = mpScene->getGeometryInstance(data.instanceID);
            std::string text;
            text += fmt::format("flags: 0x{:08x}\n", instance.flags);
            text += fmt::format("nodeID: {}\n", instance.globalMatrixID);
            text += fmt::format("meshID: {}\n", instance.geometryID);
            text += fmt::format("materialID: {}\n", instance.materialID);
            text += fmt::format("vbOffset: {}\n", instance.vbOffset);
            text += fmt::format("ibOffset: {}\n", instance.ibOffset);
            text += fmt::format("isDynamic: {}\n", instance.isDynamic());
            g.text(text);

            // Print the list of scene graph nodes affecting this mesh instance.
            std::vector<NodeID> nodes;
            {
                NodeID nodeID{instance.globalMatrixID};
                while (nodeID != NodeID::Invalid())
                {
                    nodes.push_back(nodeID);
                    nodeID = mpScene->getParentNodeID(nodeID);
                }
            }
            FALCOR_ASSERT(!nodes.empty());

            g.text("Scene graph (root first):");
            const auto& localMatrices = mpScene->getAnimationController()->getLocalMatrices();
            for (auto it = nodes.rbegin(); it != nodes.rend(); it++)
            {
                auto nodeID = *it;
                float4x4 mat = localMatrices[nodeID.get()];
                if (auto nodeGroup = widget.group("ID " + to_string(nodeID)); nodeGroup.open())
                {
                    g.matrix("", mat);
                }
            }
        }
        break;
    }
    case HitType::Curve:
    {
        {
            std::string text;
            text += fmt::format("Curve ID: {}\n", data.geometryID);
            text += fmt::format("Instance ID: {}\n", data.instanceID);
            text += fmt::format("Material ID: {}\n", data.materialID);
            text += fmt::format("BLAS ID: {}\n", data.blasID);
            widget.text(text);
            widget.dummy("#spacer2", {1, 10});
        }

        // Show mesh details.
        if (auto g = widget.group("Curve info"); g.open())
        {
            const auto& curve = mpScene->getCurve(CurveID{data.geometryID});
            std::string text;
            text += fmt::format("degree: {}\n", curve.degree);
            text += fmt::format("vertexCount: {}\n", curve.vertexCount);
            text += fmt::format("indexCount: {}\n", curve.indexCount);
            text += fmt::format("vbOffset: {}\n", curve.vbOffset);
            text += fmt::format("ibOffset: {}\n", curve.ibOffset);
            g.text(text);
        }
        break;
    }
    case HitType::SDFGrid:
    {
        {
            std::string text;
            text += fmt::format("SDF Grid ID: {}\n", data.geometryID);
            text += fmt::format("Instance ID: {}\n", data.instanceID);
            text += fmt::format("Material ID: {}\n", data.materialID);
            text += fmt::format("BLAS ID: {}\n", data.blasID);
            widget.text(text);
            widget.dummy("#spacer2", {1, 10});
        }

        // Show SDF grid details.
        if (auto g = widget.group("SDF grid info"); g.open())
        {
            const ref<SDFGrid>& pSDFGrid = mpScene->getSDFGrid(SdfGridID{data.geometryID});
            std::string text;
            text += fmt::format("gridWidth: {}\n", pSDFGrid->getGridWidth());
            g.text(text);
        }
        break;
    }
    case HitType::None:
        widget.text("Background pixel");
        break;
    default:
        widget.text("Unsupported hit type");
        break;
    }

    // Show shading data.
    if ((HitType)data.hitType != HitType::None)
    {
        if (auto g = widget.group("Shading data"); g.open())
        {
            std::string text;
            text += fmt::format("posW: {}\n", data.posW);
            text += fmt::format("V: {}\n", data.V);
            text += fmt::format("N: {}\n", data.N);
            text += fmt::format("T: {}\n", data.T);
            text += fmt::format("B: {}\n", data.B);
            text += fmt::format("uv: {}\n", data.uv);
            text += fmt::format("faceN: {}\n", data.faceN);
            text += fmt::format("tangentW: {}\n", data.tangentW);
            text += fmt::format("frontFacing: {}\n", data.frontFacing);
            text += fmt::format("curveRadius: {}\n", data.curveRadius);
            g.text(text);
        }
    }

    // Show material info.
    if (data.materialID != PixelData::kInvalidID)
    {
        if (auto g = widget.group("Material info"); g.open())
        {
            const auto& material = *mpScene->getMaterial(MaterialID{data.materialID});
            const auto& header = material.getHeader();
            std::string text;
            text += fmt::format("name: {}\n", material.getName());
            text += fmt::format("materialType: {}\n", to_string(header.getMaterialType()));
            text += fmt::format("alphaMode: {}\n", (uint32_t)header.getAlphaMode());
            text += fmt::format("alphaThreshold: {}\n", (float)header.getAlphaThreshold());
            text += fmt::format("nestedPriority: {}\n", header.getNestedPriority());
            text += fmt::format("activeLobes: 0x{:08x}\n", (uint32_t)header.getActiveLobes());
            text += fmt::format("defaultTextureSamplerID: {}\n", header.getDefaultTextureSamplerID());
            text += fmt::format("doubleSided: {}\n", header.isDoubleSided());
            text += fmt::format("thinSurface: {}\n", header.isThinSurface());
            text += fmt::format("emissive: {}\n", header.isEmissive());
            text += fmt::format("basicMaterial: {}\n", header.isBasicMaterial());
            text += fmt::format("lightProfileEnabled: {}\n", header.isLightProfileEnabled());
            text += fmt::format("deltaSpecular: {}\n", header.isDeltaSpecular());
            g.text(text);
        }
    }
}

bool SceneDebugger::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mouseEvent.type == MouseEvent::Type::ButtonDown && mouseEvent.button == Input::MouseButton::Left)
    {
        float2 cursorPos = mouseEvent.pos * (float2)mParams.frameDim;
        mParams.selectedPixel = (uint2)clamp(cursorPos, float2(0.f), float2(mParams.frameDim.x - 1, mParams.frameDim.y - 1));
    }

    return mpPixelDebug->onMouseEvent(mouseEvent);
}

void SceneDebugger::initInstanceInfo()
{
    const uint32_t instanceCount = mpScene ? mpScene->getGeometryInstanceCount() : 0;

    // If there are no instances. Just clear the buffer and return.
    if (instanceCount == 0)
    {
        mpInstanceInfo = nullptr;
        return;
    }

    // Count number of times each geometry is used.
    std::vector<std::vector<uint32_t>> instanceCounts((size_t)GeometryType::Count);
    for (auto& counts : instanceCounts)
        counts.resize(mpScene->getGeometryCount());

    for (uint32_t instanceID = 0; instanceID < instanceCount; instanceID++)
    {
        const auto& instance = mpScene->getGeometryInstance(instanceID);
        instanceCounts[(size_t)instance.getType()][instance.geometryID]++;
    }

    // Setup instance metadata.
    std::vector<InstanceInfo> instanceInfo(instanceCount);
    for (uint32_t instanceID = 0; instanceID < instanceCount; instanceID++)
    {
        const auto& instance = mpScene->getGeometryInstance(instanceID);
        auto& info = instanceInfo[instanceID];
        if (instanceCounts[(size_t)instance.getType()][instance.geometryID] > 1)
        {
            info.flags |= (uint32_t)InstanceInfoFlags::IsInstanced;
        }
    }

    // Create GPU buffer.
    mpInstanceInfo = mpDevice->createStructuredBuffer(
        sizeof(InstanceInfo),
        (uint32_t)instanceInfo.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        instanceInfo.data(),
        false
    );
}

void SceneDebugger::buildMeshlets()
{
    // Clear previous meshlet data
    mpMeshletBuffer = nullptr;
    mpMeshletVertices = nullptr;
    mpMeshletTriangles = nullptr;
    mpMeshletGlobalPositions = nullptr;
    mpMeshletData = nullptr;
    mpTriangleToMeshlet = nullptr;
    mMeshletBuildResult = MeshletBuildResult();

    if (!mpScene || mpScene->getMeshCount() == 0)
        return;

    // Extract mesh data from the first mesh
    const auto& meshDesc = mpScene->getMesh(MeshID{0});
    uint32_t vertexCount = meshDesc.vertexCount;
    uint32_t triangleCount = meshDesc.getTriangleCount();

    logInfo("Building meshlets for mesh with {} vertices and {} triangles", vertexCount, triangleCount);

    // Create output buffers for mesh data
    std::map<std::string, ref<Buffer>> buffers;
    buffers["triangleIndices"] = mpDevice->createStructuredBuffer(
        sizeof(uint3),
        triangleCount,
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal,
        nullptr,
        false
    );
    buffers["positions"] = mpDevice->createStructuredBuffer(
        sizeof(float3),
        vertexCount,
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal,
        nullptr,
        false
    );
    buffers["texcrds"] = mpDevice->createStructuredBuffer(
        sizeof(float3),
        vertexCount,
        ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess,
        MemoryType::DeviceLocal,
        nullptr,
        false
    );

    // Extract mesh data via compute shader
    mpScene->getMeshVerticesAndIndices(MeshID{0}, buffers);

    // Read back data from GPU to CPU
    std::vector<uint3> indices(triangleCount);
    std::vector<float3> positions(vertexCount);

    buffers["triangleIndices"]->getBlob(indices.data(), 0, triangleCount * sizeof(uint3));
    buffers["positions"]->getBlob(positions.data(), 0, vertexCount * sizeof(float3));

    // Flatten indices to uint32 array
    std::vector<uint32_t> flatIndices(triangleCount * 3);
    for (uint32_t i = 0; i < triangleCount; i++)
    {
        flatIndices[i * 3 + 0] = indices[i].x;
        flatIndices[i * 3 + 1] = indices[i].y;
        flatIndices[i * 3 + 2] = indices[i].z;
    }

    // Generate vertex remap for optimization
    uint32_t indexCount = (uint32_t)flatIndices.size();
    std::vector<uint32_t> remap(indexCount);
    size_t vertex_count = meshopt_generateVertexRemap(
        remap.data(),
        flatIndices.data(),
        indexCount,
        positions.data(),
        vertexCount,
        sizeof(float3)
    );

    // Remap indices
    std::vector<uint32_t> remappedIndices(indexCount);
    meshopt_remapIndexBuffer(remappedIndices.data(), flatIndices.data(), indexCount, remap.data());

    // Remap vertices - save to member variable
    mMeshletBuildResult.remappedPositions.resize(vertex_count);
    meshopt_remapVertexBuffer(
        mMeshletBuildResult.remappedPositions.data(),
        positions.data(),
        vertexCount,
        sizeof(float3),
        remap.data()
    );
    mMeshletBuildResult.remappedVertexCount = vertex_count;

    logInfo("Vertex remap: {} vertices -> {} vertices", vertexCount, vertex_count);

    // Build meshlets using meshoptimizer with recommended parameters
    const size_t maxVertices = 64;    // Match GPU workgroup size
    const size_t maxTriangles = 124;  // 124 * 3 = 372 < 384 (hardware limit)
    const float coneWeight = 0.5f;    // Enable normal cone optimization

    size_t maxMeshlets = meshopt_buildMeshletsBound(remappedIndices.size(), maxVertices, maxTriangles);
    mMeshletBuildResult.meshlets.resize(maxMeshlets);
    mMeshletBuildResult.meshletVertices.resize(maxMeshlets * maxVertices);
    mMeshletBuildResult.meshletTriangles.resize(maxMeshlets * maxTriangles * 3);

    size_t meshletCount = meshopt_buildMeshlets(
        mMeshletBuildResult.meshlets.data(),
        mMeshletBuildResult.meshletVertices.data(),
        mMeshletBuildResult.meshletTriangles.data(),
        remappedIndices.data(),
        remappedIndices.size(),
        reinterpret_cast<const float*>(mMeshletBuildResult.remappedPositions.data()),
        vertex_count,
        sizeof(float3),
        maxVertices,
        maxTriangles,
        coneWeight
    );

    // Resize to actual count
    mMeshletBuildResult.meshlets.resize(meshletCount);
    const meshopt_Meshlet& lastMeshlet = mMeshletBuildResult.meshlets.back();
    mMeshletBuildResult.meshletVertices.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
    mMeshletBuildResult.meshletTriangles.resize(lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));

    // Compute bounding spheres
    mMeshletBuildResult.meshletBounds.resize(meshletCount);
    for (size_t i = 0; i < meshletCount; i++)
    {
        const meshopt_Meshlet& m = mMeshletBuildResult.meshlets[i];
        meshopt_Bounds bounds = meshopt_computeMeshletBounds(
            &mMeshletBuildResult.meshletVertices[m.vertex_offset],
            &mMeshletBuildResult.meshletTriangles[m.triangle_offset],
            m.triangle_count,
            reinterpret_cast<const float*>(mMeshletBuildResult.remappedPositions.data()),
            vertex_count,
            sizeof(float3)
        );

        mMeshletBuildResult.meshletBounds[i] = float4(bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius);
    }

    // Calculate total triangles
    uint32_t totalTriangles = 0;
    for (const auto& m : mMeshletBuildResult.meshlets)
    {
        totalTriangles += m.triangle_count;
    }

    logInfo("Built {} meshlets with {} total triangles", meshletCount, totalTriangles);

    // Build mapping from original triangle ID to meshlet ID
    // This is critical because shader uses primitiveID (original scene order)

    // Helper function to create unique triangle key (order-independent)
    auto makeTriangleKey = [](uint32_t v0, uint32_t v1, uint32_t v2) -> uint64_t {
        if (v0 > v1) std::swap(v0, v1);
        if (v1 > v2) std::swap(v1, v2);
        if (v0 > v1) std::swap(v0, v1);
        return ((uint64_t)v0 << 42) | ((uint64_t)v1 << 21) | (uint64_t)v2;
    };

    // Build hash map of original triangles (using remapped indices)
    std::unordered_map<uint64_t, uint32_t> originalTriangleMap;
    for (uint32_t origTri = 0; origTri < triangleCount; origTri++)
    {
        uint32_t v0 = remappedIndices[origTri * 3 + 0];
        uint32_t v1 = remappedIndices[origTri * 3 + 1];
        uint32_t v2 = remappedIndices[origTri * 3 + 2];
        uint64_t key = makeTriangleKey(v0, v1, v2);
        originalTriangleMap[key] = origTri;
    }

    // Create mapping: original triangle ID -> meshlet ID
    std::vector<uint32_t> originalTriToMeshlet(triangleCount, 0xFFFFFFFF);

    for (size_t meshletIdx = 0; meshletIdx < meshletCount; meshletIdx++)
    {
        const auto& m = mMeshletBuildResult.meshlets[meshletIdx];

        for (uint32_t t = 0; t < m.triangle_count; t++)
        {
            // Get local vertex indices from meshlet triangle data
            uint32_t triOffset = m.triangle_offset + t * 3;
            uint8_t lv0 = mMeshletBuildResult.meshletTriangles[triOffset + 0];
            uint8_t lv1 = mMeshletBuildResult.meshletTriangles[triOffset + 1];
            uint8_t lv2 = mMeshletBuildResult.meshletTriangles[triOffset + 2];

            // Convert to global vertex indices
            uint32_t gv0 = mMeshletBuildResult.meshletVertices[m.vertex_offset + lv0];
            uint32_t gv1 = mMeshletBuildResult.meshletVertices[m.vertex_offset + lv1];
            uint32_t gv2 = mMeshletBuildResult.meshletVertices[m.vertex_offset + lv2];

            // Find corresponding original triangle
            uint64_t key = makeTriangleKey(gv0, gv1, gv2);
            auto it = originalTriangleMap.find(key);
            if (it != originalTriangleMap.end())
            {
                originalTriToMeshlet[it->second] = (uint32_t)meshletIdx;
            }
        }
    }

    // Verify mapping completeness
    uint32_t unmappedCount = 0;
    for (uint32_t i = 0; i < triangleCount; i++)
    {
        if (originalTriToMeshlet[i] == 0xFFFFFFFF)
        {
            unmappedCount++;
            originalTriToMeshlet[i] = 0; // Set default to avoid out-of-bounds
        }
    }
    if (unmappedCount > 0)
    {
        logWarning("Meshlet mapping: {} triangles could not be mapped", unmappedCount);
    }
    else
    {
        logInfo("Meshlet mapping: all {} triangles successfully mapped", triangleCount);
    }

    // Create GPU buffers with correct mapping
    createMeshletBuffers(originalTriToMeshlet, totalTriangles);
}

void SceneDebugger::createMeshletBuffers(const std::vector<uint32_t>& triangleToMeshlet, uint32_t totalTriangles)
{
    if (mMeshletBuildResult.meshlets.empty())
    {
        // Create empty buffers
        MeshletData meshletData = {};
        mpMeshletData = mpDevice->createStructuredBuffer(
            sizeof(MeshletData),
            1,
            ResourceBindFlags::ShaderResource,
            MemoryType::DeviceLocal,
            &meshletData,
            false
        );
        return;
    }

    // Create GPU meshlet buffer
    std::vector<GpuMeshlet> gpuMeshlets(mMeshletBuildResult.meshlets.size());
    for (size_t i = 0; i < mMeshletBuildResult.meshlets.size(); i++)
    {
        const meshopt_Meshlet& m = mMeshletBuildResult.meshlets[i];
        const float4& bounds = mMeshletBuildResult.meshletBounds[i];

        gpuMeshlets[i].vertexOffset = m.vertex_offset;
        gpuMeshlets[i].triangleOffset = m.triangle_offset;
        gpuMeshlets[i].vertexCount = m.vertex_count;
        gpuMeshlets[i].triangleCount = m.triangle_count;
        gpuMeshlets[i].boundCenter = float3(bounds.x, bounds.y, bounds.z);
        gpuMeshlets[i].boundRadius = bounds.w;
    }

    mpMeshletBuffer = mpDevice->createStructuredBuffer(
        sizeof(GpuMeshlet),
        (uint32_t)gpuMeshlets.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        gpuMeshlets.data(),
        false
    );

    // Create meshlet vertices buffer
    mpMeshletVertices = mpDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (uint32_t)mMeshletBuildResult.meshletVertices.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mMeshletBuildResult.meshletVertices.data(),
        false
    );

    // Pack uint8 triangle indices into uint32 buffer (4 indices per uint32)
    size_t packedSize = (mMeshletBuildResult.meshletTriangles.size() + 3) / 4;
    std::vector<uint32_t> packedTriangles(packedSize, 0);
    for (size_t i = 0; i < mMeshletBuildResult.meshletTriangles.size(); i++)
    {
        uint32_t byteIndex = i % 4;
        uint32_t uintIndex = i / 4;
        packedTriangles[uintIndex] |= (uint32_t)mMeshletBuildResult.meshletTriangles[i] << (byteIndex * 8);
    }

    mpMeshletTriangles = mpDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (uint32_t)packedTriangles.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        packedTriangles.data(),
        false
    );

    // Use remapped positions directly - this is the critical fix!
    mpMeshletGlobalPositions = mpDevice->createStructuredBuffer(
        sizeof(float3),
        (uint32_t)mMeshletBuildResult.remappedPositions.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mMeshletBuildResult.remappedPositions.data(),
        false
    );

    logInfo("Meshlet vertex buffer: {} vertices", mMeshletBuildResult.remappedPositions.size());

    // Create triangle to meshlet mapping buffer
    // Size is original triangle count (for primitiveID indexing in shader)
    mpTriangleToMeshlet = mpDevice->createStructuredBuffer(
        sizeof(uint32_t),
        (uint32_t)triangleToMeshlet.size(),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        triangleToMeshlet.data(),
        false
    );

    logInfo("Triangle to meshlet mapping buffer: {} entries", triangleToMeshlet.size());

    // Create meshlet metadata buffer
    MeshletData meshletData;
    meshletData.meshletCount = (uint32_t)mMeshletBuildResult.meshlets.size();
    meshletData.totalTriangles = totalTriangles;
    meshletData.originalTriangleCount = (uint32_t)triangleToMeshlet.size();
    meshletData._pad0 = 0;

    mpMeshletData = mpDevice->createStructuredBuffer(
        sizeof(MeshletData),
        1,
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        &meshletData,
        false
    );
}

