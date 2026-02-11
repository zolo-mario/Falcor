# Raster - Rasterization Shaders

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Raster.slang** - Rasterization shader

### External Dependencies

- **Scene/VertexAttrib** - Vertex attribute definitions
- **Scene/Shading** - Shading system

## Module Overview

Raster provides rasterization shaders for rendering triangle geometry. It includes vertex shader input/output structures, a default vertex shader, and helper functions for preparing vertex data, shading data, and alpha testing. The module supports dynamic geometry with previous frame positions for motion vectors, configurable interpolation modes, and integration with the scene and material systems.

## Component Specifications

### VSIn Structure

**Purpose**: Vertex shader input structure.

**Key Features**:
- Packed vertex attributes for efficient memory usage
- Instance ID for instanced rendering
- Vertex ID for vertex-specific operations
- Unpack method for converting packed data

**Fields**:
- [`pos`](Source/Falcor/Scene/Raster.slang:35) - `float3 : POSITION` - Vertex position
- [`packedNormalTangentCurveRadius`](Source/Falcor/Scene/Raster.slang:36) - `float3 : PACKED_NORMAL_TANGENT_CURVE_RADIUS` - Packed normal, tangent, and curve radius
- [`texC`](Source/Falcor/Scene/Raster.slang:37) - `float2 : TEXCOORD` - Texture coordinate
- [`instanceID`](Source/Falcor/Scene/Raster.slang:40) - `uint : DRAW_ID` - Instance ID for instanced rendering
- [`vertexID`](Source/Falcor/Scene/Raster.slang:43) - `uint : SV_VertexID` - Vertex ID (system value)

**Methods**:
- [`unpack()`](Source/Falcor/Scene/Raster.slang:45) - Unpack packed vertex data
  - Creates [`PackedStaticVertexData`](Source/Falcor/Scene/Raster.slang:47) structure
  - Sets position: [`v.position = pos`](Source/Falcor/Scene/Raster.slang:48)
  - Sets packed normal/tangent: [`v.packedNormalTangentCurveRadius = packedNormalTangentCurveRadius`](Source/Falcor/Scene/Raster.slang:49)
  - Sets texture coordinate: [`v.texCrd = texC`](Source/Falcor/Scene/Raster.slang:50)
  - Returns unpacked [`StaticVertexData`](Source/Falcor/Scene/Raster.slang:51) via [`v.unpack()`](Source/Falcor/Scene/Raster.slang:51)

**Packed Attributes**:
- Packed normal, tangent, and curve radius in single float3
- Efficient memory usage
- Unpacked on-demand
- Reduces vertex buffer size

### VSOut Structure

**Purpose**: Vertex shader output structure.

**Key Features**:
- Interpolated vertex attributes for fragment shader
- Configurable interpolation mode
- Per-triangle data (instance ID, material ID)
- Previous frame position for motion vectors

**Fields**:
- [`normalW`](Source/Falcor/Scene/Raster.slang:61) - `INTERPOLATION_MODE float3 : NORMAL` - Shading normal in world space (not normalized!)
- [`tangentW`](Source/Falcor/Scene/Raster.slang:62) - `INTERPOLATION_MODE float4 : TANGENT` - Shading tangent in world space (not normalized!)
- [`texC`](Source/Falcor/Scene/Raster.slang:63) - `INTERPOLATION_MODE float2 : TEXCRD` - Texture coordinate
- [`posW`](Source/Falcor/Scene/Raster.slang:64) - `INTERPOLATION_MODE float3 : POSW` - Position in world space
- [`prevPosH`](Source/Falcor/Scene/Raster.slang:65) - `INTERPOLATION_MODE float4 : PREVPOSH` - Position in clip space for the previous frame
- [`instanceID`](Source/Falcor/Scene/Raster.slang:68) - `nointerpolation GeometryInstanceID : INSTANCE_ID` - Geometry instance ID (per-triangle)
- [`materialID`](Source/Falcor/Scene/Raster.slang:69) - `nointerpolation uint : MATERIAL_ID` - Material ID (per-triangle)
- [`posH`](Source/Falcor/Scene/Raster.slang:71) - `float4 : SV_POSITION` - Position in clip space

**Interpolation Mode**:
- Configurable via `INTERPOLATION_MODE` macro
- Default: `linear` interpolation
- Can be changed for different interpolation modes
- Applies to interpolated attributes

**Per-Triangle Data**:
- [`nointerpolation`](Source/Falcor/Scene/Raster.slang:68) modifier for instance ID
- [`nointerpolation`](Source/Falcor/Scene/Raster.slang:69) modifier for material ID
- Not interpolated across triangle
- Constant across triangle

### defaultVS Function

**Purpose**: Default vertex shader.

**Signature**:
```slang
VSOut defaultVS(VSIn vIn)
```

**Parameters**:
- [`vIn`](Source/Falcor/Scene/Raster.slang:74) - `VSIn` - Vertex shader input

