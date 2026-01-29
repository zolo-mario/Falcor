#pragma once
#include "Core/Macros.h"
#include "MaterialTypes.slang"
#include "MaterialParamLayout.h"
#include <fmt/format.h>

namespace Falcor
{
/** Registers a new material type with the given name. Returns the existing material type if name is already registered.
    The type name will be used by the system for symbols in the generated shader code and must not contain whitespaces etc.
    The first returned MaterialType is one past `MaterialType::BuiltinCount`.
    This operation is thread safe.
    \param[in] typeName Material type name.
    \return Material type.
*/
FALCOR_API MaterialType registerMaterialType(std::string typeName);

/** Get the material type name for the given type.
    This operation is thread safe.
    \param[in] type Material type.
    \return Material type name.
*/
FALCOR_API std::string to_string(MaterialType type);

/** Returns the total number of registered material types. This includes the `MaterialType::BuiltinCount`.
    This operation is thread safe.
    \return Total number of registered material types.
*/
FALCOR_API size_t getMaterialTypeCount();

/** Return the material parameter layout of serialized material parameters (for differentiable materials).
*/
FALCOR_API MaterialParamLayout getMaterialParamLayout(MaterialType type);
}

template<>
struct fmt::formatter<Falcor::MaterialType>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(Falcor::MaterialType materialType, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{0}", Falcor::to_string(materialType));
    }
};
