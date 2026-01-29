#include "RtStateObject.h"
#include "Device.h"
#include "GFXAPI.h"
#include "Core/Program/Program.h"

namespace Falcor
{

RtStateObject::RtStateObject(ref<Device> pDevice, const RtStateObjectDesc& desc) : mpDevice(pDevice), mDesc(desc)
{
    auto pKernels = getKernels();
    gfx::RayTracingPipelineStateDesc rtpDesc = {};
    std::vector<gfx::HitGroupDesc> hitGroups;
    // Loop over the hitgroups
    for (const auto& pEntryPointGroup : pKernels->getUniqueEntryPointGroups())
    {
        if (pEntryPointGroup->getType() == EntryPointGroupKernels::Type::RtHitGroup)
        {
            const EntryPointKernel* pIntersection = pEntryPointGroup->getKernel(ShaderType::Intersection);
            const EntryPointKernel* pAhs = pEntryPointGroup->getKernel(ShaderType::AnyHit);
            const EntryPointKernel* pChs = pEntryPointGroup->getKernel(ShaderType::ClosestHit);

            gfx::HitGroupDesc hitgroupDesc = {};
            hitgroupDesc.anyHitEntryPoint = pAhs ? pAhs->getEntryPointName().c_str() : nullptr;
            hitgroupDesc.closestHitEntryPoint = pChs ? pChs->getEntryPointName().c_str() : nullptr;
            hitgroupDesc.intersectionEntryPoint = pIntersection ? pIntersection->getEntryPointName().c_str() : nullptr;
            hitgroupDesc.hitGroupName = pEntryPointGroup->getExportName().c_str();
            hitGroups.push_back(hitgroupDesc);
        }
    }

    rtpDesc.hitGroupCount = (uint32_t)hitGroups.size();
    rtpDesc.hitGroups = hitGroups.data();
    rtpDesc.maxRecursion = mDesc.maxTraceRecursionDepth;

    static_assert((uint32_t)gfx::RayTracingPipelineFlags::SkipProcedurals == (uint32_t)RtPipelineFlags::SkipProceduralPrimitives);
    static_assert((uint32_t)gfx::RayTracingPipelineFlags::SkipTriangles == (uint32_t)RtPipelineFlags::SkipTriangles);

    rtpDesc.flags = (gfx::RayTracingPipelineFlags::Enum)mDesc.pipelineFlags;
    auto rtProgram = dynamic_cast<Program*>(mDesc.pProgramKernels->getProgramVersion()->getProgram());
    FALCOR_ASSERT(rtProgram);
    rtpDesc.maxRayPayloadSize = rtProgram->getDesc().maxPayloadSize;
    rtpDesc.maxAttributeSizeInBytes = rtProgram->getDesc().maxAttributeSize;
    rtpDesc.program = mDesc.pProgramKernels->getGfxProgram();

    FALCOR_GFX_CALL(mpDevice->getGfxDevice()->createRayTracingPipelineState(rtpDesc, mGfxPipelineState.writeRef()));

    // Get shader identifiers.
    // In GFX, a shader identifier is just the entry point group name.
    for (const auto& pEntryPointGroup : pKernels->getUniqueEntryPointGroups())
    {
        mEntryPointGroupExportNames.push_back(pEntryPointGroup->getExportName());
    }
}

RtStateObject::~RtStateObject()
{
    mpDevice->releaseResource(mGfxPipelineState);
}

} // namespace Falcor