**Implementation**:
1. **Initialize Output**:
   - Creates [`VSOut`](Source/Falcor/Scene/Raster.slang:76) structure

2. **Instance ID**:
   - Gets instance ID: [`const GeometryInstanceID instanceID = { vIn.instanceID }`](Source/Falcor/Scene/Raster.slang:77)

3. **World Space Position**:
   - Gets world matrix: [`float4x4 worldMat = gScene.getWorldMatrix(instanceID)`](Source/Falcor/Scene/Raster.slang:79)
   - Transforms position to world space: [`float3 posW = mul(worldMat, float4(vIn.pos, 1.f)).xyz`](Source/Falcor/Scene/Raster.slang:80)
   - Stores world space position: [`vOut.posW = posW`](Source/Falcor/Scene/Raster.slang:81)

4. **Clip Space Position**:
   - Transforms to clip space: [`vOut.posH = mul(gScene.camera.getViewProj(), float4(posW, 1.f))`](Source/Falcor/Scene/Raster.slang:82)
   - Uses camera view-projection matrix

5. **Instance and Material ID**:
   - Stores instance ID: [`vOut.instanceID = instanceID`](Source/Falcor/Scene/Raster.slang:84)
   - Gets material ID: [`vOut.materialID = gScene.getMaterialID(instanceID)`](Source/Falcor/Scene/Raster.slang:85)

6. **Texture Coordinate**:
   - Passes through texture coordinate: [`vOut.texC = vIn.texC`](Source/Falcor/Scene/Raster.slang:87)

7. **Normal**:
   - Unpacks vertex data: [`vIn.unpack().normal`](Source/Falcor/Scene/Raster.slang:88)
   - Transforms normal to world space: [`vOut.normalW = mul(gScene.getInverseTransposeWorldMatrix(instanceID), vIn.unpack().normal)`](Source/Falcor/Scene/Raster.slang:88)
   - Uses inverse transpose world matrix for correct normal transformation

8. **Tangent**:
   - Unpacks tangent: [`float4 tangent = vIn.unpack().tangent`](Source/Falcor/Scene/Raster.slang:89)
   - Transforms tangent to world space: [`vOut.tangentW = float4(mul((float3x3)gScene.getWorldMatrix(instanceID), tangent.xyz), tangent.w)`](Source/Falcor/Scene/Raster.slang:90)
   - Preserves tangent.w (orientation)

9. **Previous Frame Position**:
   - Gets previous position: [`float3 prevPos = vIn.pos`](Source/Falcor/Scene/Raster.slang:93)
   - Gets geometry instance data: [`GeometryInstanceData instance = gScene.getGeometryInstance(instanceID)`](Source/Falcor/Scene/Raster.slang:94)
   - Checks if dynamic: [`if (instance.isDynamic())`](Source/Falcor/Scene/Raster.slang:95)
   - Gets previous vertex index: [`uint prevVertexIndex = gScene.meshes[instance.geometryID].prevVbOffset + vIn.vertexID`](Source/Falcor/Scene/Raster.slang:97)
   - Gets previous vertex position: [`prevPos = gScene.prevVertices[prevVertexIndex].position`](Source/Falcor/Scene/Raster.slang:98)
   - Transforms to previous world space: [`float3 prevPosW = mul(gScene.getPrevWorldMatrix(instanceID), float4(prevPos, 1.f)).xyz`](Source/Falcor/Scene/Raster.slang:100)
   - Transforms to previous clip space: [`vOut.prevPosH = mul(gScene.camera.data.prevViewProjMatNoJitter, float4(prevPosW, 1.f))`](Source/Falcor/Scene/Raster.slang:101)
   - Uses previous view-projection matrix without jitter

10. **Return Output**:
    - Returns [`VSOut`](Source/Falcor/Scene/Raster.slang:103) structure

**Dynamic Geometry Support**:
- Checks if geometry instance is dynamic
- Uses previous vertex positions for dynamic geometry
- Uses current vertex positions for static geometry
- Enables motion vectors for dynamic geometry

### prepareVertexData Function

**Purpose**: Setup vertex data based on interpolated vertex attributes.

**Signature**:
```slang
VertexData prepareVertexData(VSOut vsOut, float3 faceNormalW)
```

**Parameters**:
- [`vsOut`](Source/Falcor/Scene/Raster.slang:111) - `VSOut` - Interpolated vertex attributes
- [`faceNormalW`](Source/Falcor/Scene/Raster.slang:111) - `float3` - Face normal in world space (normalized)

**Implementation**:
1. **Initialize VertexData**:
   - Creates [`VertexData`](Source/Falcor/Scene/Raster.slang:113) structure

2. **Position**:
   - Stores world space position: [`v.posW = vsOut.posW`](Source/Falcor/Scene/Raster.slang:114)

3. **Texture Coordinate**:
   - Stores texture coordinate: [`v.texC = vsOut.texC`](Source/Falcor/Scene/Raster.slang:115)

4. **Normal**:
   - Normalizes interpolated normal: [`v.normalW = normalize(vsOut.normalW)`](Source/Falcor/Scene/Raster.slang:116)
   - Normal is not normalized in VSOut (interpolated)

