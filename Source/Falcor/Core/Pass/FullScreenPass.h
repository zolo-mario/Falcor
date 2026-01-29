#pragma once
#include "BaseGraphicsPass.h"
#include "Core/Macros.h"
#include "Core/Program/Program.h"
#include <filesystem>
#include <memory>

namespace Falcor
{
class FALCOR_API FullScreenPass : public BaseGraphicsPass
{
public:
    struct SharedData;

    virtual ~FullScreenPass();

    /**
     * Create a new fullscreen pass from file.
     * @param[in] pDevice GPU device.
     * @param[in] path Pixel shader file path. This method expects a pixel shader named "main()" in the file.
     * @param[in] defines Optional list of macro definitions to set into the program.
     * @param[in] viewportMask Optional value to initialize viewport mask with. Useful for multi-projection passes.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<FullScreenPass> create(
        ref<Device> pDevice,
        const std::filesystem::path& path,
        const DefineList& defines = DefineList(),
        uint32_t viewportMask = 0
    );

    /**
     * Create a new fullscreen pass.
     * @param[in] pDevice GPU device.
     * @param[in] desc The program description.
     * @param[in] defines Optional list of macro definitions to set into the program.
     * @param[in] viewportMask Optional value to initialize viewport mask with. Useful for multi-projection passes.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<FullScreenPass> create(
        ref<Device> pDevice,
        const ProgramDesc& desc,
        const DefineList& defines = DefineList(),
        uint32_t viewportMask = 0
    );

    /**
     * Execute the pass using an FBO
     * @param[in] pRenderContext The render context.
     * @param[in] pFbo The target FBO
     * @param[in] autoSetVpSc If true, the pass will set the viewports and scissors to match the FBO size. If you want to override the VP or
     * SC, get the state by calling `getState()`, bind the SC and VP yourself and set this arg to false
     */
    virtual void execute(RenderContext* pRenderContext, const ref<Fbo>& pFbo, bool autoSetVpSc = true) const;

protected:
    FullScreenPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines);

private:
    std::shared_ptr<SharedData> mpSharedData;
};
} // namespace Falcor
