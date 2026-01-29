#pragma once
#include "VertexLayout.h"
#include "Buffer.h"
#include "Core/Macros.h"
#include "Core/Error.h"
#include "Core/Object.h"
#include <vector>

namespace Falcor
{
/**
 * Abstracts vertex array objects. A VAO must at least specify a primitive topology. You may additionally specify a number of vertex buffer
 * layouts corresponding to the number of vertex buffers to be bound. The number of vertex buffers to be bound must match the number
 * described in the layout.
 */
class FALCOR_API Vao : public Object
{
    FALCOR_OBJECT(Vao)
public:
    ~Vao() = default;

    /**
     * Primitive topology
     */
    enum class Topology
    {
        Undefined,
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip
    };

    struct ElementDesc
    {
        static constexpr uint32_t kInvalidIndex = -1;
        uint32_t vbIndex = kInvalidIndex;
        uint32_t elementIndex = kInvalidIndex;
    };

    using BufferVec = std::vector<ref<Buffer>>;

    /**
     * Create a new vertex array object.
     * @param primTopology The primitive topology.
     * @param pLayout The vertex layout description. Can be nullptr.
     * @param pVBs Array of pointers to vertex buffers. Number of buffers must match with pLayout.
     * @param pIB Pointer to the index buffer. Can be nullptr, in which case no index buffer will be bound.
     * @param ibFormat The resource format of the index buffer. Can be either R16Uint or R32Uint.
     * @return New object, or throws an exception on error.
     */
    static ref<Vao> create(
        Topology primTopology,
        ref<VertexLayout> pLayout = nullptr,
        const BufferVec& pVBs = BufferVec(),
        ref<Buffer> pIB = nullptr,
        ResourceFormat ibFormat = ResourceFormat::Unknown
    );

    /**
     * Get the vertex buffer count
     */
    uint32_t getVertexBuffersCount() const { return (uint32_t)mpVBs.size(); }

    /**
     * Get a vertex buffer
     */
    const ref<Buffer>& getVertexBuffer(uint32_t index) const
    {
        FALCOR_ASSERT(index < (uint32_t)mpVBs.size());
        return mpVBs[index];
    }

    /**
     * Get a vertex buffer layout
     */
    const ref<VertexLayout>& getVertexLayout() const { return mpVertexLayout; }

    /**
     * Return the vertex buffer index and the element index by its location.
     * If the element is not found, returns the default ElementDesc
     */
    ElementDesc getElementIndexByLocation(uint32_t elementLocation) const;

    /**
     * Get the index buffer
     */
    const ref<Buffer>& getIndexBuffer() const { return mpIB; }

    /**
     * Get the index buffer format
     */
    ResourceFormat getIndexBufferFormat() const { return mIbFormat; }

    /**
     * Get the primitive topology
     */
    Topology getPrimitiveTopology() const { return mTopology; }

protected:
    friend class RenderContext;

private:
    Vao(const BufferVec& pVBs, ref<VertexLayout> pLayout, ref<Buffer> pIB, ResourceFormat ibFormat, Topology primTopology);

    ref<VertexLayout> mpVertexLayout;
    BufferVec mpVBs;
    ref<Buffer> mpIB;
    void* mpPrivateData = nullptr;
    ResourceFormat mIbFormat;
    Topology mTopology;
};
} // namespace Falcor