5. **Face Normal**:
   - Stores face normal: [`v.faceNormalW = faceNormalW`](Source/Falcor/Scene/Raster.slang:117)
   - Face normal is already normalized

6. **Tangent**:
   - Normalizes interpolated tangent: [`v.tangentW.xyz = normalize(vsOut.tangentW.xyz)`](Source/Falcor/Scene/Raster.slang:118)
   - Tangent is not normalized in VSOut (interpolated)
   - Preserves tangent.w sign: [`v.tangentW.w = sign(vsOut.tangentW.w)`](Source/Falcor/Scene/Raster.slang:119)
   - Sign preserves zero to indicate invalid tangent

7. **Return VertexData**:
   - Returns [`VertexData`](Source/Falcor/Scene/Raster.slang:120) structure

**Normalization**:
- Normalizes interpolated normal (not normalized in VSOut)
- Normalizes interpolated tangent (not normalized in VSOut)
- Face normal is already normalized
- Preserves tangent.w sign for invalid tangent indication

### prepareShadingData Function

**Purpose**: Prepare ShadingData struct based on VSOut.

**Signature**:
```slang
ShadingData prepareShadingData(VSOut vsOut, uint triangleIndex, float3 viewDir)
```

**Parameters**:
- [`vsOut`](Source/Falcor/Scene/Raster.slang:129) - `VSOut` - Interpolated vertex attributes
- [`triangleIndex`](Source/Falcor/Scene/Raster.slang:129) - `uint` - Triangle index
- [`viewDir`](Source/Falcor/Scene/Raster.slang:129) - `float3` - View direction (points from shading point towards viewer)

**Implementation**:
1. **Get Face Normal**:
   - Gets face normal: [`float3 faceNormal = gScene.getFaceNormalW(vsOut.instanceID, triangleIndex)`](Source/Falcor/Scene/Raster.slang:131)
   - Face normal in world space (normalized)

2. **Prepare Vertex Data**:
   - Calls [`prepareVertexData(vsOut, faceNormal)`](Source/Falcor/Scene/Raster.slang:132)
   - Creates [`VertexData`](Source/Falcor/Scene/Raster.slang:132) structure

3. **Prepare Shading Data**:
   - Calls [`gScene.materials.prepareShadingData(v, vsOut.materialID, viewDir)`](Source/Falcor/Scene/Raster.slang:133)
   - Prepares shading data from material system
   - Uses vertex data, material ID, and view direction

4. **Return ShadingData**:
   - Returns [`ShadingData`](Source/Falcor/Scene/Raster.slang:133) structure

**Integration**:
- Integrates with scene system for face normal
- Integrates with material system for shading data
- Uses vertex data from interpolated attributes
- Uses view direction for shading

### alphaTest Function

**Purpose**: Evaluate alpha testing based on VSOut.

**Signature**:
```slang
bool alphaTest<L:ITextureSampler>(VSOut vsOut, uint triangleIndex, L lod)
```

**Parameters**:
- [`vsOut`](Source/Falcor/Scene/Raster.slang:142) - `VSOut` - Interpolated vertex attributes
- [`triangleIndex`](Source/Falcor/Scene/Raster.slang:142) - `uint` - Triangle index
- [`lod`](Source/Falcor/Scene/Raster.slang:142) - `L` - Method for computing texture level-of-detail, must implement `ITextureSampler` interface

**Implementation**:
1. **Get Face Normal**:
   - Gets face normal: [`float3 faceNormal = gScene.getFaceNormalW(vsOut.instanceID, triangleIndex)`](Source/Falcor/Scene/Raster.slang:144)
   - Face normal in world space (normalized)

2. **Prepare Vertex Data**:
   - Calls [`prepareVertexData(vsOut, faceNormal)`](Source/Falcor/Scene/Raster.slang:145)
   - Creates [`VertexData`](Source/Falcor/Scene/Raster.slang:145) structure

3. **Alpha Test**:
   - Calls [`gScene.materials.alphaTest(v, vsOut.materialID, lod)`](Source/Falcor/Scene/Raster.slang:146)
   - Performs alpha test from material system
   - Returns true if hit should be ignored/discarded

4. **Return Result**:
   - Returns `bool` - True if hit should be ignored/discarded

**Template Parameter**:
- `L:ITextureSampler` - Texture sampler interface
- Used for computing texture level-of-detail
- Enables different LOD computation methods
- Supports different texture sampling strategies

**Alpha Testing**:
- Discards fragments based on alpha value
- Uses material system for alpha test
- Returns true if fragment should be discarded
- Used for cutout materials

## Technical Details

### Interpolation Mode

**Purpose**: Configure interpolation mode for vertex attributes.

**Default Value**:
- Default: `linear` interpolation
- Defined via `INTERPOLATION_MODE` macro
- Can be changed for different interpolation modes

