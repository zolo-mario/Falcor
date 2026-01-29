#pragma once

#include "Core/Error.h"
#include "Utils/Math/Vector.h"
#include <array>

namespace Falcor
{

struct SerializedMaterialParams : public std::array<float, 20>
{
    static constexpr size_t kParamCount = 20;

    void write(float value, size_t offset)
    {
        FALCOR_ASSERT(offset <= size());
        (*this)[offset] = value;
    }

    template<int N>
    void write(math::vector<float, N> value, size_t offset)
    {
        FALCOR_ASSERT(offset + N <= size());
        for (size_t i = 0; i < N; ++i)
            (*this)[offset + i] = value[i];
    }

    void read(float& value, size_t offset) const
    {
        FALCOR_ASSERT(offset <= size());
        value = (*this)[offset];
    }

    template<int N>
    void read(math::vector<float, N>& value, size_t offset) const
    {
        FALCOR_ASSERT(offset + N <= size());
        for (size_t i = 0; i < N; ++i)
            value[i] = (*this)[offset + i];
    }
};

} // namespace Falcor
