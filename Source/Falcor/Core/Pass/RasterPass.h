#pragma once
#include "BaseGraphicsPass.h"
#include "Core/Macros.h"
#include "Core/Program/Program.h"
#include <filesystem>
#include <string>

namespace Falcor
{
class FALCOR_API RasterPass : public BaseGraphicsPass
{
public:
    /**
     * Create a new object.
     * @param[in] pDevice GPU devuce.
     * @param[in] path Program file path.
     * @param[in] vsEntry Vertex shader entry point. If this string is empty (""), it will use a default vertex shader which transforms and
     * outputs all default vertex attributes.
     * @param[in] psEntry Pixel shader entry point
     * @param[in] programDefines Optional list of macro definitions to set into the program. The macro definitions will be set on all shader
     * stages.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<RasterPass> create(
        ref<Device> pDevice,
        const std::filesystem::path& path,
        const std::string& vsEntry,
        const std::string& psEntry,
        const DefineList& defines = DefineList()
    );

    /**
     * Create a new object.
     * @param[in] pDevice GPU devuce.
     * @param[in] progDesc The program description.
     * @param[in] programDefines Optional list of macro definitions to set into the program. The macro definitions will be set on all shader
     * stages.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<RasterPass> create(ref<Device> pDevice, const ProgramDesc& desc, const DefineList& defines = DefineList());

    /**
     * Ordered draw call.
     * @param[in] vertexCount Number of vertices to draw
     * @param[in] startVertexLocation The location of the first vertex to read from the vertex buffers (offset in vertices)
     */
    void draw(RenderContext* pRenderContext, uint32_t vertexCount, uint32_t startVertexLocation);

    /**
     * Indexed draw call.
     * @param[in] indexCount Number of indices to draw
     * @param[in] startIndexLocation The location of the first index to read from the index buffer (offset in indices)
     * @param[in] baseVertexLocation A value which is added to each index before reading a vertex from the vertex buffer
     */
    void drawIndexed(RenderContext* pRenderContext, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);

protected:
    RasterPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines);
};
} // namespace Falcor
