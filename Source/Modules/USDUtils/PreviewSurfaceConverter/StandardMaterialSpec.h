#pragma once
#include "Core/API/Texture.h"
#include "Core/API/Sampler.h"
#include "Scene/Material/Material.h"
#include "Scene/Material/StandardMaterial.h"
#include "Utils/Logger.h"
#include "USDUtils/USDUtils.h"
#include "USDUtils/ConvertedInput.h"

#include <string>

namespace Falcor
{
/**
 * Falcor::StandardMaterial specification.
 * Used to hold parameters needed to construct a StandardMaterial instance prior to actually doing so.
 * Can be hashed for use in, for example, a std::unordered_map, to avoid creating duplicate materials.
 */
struct StandardMaterialSpec
{
    StandardMaterialSpec() {}
    StandardMaterialSpec(const std::string& name) : name(name) {}

    bool operator==(const StandardMaterialSpec& o) const
    {
        return texTransform.transform == o.texTransform.transform && baseColor == o.baseColor && normal == o.normal &&
               metallic == o.metallic && roughness == o.roughness && opacity == o.opacity && emission == o.emission && disp == o.disp &&
               volumeAbsorption == o.volumeAbsorption && volumeScattering == o.volumeScattering && opacityThreshold == o.opacityThreshold &&
               ior == o.ior;
    }

    std::string name;
    ConvertedTexTransform texTransform;
    ConvertedInput baseColor;
    ConvertedInput normal;
    ConvertedInput metallic;
    ConvertedInput roughness;
    ConvertedInput opacity = {1.f};
    ConvertedInput emission;
    ConvertedInput disp;
    ConvertedInput volumeAbsorption;
    ConvertedInput volumeScattering;
    float opacityThreshold = 0.f;
    float ior = 1.5f;
};

/**
 * Hash object for use by hashed containers.
 */
class StandardMaterialSpecHash
{
public:
    size_t operator()(const StandardMaterialSpec& o) const
    {
        size_t hash = 0;
        hash_combine(
            hash,
            o.texTransform.transform,
            o.baseColor,
            o.normal,
            o.metallic,
            o.roughness,
            o.opacity,
            o.emission,
            o.disp,
            o.volumeAbsorption,
            o.volumeScattering,
            o.opacityThreshold,
            o.ior
        );
        return hash;
    }
};
} // namespace Falcor
