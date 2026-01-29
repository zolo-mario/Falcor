#include "Scene/SDFs/SDFGridNoDefines.slangh"

 /** The scene sets the SCENE_SDF_GRID_IMPLEMENTATION define to select what SDFGridHitData implementation should be used.
 */

struct SDFGridHitData
{
#if SCENE_SDF_GRID_IMPLEMENTATION == SCENE_SDF_GRID_IMPLEMENTATION_NDSDF
    uint lod;
#elif SCENE_SDF_GRID_IMPLEMENTATION == SCENE_SDF_GRID_IMPLEMENTATION_SVS || SCENE_SDF_GRID_IMPLEMENTATION == SCENE_SDF_GRID_IMPLEMENTATION_SBS
    uint primitiveID;
#elif SCENE_SDF_GRID_IMPLEMENTATION == SCENE_SDF_GRID_IMPLEMENTATION_SVO
    uint svoIndex;
#endif

    float hitT;
};