**Usage**:
- Applies to interpolated attributes in VSOut
- Controls how attributes are interpolated across triangle
- Affects normal, tangent, texture coordinate, position, and previous position

**Possible Values**:
- `linear` - Linear interpolation (default)
- `noperspective` - No perspective correction
- `centroid` - Centroid interpolation for MSAA
- `sample` - Per-sample interpolation for MSAA

### Packed Vertex Attributes

**Purpose**: Efficient storage of vertex attributes.

**Packed Attributes**:
- [`packedNormalTangentCurveRadius`](Source/Falcor/Scene/Raster.slang:36) - Packed normal, tangent, and curve radius
- Stored in single float3
- Reduces vertex buffer size
- Unpacked on-demand

**Unpacking**:
- [`unpack()`](Source/Falcor/Scene/Raster.slang:45) method in VSIn
- Creates [`PackedStaticVertexData`](Source/Falcor/Scene/Raster.slang:47) structure
- Calls [`v.unpack()`](Source/Falcor/Scene/Raster.slang:51) to unpack
- Returns [`StaticVertexData`](Source/Falcor/Scene/Raster.slang:51) structure

**Benefits**:
- Reduced memory bandwidth
- Smaller vertex buffers
- Efficient GPU memory usage
- On-demand unpacking

### Normal Transformation

**Purpose**: Transform normals correctly to world space.

**Transformation**:
- Uses inverse transpose world matrix: [`gScene.getInverseTransposeWorldMatrix(instanceID)`](Source/Falcor/Scene/Raster.slang:88)
- Transforms normal: [`vOut.normalW = mul(gScene.getInverseTransposeWorldMatrix(instanceID), vIn.unpack().normal)`](Source/Falcor/Scene/Raster.slang:88)
- Correct for non-uniform scaling
- Preserves normal direction

**Inverse Transpose Matrix**:
- Corrects for non-uniform scaling
- Preserves orthogonality of normals
- Standard technique for normal transformation
- Ensures correct lighting

**Normalization**:
- Normal is not normalized in VSOut (interpolated)
- Normalized in [`prepareVertexData()`](Source/Falcor/Scene/Raster.slang:116)
- Interpolation can denormalize normals
- Re-normalization required for correct lighting

### Tangent Transformation

**Purpose**: Transform tangents correctly to world space.

**Transformation**:
- Uses world matrix (3x3): [`(float3x3)gScene.getWorldMatrix(instanceID)`](Source/Falcor/Scene/Raster.slang:90)
- Transforms tangent: [`vOut.tangentW = float4(mul((float3x3)gScene.getWorldMatrix(instanceID), tangent.xyz), tangent.w)`](Source/Falcor/Scene/Raster.slang:90)
- Preserves tangent.w (orientation)
- Uses 3x3 matrix for direction only

**Tangent.w**:
- Preserves orientation: [`tangent.w`](Source/Falcor/Scene/Raster.slang:90)
- Used for bitangent calculation: `b = cross(n, t)`
- Sign indicates tangent orientation
- Zero indicates invalid tangent

**Normalization**:
- Tangent is not normalized in VSOut (interpolated)
- Normalized in [`prepareVertexData()`](Source/Falcor/Scene/Raster.slang:118)
- Interpolation can denormalize tangents
- Re-normalization required for correct normal mapping

### Dynamic Geometry

**Purpose**: Support dynamic geometry with motion vectors.

**Dynamic Check**:
- Checks if geometry instance is dynamic: [`if (instance.isDynamic())`](Source/Falcor/Scene/Raster.slang:95)
- Uses previous vertex positions for dynamic geometry
- Uses current vertex positions for static geometry

**Previous Vertex Position**:
- Gets previous vertex index: [`uint prevVertexIndex = gScene.meshes[instance.geometryID].prevVbOffset + vIn.vertexID`](Source/Falcor/Scene/Raster.slang:97)
- Gets previous vertex position: [`prevPos = gScene.prevVertices[prevVertexIndex].position`](Source/Falcor/Scene/Raster.slang:98)
- Uses previous vertex buffer offset
- Uses vertex ID for indexing

**Previous World Matrix**:
- Gets previous world matrix: [`gScene.getPrevWorldMatrix(instanceID)`](Source/Falcor/Scene/Raster.slang:100)
- Transforms to previous world space: [`float3 prevPosW = mul(gScene.getPrevWorldMatrix(instanceID), float4(prevPos, 1.f)).xyz`](Source/Falcor/Scene/Raster.slang:100)
- Accounts for previous frame transform

**Previous Clip Space Position**:
- Uses previous view-projection matrix: [`gScene.camera.data.prevViewProjMatNoJitter`](Source/Falcor/Scene/Raster.slang:101)
- Transforms to previous clip space: [`vOut.prevPosH = mul(gScene.camera.data.prevViewProjMatNoJitter, float4(prevPosW, 1.f))`](Source/Falcor/Scene/Raster.slang:101)
- No jitter for motion vectors
- Used for motion vector calculation

