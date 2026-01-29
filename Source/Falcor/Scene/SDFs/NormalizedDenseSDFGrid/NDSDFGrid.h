#pragma once

#include "Scene/SDFs/SDFGrid.h"
#include "Core/API/Texture.h"

namespace Falcor
{
    /** A normalized dense SDF grid, represented as a set of textures. Can only be accessed on the GPU.
    */
    class FALCOR_API NDSDFGrid : public SDFGrid
    {
    public:
        struct SharedData;

        /** Create a new, empty normalized dense SDF grid.
            \param[in] narrowBandThickness NDSDFGrids operate on normalized distances, the distances are normalized so that a normalized distance of +- 1 represents a distance of "narrowBandThickness" voxel diameters. Should not be less than 1.
            \return NDSDFGrid object, or nullptr if errors occurred.
        */
        static ref<NDSDFGrid> create(ref<Device> pDevice, float narrowBandThickness) { return make_ref<NDSDFGrid>(pDevice, narrowBandThickness); }

        NDSDFGrid(ref<Device> pDevice, float narrowBandThickness);

        virtual size_t getSize() const override;
        virtual uint32_t getMaxPrimitiveIDBits() const override;
        virtual Type getType() const override { return Type::NormalizedDenseGrid; }


        virtual void createResources(RenderContext* pRenderContext, bool deleteScratchData = true) override;
        virtual const ref<Buffer>& getAABBBuffer() const override;
        virtual uint32_t getAABBCount() const override { return 1; }
        virtual void bindShaderData(const ShaderVar& var) const override;

    protected:
        virtual void setValuesInternal(const std::vector<float>& cornerValues) override;

        float calculateNormalizationFactor(uint32_t gridWidth) const;

    private:
        // CPU data.
        std::vector<std::vector<int8_t>> mValues;

        // Specs.
        uint32_t mCoarsestLODGridWidth = 0;
        float mCoarsestLODNormalizationFactor = 0.0f;
        float mNarrowBandThickness = 0.0f;

        // GPU data.
        std::vector<ref<Texture>> mNDSDFTextures;
        std::shared_ptr<SharedData> mpSharedData; ///< Shared data among all instances.
    };
}
