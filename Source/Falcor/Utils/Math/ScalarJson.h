#pragma once

#include "Float16.h"

#include "Core/Error.h"

#include <nlohmann/json.hpp>

namespace Falcor
{
namespace math
{

inline void to_json(nlohmann::json& j, const float16_t& v)
{
    j = double(v);
}

inline void from_json(const nlohmann::json& j, float16_t& v)
{
    FALCOR_ASSERT(j.is_number());
    double d;
    j.get_to(d);
    v = float16_t(d);
}

} // namespace math

} // namespace Falcor
