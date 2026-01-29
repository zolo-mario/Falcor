#pragma once

#include "Falcor.h"
#include "Marker2DSet.h"

namespace Falcor
{
class SelectionWheel
{
public:
    static constexpr uint32_t kInvalidIndex = -1;

    struct Desc
    {
        std::vector<uint32_t> sectorGroups; ///< Describes how many sectors each group should have in the selection wheel.
        float2 position;                    ///< Center position of the selection wheel.
        float minRadius;                    ///< The minimum radius of the selection wheel.
        float maxRadius;                    ///< The maximum radius of the selection wheel.
        float4 baseColor;                   ///< The base color of the selection wheel.
        float4 highlightColor;              ///< The highlight color for the selected sector.
        float4 lineColor;                   ///< The color of the lines that separate sectors and groups.
        float borderWidth;                  ///< Thickness of the border in pixels.
    };

    SelectionWheel(Marker2DSet& marker2DSet) : mMarker2DSet(marker2DSet) {}

    void update(const float2& mousePos, const Desc& description);

    bool isMouseOnSector(const float2& mousePos, uint32_t groupIndex, uint32_t sectorIndex);
    bool isMouseOnGroup(const float2& mousePos, uint32_t groupIndex, uint32_t& sectorIndex);

    float2 getCenterPositionOfSector(uint32_t groupIndex, uint32_t sectorIndex);

    float getAngleOfSectorInGroup(uint32_t groupIndex);
    float getRotationOfSector(uint32_t groupIndex, uint32_t sectorIndex);
    float getGroupAngle();

private:
    void computeMouseAngleAndDirLength(const float2& mousePos, float& mouseAngle, float& dirLength);
    void computeGroupAndSectorIndexFromAngle(float mouseAngle, uint32_t& groupIndex, uint32_t& sectorIndex);

    /**
     * Specialized add function for this class. When adding a circle sector this will allow for cutting off some parts of the sector's
     * sides.
     * @param[in] rotation The starting angle of the sector. It is the left side of the sector.
     * @param[in] angle The angle that describes the circle sector.
     * @param[in] color The color of the circle sector.
     * @param[in] margin The cutoff of one of the sides. If positive, it will cut off from the left side, if negative, from the right, and
     * if zero, no cut off.
     * @param[in] marginOnBothSides If true, does the cut off on both sides instead of one. The margin variable need to be positive if this
     * is true.
     * @param[in] excludeBorderFlags Flags for which borders should be excluded from rendering.
     */
    void addCircleSector(
        float rotation,
        float angle,
        const float4& color,
        const float4& borderColor,
        float margin = 0.f,
        bool marginOnBothSides = false,
        ExcludeBorderFlags excludeBorderFlags = ExcludeBorderFlags::None
    );

private:
    Desc mDescription;
    Marker2DSet& mMarker2DSet;
};
} // namespace Falcor
