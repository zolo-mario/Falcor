#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "../GBufferBase.h"
#include <meshoptimizer.h>

using namespace Falcor;

class VBufferMeshletRaster : public GBufferBase
{
public:
    FALCOR_PLUGIN_CLASS(VBufferMeshletRaster, "VBufferMeshletRaster", "Software rasterizer for Nanite-style rendering using meshlets.");

    static ref<VBufferMeshletRaster> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<VBufferMeshletRaster>(pDevice, props);
    }

    VBufferMeshletRaster(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

private:
    // GPU meshlet structure (per-instance)
    struct GpuMeshlet
    {
        uint32_t vertexOffset;      // Offset into meshlet vertices buffer
        uint32_t triangleOffset;    // Offset into meshlet triangles buffer
        uint32_t vertexCount;       // Number of vertices in this meshlet
        uint32_t triangleCount;     // Number of triangles in this meshlet

        float3 boundCenter;         // Bounding sphere center (world space)
        float boundRadius;          // Bounding sphere radius

        uint32_t instanceID;        // Instance ID for this meshlet
        uint32_t primitiveOffset;   // Global primitive offset for this instance
        uint32_t meshID;            // Mesh ID
        uint32_t _pad0;
    };

    // Meshlet building result
    struct MeshletBuildResult
    {
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<uint32_t> meshletVertices;
        std::vector<uint8_t> meshletTriangles;
        std::vector<GpuMeshlet> gpuMeshlets;

        void clear()
        {
            meshlets.clear();
            meshletVertices.clear();
            meshletTriangles.clear();
            gpuMeshlets.clear();
        }
    };

    void buildMeshlets();
    void createGpuBuffers();

    // Render passes
    ref<ComputePass> mpRasterPass;
    ref<ComputePass> mpResolvePass;

    // GPU buffers
    ref<Texture> mpVisBufferDepth;      // Visibility buffer depth (R32Uint)
    ref<Texture> mpVisBufferPayload;    // Visibility buffer payload (R32Uint)
    ref<Buffer> mpMeshletBuffer;        // Structured buffer of GpuMeshlet
    ref<Buffer> mpMeshletVertices;      // Vertex indices
    ref<Buffer> mpMeshletTriangles;     // Triangle indices (packed as uint8)

    // Meshlet data
    MeshletBuildResult mBuildResult;
    uint32_t mMeshletCount = 0;
};
