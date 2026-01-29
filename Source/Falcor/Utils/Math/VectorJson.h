#pragma once

#include "VectorTypes.h"
#include "ScalarJson.h"

#include "Core/Error.h"

#include <nlohmann/json.hpp>

namespace Falcor
{
namespace math
{

template<typename T, int N>
void to_json(nlohmann::json& j, const vector<T, N>& v)
{
    j = nlohmann::json::array();
    for (int i = 0; i < N; ++i)
        j.push_back(v[i]);
}

template<typename T, int N>
void from_json(const nlohmann::json& j, vector<T, N>& v)
{
    FALCOR_ASSERT(j.is_array() && j.size() == N);
    for (int i = 0; i < N; ++i)
        j[i].get_to(v[i]);
}

} // namespace math

} // namespace Falcor
