/**
 * Most of this code is derived from the GLM library at https://github.com/g-truc/glm
 *
 * License: https://github.com/g-truc/glm/blob/master/copying.txt
 */

#pragma once

#include "MatrixTypes.h"
#include "ScalarJson.h"

#include "Core/Error.h"

#include <nlohmann/json.hpp>

namespace Falcor
{
namespace math
{

template<typename T, int RowCount, int ColCount>
void to_json(nlohmann::json& j, const matrix<T, RowCount, ColCount>& v)
{
    j = nlohmann::json::array();
    for (int i = 0; i < RowCount * ColCount; ++i)
        j.push_back(v.data()[i]);
}

template<typename T, int RowCount, int ColCount>
void from_json(const nlohmann::json& j, matrix<T, RowCount, ColCount>& v)
{
    FALCOR_ASSERT(j.is_array() && j.size() == RowCount * ColCount);
    for (int i = 0; i < RowCount * ColCount; ++i)
        j[i].get_to(v.data()[i]);
}

} // namespace math
} // namespace Falcor
