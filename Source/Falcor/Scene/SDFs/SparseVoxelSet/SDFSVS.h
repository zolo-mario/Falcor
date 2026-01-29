#pragma once

#include "Scene/SDFs/SDFGrid.h"
#include "Core/API/Buffer.h"
#include "Core/API/Texture.h"
#include "Core/Pass/ComputePass.h"

namespace Falcor
{
    /** A single SDF Sparse Voxel Set. Can only be utilized on the GPU.
    */
    class FALCOR_API SDFSVS : public SDFGrid
    {
    public:
        static ref<SDFSVS> create(ref<Device> pDevice) { return make_ref<SDFSVS>(pDevice); }

        /// Create am empty SDF sparse voxel set.
        SDFSVS(ref<Device> pDevice) : SDFGrid(pDevice) {}

        virtual size_t getSize() const override;
        virtual uint32_t getMaxPrimitiveIDBits() const override;
        virtual Type getType() const override { return Type::SparseVoxelSet; }

        virtual void createResources(RenderContext* pRenderContext, bool deleteScratchData = true) override;

        virtual const ref<Buffer>& getAABBBuffer() const override { return mpVoxelAABBBuffer; }
        virtual uint32_t getAABBCount() const override { return mVoxelCount; }

        virtual void bindShaderData(const ShaderVar& var) const override;

    protected:
        virtual void setValuesInternal(const std::vector<float>& cornerValues) override;

    private:
        // CPU data.
        std::vector<int8_t> mValues;

        // Specs.
        ref<Buffer> mpVoxelAABBBuffer;
        ref<Buffer> mpVoxelBuffer;
        uint32_t mVoxelCount = 0;

        // Compute passes used to build the SVS.
        ref<ComputePass> mpCountSurfaceVoxelsPass;
        ref<ComputePass> mpSDFSVSVoxelizerPass;

        // Scratch data used for building.
        ref<Buffer> mpSurfaceVoxelCounter;
        ref<Texture> mpSDFGridTexture;
    };
}
