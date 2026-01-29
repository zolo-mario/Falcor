#include "Marker2DSet.h"
#include "Falcor.h"

void Falcor::Marker2DSet::addMarker(const Marker2DDataBlob& newMarker)
{
    if (mMarkers.size() >= mMaxMarkerCount)
    {
        FALCOR_THROW("Number of markers exceeds the maximum number allowed!");
    }

    mMarkers.push_back(newMarker);
    mDirtyBuffer = true;
}

void Falcor::Marker2DSet::clear()
{
    mMarkers.clear();
    mDirtyBuffer = true;
}

void Falcor::Marker2DSet::addSimpleMarker(
    const SDF2DShapeType markerType,
    const float size,
    const float2& pos,
    const float rotation,
    const float4& color
)
{
    Marker2DDataBlob markerBlob;
    ;
    markerBlob.type = markerType;
    SimpleMarker2DData* pSimpleMarker = reinterpret_cast<SimpleMarker2DData*>(markerBlob.payload.data);
    pSimpleMarker->transform.scale = size;
    pSimpleMarker->transform.rotation = rotation;
    pSimpleMarker->transform.translation = pos;
    pSimpleMarker->color = color;
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::addRoundedLine(const float2& posA, const float2& posB, const float lineWidth, const float4& color)
{
    Marker2DDataBlob markerBlob;
    markerBlob.type = SDF2DShapeType::RoundedLine;
    RoundedLineMarker2DData* pMarker = reinterpret_cast<RoundedLineMarker2DData*>(markerBlob.payload.data);
    pMarker->line.positionA = posA;
    pMarker->line.positionB = posB;
    pMarker->line.width = lineWidth;
    pMarker->color = color;
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::addVector(
    const float2& posA,
    const float2& posB,
    const float lineWidth,
    const float arrowHeight,
    const float4& color
)
{
    Marker2DDataBlob markerBlob;
    markerBlob.type = SDF2DShapeType::Vector;
    VectorMarker2DData* pMarker = reinterpret_cast<VectorMarker2DData*>(markerBlob.payload.data);
    pMarker->line.positionA = posA;
    pMarker->line.positionB = posB;
    pMarker->line.width = lineWidth;
    pMarker->arrowHeight = arrowHeight;
    pMarker->color = color;
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::addTriangle(const float2& posA, const float2& posB, const float2& posC, const float4& color)
{
    Marker2DDataBlob markerBlob;
    markerBlob.type = SDF2DShapeType::Triangle;
    TriangleMarker2DData* pMarker = reinterpret_cast<TriangleMarker2DData*>((void*)markerBlob.payload.data);
    pMarker->positionA = posA;
    pMarker->positionB = posB;
    pMarker->positionC = posC;
    pMarker->color = color;
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::addRoundedBox(
    const float2& pos,
    const float2& halfSides,
    const float radius,
    const float rotation,
    const float4& color
)
{
    Marker2DDataBlob markerBlob;
    markerBlob.type = SDF2DShapeType::RoundedBox;
    RoundedBoxMarker2DData* pMarker = reinterpret_cast<RoundedBoxMarker2DData*>(markerBlob.payload.data);
    pMarker->transform.translation = pos;
    pMarker->transform.scale = radius;
    pMarker->transform.rotation = rotation;
    pMarker->halfSides = halfSides;
    pMarker->color = color;
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::addMarkerOpMarker(
    const SDFOperationType op,
    const SDF2DShapeType typeA,
    const float2& posA,
    const float markerSizeA,
    const SDF2DShapeType typeB,
    const float2& posB,
    const float markerSizeB,
    const float4& color,
    const float4 dimmedColor
)
{
    Marker2DDataBlob markerBlob;
    markerBlob.type = SDF2DShapeType::MarkerOpMarker;
    MarkerOpMarker2DData* pMarker = reinterpret_cast<MarkerOpMarker2DData*>(markerBlob.payload.data);
    pMarker->operation = op;
    pMarker->markerA.position = posA;
    pMarker->markerA.size = markerSizeA;
    pMarker->markerA.type = typeA;
    pMarker->markerB.position = posB;
    pMarker->markerB.size = markerSizeB;
    pMarker->markerB.type = typeB;
    pMarker->color = color;
    pMarker->dimmedColor = dimmedColor;
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::addArrowFromTwoTris(
    const float2& startPos,
    const float2& endPos,
    const float headLength,
    const float headWidth,
    const float shaftWidth,
    const float4& color
)
{
    Marker2DDataBlob markerBlob;
    markerBlob.type = SDF2DShapeType::ArrowFromTwoTris;
    ArrowFromTwoTrisMarker2DData* pMarker = reinterpret_cast<ArrowFromTwoTrisMarker2DData*>(markerBlob.payload.data);
    pMarker->line.positionA = startPos;
    pMarker->line.positionB = endPos;
    pMarker->line.width = shaftWidth;
    pMarker->headLength = headLength;
    pMarker->headWidth = headWidth;
    pMarker->color = color;
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::addCircleSector(
    const float2& pos,
    const float rotation,
    const float angle,
    const float minRadius,
    const float maxRadius,
    const float4& color,
    const float4& borderColorXYZThicknessW,
    ExcludeBorderFlags excludeBorderFlags
)
{
    Marker2DDataBlob markerBlob;
    markerBlob.type = SDF2DShapeType::CircleSector;
    CircleSectorMarker2DData* pMarker = reinterpret_cast<CircleSectorMarker2DData*>(markerBlob.payload.data);
    pMarker->position = pos;
    pMarker->rotation = rotation;
    pMarker->angle = angle * 0.5f;
    pMarker->maxRadius = maxRadius;
    pMarker->minRadius = minRadius;
    pMarker->color = color;
    pMarker->borderColor = borderColorXYZThicknessW;
    pMarker->excludeBorders = uint32_t(excludeBorderFlags);
    addMarker(markerBlob);
}

void Falcor::Marker2DSet::bindShaderData(const ShaderVar& var)
{
    updateBuffer();

    var["markers"] = mpMarkerBuffer;
    var["markerCount"] = (uint32_t)mMarkers.size();
}

void Falcor::Marker2DSet::updateBuffer()
{
    if (!mpMarkerBuffer || mDirtyBuffer)
    {
        mDirtyBuffer = false;

        // Invalidate buffer if it is empty.
        if (mMarkers.empty())
        {
            mpMarkerBuffer = nullptr;
        }
        // Create a new buffer if it does not exist or if the size is too small for the markers.
        else if (!mpMarkerBuffer || mpMarkerBuffer->getElementCount() < (uint32_t)mMarkers.size())
        {
            mpMarkerBuffer = mpDevice->createStructuredBuffer(
                sizeof(Marker2DDataBlob),
                (uint32_t)mMarkers.size(),
                ResourceBindFlags::ShaderResource,
                MemoryType::DeviceLocal,
                mMarkers.data(),
                false
            );
            mpMarkerBuffer->setName("Marker2DSet::mpMarkerBuffer");
        }
        // Else update the existing buffer.
        else
        {
            mpMarkerBuffer->setBlob(mMarkers.data(), 0, mMarkers.size() * sizeof(Marker2DDataBlob));
        }
    }
}