**Motion Vectors**:
- Previous clip space position used for motion vectors
- Current clip space position: `posH`
- Previous clip space position: `prevPosH`
- Motion vector: `prevPosH.xy / prevPosH.w - posH.xy / posH.w`

### Previous Frame Position

**Purpose**: Provide previous frame position for motion vectors.

**Static Geometry**:
- Uses current vertex position: [`float3 prevPos = vIn.pos`](Source/Falcor/Scene/Raster.slang:93)
- Transforms with previous world matrix
- Assumes static geometry position is same

**Dynamic Geometry**:
- Uses previous vertex position: [`prevPos = gScene.prevVertices[prevVertexIndex].position`](Source/Falcor/Scene/Raster.slang:98)
- Transforms with previous world matrix
- Accounts for vertex animation

**Previous View-Projection Matrix**:
- No jitter: `prevViewProjMatNoJitter`
- Used for motion vector calculation
- Avoids jitter artifacts in motion vectors
- Provides stable previous frame position

### Per-Triangle Data

**Purpose**: Provide per-triangle data for fragment shader.

**Instance ID**:
- [`nointerpolation GeometryInstanceID instanceID : INSTANCE_ID`](Source/Falcor/Scene/Raster.slang:68)
- Not interpolated across triangle
- Constant across triangle
- Used for scene data access

**Material ID**:
- [`nointerpolation uint materialID : MATERIAL_ID`](Source/Falcor/Scene/Raster.slang:69)
- Not interpolated across triangle
- Constant across triangle
- Used for material data access

**Benefits**:
- Efficient data access
- No interpolation overhead
- Constant across triangle
- Simplifies fragment shader

### Vertex Data Preparation

**Purpose**: Prepare vertex data for shading.

**VertexData Structure**:
- [`posW`](Source/Falcor/Scene/Raster.slang:114) - World space position
- [`texC`](Source/Falcor/Scene/Raster.slang:115) - Texture coordinate
- [`normalW`](Source/Falcor/Scene/Raster.slang:116) - World space normal (normalized)
- [`faceNormalW`](Source/Falcor/Scene/Raster.slang:117) - Face normal (normalized)
- [`tangentW`](Source/Falcor/Scene/Raster.slang:118-119) - World space tangent (normalized, with sign)

**Normalization**:
- Normalizes interpolated normal
- Normalizes interpolated tangent
- Face normal is already normalized
- Preserves tangent.w sign

**Face Normal**:
- Provided as parameter
- Already normalized
- Used for flat shading
- Used for back-face culling

### Shading Data Preparation

**Purpose**: Prepare shading data for material evaluation.

**Integration**:
- Gets face normal from scene: [`gScene.getFaceNormalW(vsOut.instanceID, triangleIndex)`](Source/Falcor/Scene/Raster.slang:131)
- Prepares vertex data: [`prepareVertexData(vsOut, faceNormal)`](Source/Falcor/Scene/Raster.slang:132)
- Prepares shading data: [`gScene.materials.prepareShadingData(v, vsOut.materialID, viewDir)`](Source/Falcor/Scene/Raster.slang:133)

**ShadingData Structure**:
- Contains material properties
- Contains lighting parameters
- Contains view direction
- Used for material evaluation

**View Direction**:
- Points from shading point towards viewer
- Used for material evaluation
- Used for lighting calculations
- Required for BRDF evaluation

### Alpha Testing

**Purpose**: Discard fragments based on alpha value.

**Integration**:
- Gets face normal from scene: [`gScene.getFaceNormalW(vsOut.instanceID, triangleIndex)`](Source/Falcor/Scene/Raster.slang:144)
- Prepares vertex data: [`prepareVertexData(vsOut, faceNormal)`](Source/Falcor/Scene/Raster.slang:145)
- Performs alpha test: [`gScene.materials.alphaTest(v, vsOut.materialID, lod)`](Source/Falcor/Scene/Raster.slang:146)

**Texture LOD**:
- Template parameter: `L:ITextureSampler`
- Implements `ITextureSampler` interface
- Used for computing texture level-of-detail
- Enables different LOD computation methods

**Alpha Test Result**:
- Returns true if fragment should be discarded
- Returns false if fragment should be kept
- Used for cutout materials
- Discards fragments with low alpha

## Integration Points

### Scene Integration

**Matrix Access**:
- [`gScene.getWorldMatrix(instanceID)`](Source/Falcor/Scene/Raster.slang:79) - Get world matrix
- [`gScene.getInverseTransposeWorldMatrix(instanceID)`](Source/Falcor/Scene/Raster.slang:88) - Get inverse transpose world matrix
- [`gScene.getPrevWorldMatrix(instanceID)`](Source/Falcor/Scene/Raster.slang:100) - Get previous world matrix

**Geometry Instance Access**:
- [`gScene.getGeometryInstance(instanceID)`](Source/Falcor/Scene/Raster.slang:94) - Get geometry instance data
- [`gScene.getMaterialID(instanceID)`](Source/Falcor/Scene/Raster.slang:85) - Get material ID
- [`gScene.getFaceNormalW(vsOut.instanceID, triangleIndex)`](Source/Falcor/Scene/Raster.slang:131) - Get face normal

