#pragma once
#include "D3D12Handles.h"
#include "D3D12DescriptorSet.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/API/ShaderResourceType.h"
#include <d3d12.h>
#include <memory>
#include <vector>

namespace Falcor
{
class ProgramReflection;
class EntryPointGroupReflection;
class CopyContext;

/**
 * The root signature defines what resources are bound to the pipeline.
 *
 * The layout is defined by traversing the ParameterBlock hierarchy
 * of a program to find all required root parameters. These are then
 * arranged consecutively in the following order in the root signature:
 *
 * 1. descriptor tables
 * 2. root descriptors
 * 3. root constants
 *
 * The get*BaseIndex() functions return the base index of the
 * corresponding root parameter type in the root signature.
 */
class FALCOR_API D3D12RootSignature : public Object
{
    FALCOR_OBJECT(D3D12RootSignature)
public:
    using ApiHandle = ID3D12RootSignaturePtr;
    using DescType = ShaderResourceType;

    struct RootDescriptorDesc
    {
        DescType type;
        uint32_t regIndex;
        uint32_t spaceIndex;
        ShaderVisibility visibility;
    };

    struct RootConstantsDesc
    {
        uint32_t regIndex;
        uint32_t spaceIndex;
        uint32_t count;
    };

    class FALCOR_API Desc
    {
    public:
        Desc& addDescriptorSet(const D3D12DescriptorSetLayout& setLayout);
        Desc& addRootDescriptor(DescType type, uint32_t regIndex, uint32_t spaceIndex, ShaderVisibility visibility = ShaderVisibility::All);
        Desc& addRootConstants(uint32_t regIndex, uint32_t spaceIndex, uint32_t count); // #SHADER_VAR Make sure this works with the
                                                                                        // reflectors

        size_t getSetsCount() const { return mSets.size(); }
        const D3D12DescriptorSetLayout& getSet(size_t index) const { return mSets[index]; }

        size_t getRootDescriptorCount() const { return mRootDescriptors.size(); }
        const RootDescriptorDesc& getRootDescriptorDesc(size_t index) const { return mRootDescriptors[index]; }

        size_t getRootConstantCount() const { return mRootConstants.size(); }
        const RootConstantsDesc& getRootConstantDesc(size_t index) const { return mRootConstants[index]; }

    private:
        friend class D3D12RootSignature;

        std::vector<D3D12DescriptorSetLayout> mSets;
        std::vector<RootDescriptorDesc> mRootDescriptors;
        std::vector<RootConstantsDesc> mRootConstants;
    };

    ~D3D12RootSignature();

    /**
     * Create a root signature.
     * @param[in] pDevice GPU device.
     * @param[in] desc Root signature description.
     * @return New object, or throws an exception if creation failed.
     */
    static ref<D3D12RootSignature> create(ref<Device> pDevice, const Desc& desc);

    /**
     * Create a root signature from program reflection.
     * @param[in] pReflection Reflection object.
     * @return New object, or throws an exception if creation failed.
     */
    static ref<D3D12RootSignature> create(ref<Device> pDevice, const ProgramReflection* pReflection);

    const ApiHandle& getApiHandle() const { return mApiHandle; }

    size_t getDescriptorSetCount() const { return mDesc.mSets.size(); }
    const D3D12DescriptorSetLayout& getDescriptorSet(size_t index) const { return mDesc.mSets[index]; }

    uint32_t getDescriptorSetBaseIndex() const { return 0; }
    uint32_t getRootDescriptorBaseIndex() const { return getDescriptorSetBaseIndex() + (uint32_t)mDesc.mSets.size(); }
    uint32_t getRootConstantBaseIndex() const { return getRootDescriptorBaseIndex() + (uint32_t)mDesc.mRootDescriptors.size(); }

    uint32_t getSizeInBytes() const { return mSizeInBytes; }
    uint32_t getElementByteOffset(uint32_t elementIndex) { return mElementByteOffset[elementIndex]; }

    void bindForGraphics(CopyContext* pCtx);
    void bindForCompute(CopyContext* pCtx);

    const Desc& getDesc() const { return mDesc; }

protected:
    D3D12RootSignature(ref<Device> pDevice, const Desc& desc);
    void createApiHandle(ID3DBlobPtr pSigBlob);
    ref<Device> mpDevice;
    Desc mDesc;
    ApiHandle mApiHandle;

    uint32_t mSizeInBytes;
    std::vector<uint32_t> mElementByteOffset;
};
} // namespace Falcor
