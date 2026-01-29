#pragma once
#include "Core/API/Texture.h"

namespace Falcor
{
    struct BrickedGrid
    {
        ref<Texture> range;
        ref<Texture> indirection;
        ref<Texture> atlas;
    };
}