**Mesh Access**:
- [`gScene.meshes[instance.geometryID].prevVbOffset`](Source/Falcor/Scene/Raster.slang:97) - Get previous vertex buffer offset

**Vertex Access**:
- [`gScene.prevVertices[prevVertexIndex].position`](Source/Falcor/Scene/Raster.slang:98) - Get previous vertex position

### Camera Integration

**View-Projection Matrix**:
- [`gScene.camera.getViewProj()`](Source/Falcor/Scene/Raster.slang:82) - Get view-projection matrix
- [`gScene.camera.data.prevViewProjMatNoJitter`](Source/Falcor/Scene/Raster.slang:101) - Get previous view-projection matrix without jitter

### Material Integration

**Shading Data Preparation**:
- [`gScene.materials.prepareShadingData(v, vsOut.materialID, viewDir)`](Source/Falcor/Scene/Raster.slang:133) - Prepare shading data

**Alpha Testing**:
- [`gScene.materials.alphaTest(v, vsOut.materialID, lod)`](Source/Falcor/Scene/Raster.slang:146) - Perform alpha test

### VertexAttrib Integration

**Vertex Attribute Definitions**:
- `#include "VertexAttrib.slangh"` - Include vertex attribute definitions
- Provides vertex attribute structures
- Provides vertex attribute semantics
- Provides vertex attribute packing/unpacking

### Shading Integration

**Shading System**:
- `__exported import Scene.Shading` - Import shading system
- Provides shading data structures
- Provides shading functions
- Provides material evaluation

## Architecture Patterns

### Vertex Shader Pattern

- Input structure (VSIn) with packed attributes
- Output structure (VSOut) with interpolated attributes
- Vertex shader transforms positions, normals, tangents
- Vertex shader passes through texture coordinates
- Vertex shader computes previous frame position for motion vectors

### Interpolation Pattern

- Configurable interpolation mode
- Applies to interpolated attributes
- Controls how attributes are interpolated across triangle
- Supports different interpolation modes (linear, noperspective, centroid, sample)

### Per-Triangle Data Pattern

- `nointerpolation` modifier for per-triangle data
- Instance ID and material ID are per-triangle
- Not interpolated across triangle
- Constant across triangle

### Dynamic Geometry Pattern

- Check if geometry instance is dynamic
- Use previous vertex positions for dynamic geometry
- Use current vertex positions for static geometry
- Compute previous frame position for motion vectors

### Helper Function Pattern

- [`prepareVertexData()`](Source/Falcor/Scene/Raster.slang:111) - Prepare vertex data for shading
- [`prepareShadingData()`](Source/Falcor/Scene/Raster.slang:129) - Prepare shading data for material evaluation
- [`alphaTest()`](Source/Falcor/Scene/Raster.slang:142) - Perform alpha testing
- Reusable functions for common operations

## Code Patterns

### Vertex Shader Input Pattern

```slang
struct VSIn
{
    float3 pos : POSITION;
    float3 packedNormalTangentCurveRadius : PACKED_NORMAL_TANGENT_CURVE_RADIUS;
    float2 texC : TEXCOORD;
    uint instanceID : DRAW_ID;
    uint vertexID : SV_VertexID;

    StaticVertexData unpack()
    {
        PackedStaticVertexData v;
        v.position = pos;
        v.packedNormalTangentCurveRadius = packedNormalTangentCurveRadius;
        v.texCrd = texC;
        return v.unpack();
    }
};
```

### Vertex Shader Output Pattern

```slang
struct VSOut
{
    INTERPOLATION_MODE float3 normalW : NORMAL;
    INTERPOLATION_MODE float4 tangentW : TANGENT;
    INTERPOLATION_MODE float2 texC : TEXCRD;
    INTERPOLATION_MODE float3 posW : POSW;
    INTERPOLATION_MODE float4 prevPosH : PREVPOSH;
    nointerpolation GeometryInstanceID instanceID : INSTANCE_ID;
    nointerpolation uint materialID : MATERIAL_ID;
    float4 posH : SV_POSITION;
};
```

### Vertex Shader Pattern

```slang
VSOut defaultVS(VSIn vIn)
{
    VSOut vOut;
    const GeometryInstanceID instanceID = { vIn.instanceID };

    float4x4 worldMat = gScene.getWorldMatrix(instanceID);
    float3 posW = mul(worldMat, float4(vIn.pos, 1.f)).xyz;
    vOut.posW = posW;
    vOut.posH = mul(gScene.camera.getViewProj(), float4(posW, 1.f));

    vOut.instanceID = instanceID;
    vOut.materialID = gScene.getMaterialID(instanceID);

    vOut.texC = vIn.texC;
    vOut.normalW = mul(gScene.getInverseTransposeWorldMatrix(instanceID), vIn.unpack().normal);
    float4 tangent = vIn.unpack().tangent;
    vOut.tangentW = float4(mul((float3x3)gScene.getWorldMatrix(instanceID), tangent.xyz), tangent.w);

    float3 prevPos = vIn.pos;
    GeometryInstanceData instance = gScene.getGeometryInstance(instanceID);
    if (instance.isDynamic())
    {
        uint prevVertexIndex = gScene.meshes[instance.geometryID].prevVbOffset + vIn.vertexID;
        prevPos = gScene.prevVertices[prevVertexIndex].position;
    }
    float3 prevPosW = mul(gScene.getPrevWorldMatrix(instanceID), float4(prevPos, 1.f)).xyz;
    vOut.prevPosH = mul(gScene.camera.data.prevViewProjMatNoJitter, float4(prevPosW, 1.f));

    return vOut;
}
```

