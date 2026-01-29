#pragma once
#include "Core/Macros.h"
#include "Core/API/Formats.h"
#include "Core/Program/DefineList.h"

namespace Falcor
{
    class Scene;

    /** Host side utility to setup the bit allocations for device side HitInfo.

        By default, HitInfo is encoded in 128 bits. There is a compression mode
        where HitInfo is encoded in 64 bits. This mode is only available in
        scenes that exclusively use triangle meshes and are small enough so
        the header information fits in 32 bits. In compression mode,
        barycentrics are quantized to 16 bit unorms.

        See HitInfo.slang for more information.
    */
    class FALCOR_API HitInfo
    {
    public:
        static constexpr uint32_t kMaxPackedSizeInBytes = 16;
        static constexpr ResourceFormat kDefaultFormat = ResourceFormat::RGBA32Uint;

        HitInfo() = default;
        HitInfo(const Scene& scene, bool useCompression = false) { init(scene, useCompression); }
        void init(const Scene& scene, bool useCompression);

        /** Returns defines needed packing/unpacking a HitInfo struct.
        */
        DefineList getDefines() const;

        /** Returns the resource format required for encoding packed hit information.
        */
        ResourceFormat getFormat() const;

    private:
        bool mUseCompression = false;       ///< Store in compressed format (64 bits instead of 128 bits).

        uint32_t mTypeBits = 0;             ///< Number of bits to store hit type.
        uint32_t mInstanceIDBits = 0;       ///< Number of bits to store instance ID.
        uint32_t mPrimitiveIndexBits = 0;   ///< Number of bits to store primitive index.
    };
}