### Vertex Data Preparation Pattern

```slang
VertexData prepareVertexData(VSOut vsOut, float3 faceNormalW)
{
    VertexData v;
    v.posW = vsOut.posW;
    v.texC = vsOut.texC;
    v.normalW = normalize(vsOut.normalW);
    v.faceNormalW = faceNormalW;
    v.tangentW.xyz = normalize(vsOut.tangentW.xyz);
    v.tangentW.w = sign(vsOut.tangentW.w);
    return v;
}
```

### Shading Data Preparation Pattern

```slang
ShadingData prepareShadingData(VSOut vsOut, uint triangleIndex, float3 viewDir)
{
    float3 faceNormal = gScene.getFaceNormalW(vsOut.instanceID, triangleIndex);
    VertexData v = prepareVertexData(vsOut, faceNormal);
    return gScene.materials.prepareShadingData(v, vsOut.materialID, viewDir);
}
```

### Alpha Test Pattern

```slang
bool alphaTest<L:ITextureSampler>(VSOut vsOut, uint triangleIndex, L lod)
{
    float3 faceNormal = gScene.getFaceNormalW(vsOut.instanceID, triangleIndex);
    VertexData v = prepareVertexData(vsOut, faceNormal);
    return gScene.materials.alphaTest(v, vsOut.materialID, lod);
}
```

## Use Cases

### Rasterization

- **Triangle Rasterization**:
  - Render triangle geometry
  - Transform vertices to clip space
  - Interpolate vertex attributes
  - Generate fragments

- **Instanced Rendering**:
  - Render multiple instances of geometry
  - Use instance ID for per-instance data
  - Efficient memory usage
  - Batch rendering

### Shading

- **Material Shading**:
  - Prepare shading data for material evaluation
  - Use vertex data for interpolation
  - Use material system for material properties
  - Evaluate materials per fragment

- **Lighting**:
  - Use shading data for lighting calculations
  - Use view direction for BRDF evaluation
  - Use normal, tangent for normal mapping
  - Use texture coordinates for texture sampling

### Motion Vectors

- **Dynamic Geometry**:
  - Support dynamic geometry with motion vectors
  - Use previous vertex positions for dynamic geometry
  - Compute previous frame position
  - Generate motion vectors for temporal effects

- **Temporal Effects**:
  - Use motion vectors for temporal anti-aliasing
  - Use motion vectors for motion blur
  - Use motion vectors for temporal reprojection
  - Use motion vectors for temporal accumulation

### Alpha Testing

- **Cutout Materials**:
  - Discard fragments based on alpha value
  - Use material system for alpha test
  - Support cutout materials (foliage, fences, etc.)
  - Efficient rendering of cutout materials

## Performance Considerations

### GPU Computation

**Vertex Shader**:
- O(1) per vertex
- Matrix multiplication for position transformation
- Matrix multiplication for normal/tangent transformation
- Dynamic geometry check for previous position
- Minimal arithmetic operations

**Fragment Shader**:
- O(1) per fragment
- Vertex data preparation for shading
- Shading data preparation for material evaluation
- Alpha testing for cutout materials
- Minimal arithmetic operations

**Memory Access**:
- Scene data access for matrices
- Vertex data access for previous positions
- Material data access for shading
- Efficient memory access patterns

### Memory Usage

**Vertex Buffer**:
- Packed vertex attributes for efficient storage
- Reduced memory bandwidth
- Smaller vertex buffers
- Efficient GPU memory usage

**Previous Vertex Buffer**:
- Stores previous vertex positions for dynamic geometry
- Only for dynamic geometry
- Memory: vertexCount * 12 bytes per dynamic mesh
- Efficient memory usage

**Interpolation**:
- Interpolated attributes in VSOut
- Per-triangle data (instance ID, material ID) not interpolated
- Efficient interpolation
- Minimal memory overhead

### Optimization Opportunities

**Packed Attributes**:
- Use packed vertex attributes for efficient storage
- Reduce memory bandwidth
- Smaller vertex buffers
- On-demand unpacking

**Interpolation Mode**:
- Choose appropriate interpolation mode for use case
- Use `noperspective` for screen-space attributes
- Use `centroid` for MSAA
- Use `sample` for per-sample MSAA

**Dynamic Geometry**:
- Only use previous vertex positions for dynamic geometry
- Skip previous position computation for static geometry
- Reduce memory bandwidth for static geometry
- Optimize for mixed static/dynamic scenes

**Per-Triangle Data**:
- Use `nointerpolation` for per-triangle data
- Reduce interpolation overhead
- Constant across triangle
- Simplify fragment shader

## Limitations

### Feature Limitations

- **Triangle-Only**:
  - Only supports triangle geometry
  - No support for other geometry types
  - Limited to triangle lists

- **Interpolation Mode**:
  - Single interpolation mode for all attributes
  - Cannot mix interpolation modes
  - Limited flexibility

- **Packed Attributes**:
  - Fixed packing scheme
  - Cannot customize packing
  - Limited to specific attribute combinations

### Performance Limitations

- **Dynamic Geometry**:
  - Dynamic geometry check adds overhead
  - Previous vertex position access adds memory bandwidth
  - Previous world matrix access adds overhead
  - May impact performance for dynamic scenes

- **Interpolation**:
  - Interpolation can denormalize normals/tangents
  - Re-normalization required for correct lighting
  - Adds overhead in fragment shader
  - Necessary for correct results

### Integration Limitations

- **Scene Coupling**:
  - Tightly coupled to Falcor scene system
  - Requires scene to be initialized
  - Not suitable for standalone use
  - Requires specific scene implementation

- **Material Coupling**:
  - Tightly coupled to Falcor material system
  - Requires material system to be initialized
  - Not suitable for standalone use
  - Requires specific material implementation

### Debugging Limitations

- **Limited Error Reporting**:
  - No detailed error messages
  - No logging of failures
  - Difficult to debug rasterization issues
  - No validation of input parameters

## Best Practices

### Rasterization

- **Vertex Shader**:
  - Use packed vertex attributes for efficient storage
  - Transform positions, normals, tangents correctly
  - Use inverse transpose matrix for normal transformation
  - Preserve tangent.w for orientation

- **Interpolation**:
  - Choose appropriate interpolation mode for use case
  - Use `linear` for most attributes
  - Use `noperspective` for screen-space attributes
  - Use `centroid` for MSAA

### Shading

- **Vertex Data Preparation**:
  - Normalize interpolated normals/tangents
  - Preserve tangent.w sign
  - Use face normal for flat shading
  - Prepare vertex data for material evaluation

- **Shading Data Preparation**:
  - Use face normal from scene
  - Prepare vertex data correctly
  - Use material system for shading data
  - Pass view direction for material evaluation

### Motion Vectors

- **Dynamic Geometry**:
  - Check if geometry instance is dynamic
  - Use previous vertex positions for dynamic geometry
  - Use current vertex positions for static geometry
  - Compute previous frame position correctly

- **Previous Frame Position**:
  - Use previous view-projection matrix without jitter
  - Avoid jitter artifacts in motion vectors
  - Compute motion vectors correctly
  - Use for temporal effects

### Alpha Testing

- **Cutout Materials**:
  - Use material system for alpha test
  - Discard fragments with low alpha
  - Support cutout materials efficiently
  - Use texture LOD for correct alpha sampling

### Performance Optimization

- **Packed Attributes**:
  - Use packed vertex attributes for efficient storage
  - Reduce memory bandwidth
  - Smaller vertex buffers
  - On-demand unpacking

- **Interpolation Mode**:
  - Choose appropriate interpolation mode
  - Optimize for specific use case
  - Profile for optimal performance
  - Minimize interpolation overhead

- **Dynamic Geometry**:
  - Only use previous vertex positions for dynamic geometry
  - Skip previous position computation for static geometry
  - Reduce memory bandwidth for static geometry
  - Optimize for mixed static/dynamic scenes

### Debugging

- **Validate Input Parameters**:
  - Check vertex attributes are valid
  - Check instance ID is valid
  - Check material ID is valid
  - Check triangle index is valid

- **Visual Debugging**:
  - Visualize vertex positions
  - Visualize normals/tangents
  - Visualize texture coordinates
  - Verify rasterization

- **Performance Profiling**:
  - Measure vertex shader execution time
  - Profile memory access patterns
  - Identify bottlenecks
  - Optimize hot paths

## Progress Log

- **2026-01-08T00:03:00Z**: Raster analysis completed. Analyzed Raster.slang (147 lines) containing rasterization shaders. Documented VSIn structure with packed vertex attributes, instance ID, vertex ID, and unpack method. Documented VSOut structure with configurable interpolation mode, interpolated attributes (normal, tangent, texture coordinate, position, previous position), per-triangle data (instance ID, material ID), and clip space position. Documented defaultVS vertex shader with world space position transformation, clip space position transformation, normal/tangent transformation, previous frame position computation for dynamic geometry, and integration with scene and camera systems. Documented prepareVertexData helper function for preparing vertex data with normalization, prepareShadingData helper function for preparing shading data with material system integration, and alphaTest helper function for alpha testing with material system integration. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The Raster module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
