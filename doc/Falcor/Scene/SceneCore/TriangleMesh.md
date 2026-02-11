# TriangleMesh - Triangle Mesh

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **TriangleMesh.h** - Triangle mesh header (200 lines)
- [x] **TriangleMesh.cpp** - Triangle mesh implementation (364 lines)

### External Dependencies

- **Transform.h** - Transform utilities
- **Core/Macros.h** - Core macros
- **Core/Object.h** - Base object class
- **Utils/Math/Vector.h** - Vector mathematics
- **Utils/Math/Matrix.h** - Matrix mathematics
- **GlobalState.h** - Global state management
- **Core/Error.h** - Error handling
- **Core/Platform/OS.h** - Operating system utilities
- **Utils/Logger.h** - Logging utilities
- **Utils/Scripting/ScriptBindings.h** - Script bindings
- **assimp/Importer.hpp** - ASSIMP asset importer
- **assimp/scene.h** - ASSIMP scene structures
- **assimp/postprocess.h** - ASSIMP post-processing flags

## Module Overview

TriangleMesh provides a simple indexed triangle mesh utility for passing geometry to SceneBuilder. Vertices have position, normal, and texture coordinate attributes. The class supports creating various primitive shapes (quad, disk, cube, sphere), loading meshes from files using ASSIMP, applying transforms, and building meshes programmatically. It provides Python bindings for scripting support and is designed as a utility class for scene construction.

## Component Specifications

### TriangleMesh Class

**Purpose**: Simple indexed triangle mesh for passing geometry to SceneBuilder.

**Base Class**: [`Object`](Source/Falcor/Scene/TriangleMesh.h:45) - Base object class

**API**: `FALCOR_API`

**Object Macro**: `FALCOR_OBJECT(TriangleMesh)`

### ImportFlags Enumeration

**Purpose**: Flags controlling ASSIMP mesh import options.

**Values**:
- [`None`](Source/Falcor/Scene/TriangleMesh.h:51) - `0x0` - No import flags
- [`GenSmoothNormals`](Source/Falcor/Scene/TriangleMesh.h:52) - `0x1` - Generate smooth normals instead of facet normals
- [`JoinIdenticalVertices`](Source/Falcor/Scene/TriangleMesh.h:53) - `0x2` - Join identical vertices
- [`Default`](Source/Falcor/Scene/TriangleMesh.h:55) - None - Default import flags

### Vertex Structure

**Purpose**: Vertex data structure for triangle mesh.

**Fields**:
- [`position`](Source/Falcor/Scene/TriangleMesh.h:60) - `float3` - Vertex position
- [`normal`](Source/Falcor/Scene/TriangleMesh.h:61) - `float3` - Vertex normal
- [`texCoord`](Source/Falcor/Scene/TriangleMesh.h:62) - `float2` - Vertex texture coordinate

### Type Definitions

- [`VertexList`](Source/Falcor/Scene/TriangleMesh.h:65) - `std::vector<Vertex>` - Vertex list type
- [`IndexList`](Source/Falcor/Scene/TriangleMesh.h:66) - `std::vector<uint32_t>` - Index list type

### Public Methods

#### Creation Methods

- [`create()`](Source/Falcor/Scene/TriangleMesh.cpp:41) - Creates an empty triangle mesh
  - Returns: ref<TriangleMesh> - Empty triangle mesh

- [`create(const VertexList& vertices, const IndexList& indices, bool frontFaceCW = false)`](Source/Falcor/Scene/TriangleMesh.cpp:46) - Creates a triangle mesh from vertices and indices
  - Parameters:
    - vertices - Vertex list
    - indices - Index list
    - frontFaceCW - Triangle winding (default: false)
  - Returns: ref<TriangleMesh> - Triangle mesh

- [`createDummy()`](Source/Falcor/Scene/TriangleMesh.cpp:51) - Creates a dummy mesh (single degenerate triangle)
  - Returns: ref<TriangleMesh> - Dummy triangle mesh
  - Implementation: Single vertex at (0, 0, 0) with normal (0, 1, 0) and texCoord (0, 0), indices (0, 0, 0)

- [`createQuad(float2 size = float2(1.f))`](Source/Falcor/Scene/TriangleMesh.cpp:58) - Creates a quad mesh, centered at origin with normal pointing in positive Y direction
  - Parameters: size - Size of quad in X and Z (default: (1, 1))
  - Returns: ref<TriangleMesh> - Quad triangle mesh
  - Implementation: 4 vertices, 6 indices, frontFaceCW determined by sign of size.x * size.y

- [`createDisk(float radius, uint32_t segments = 32)`](Source/Falcor/Scene/TriangleMesh.cpp:79) - Creates a disk mesh, centered at origin with normal pointing in positive Y direction
  - Parameters:
    - radius - Radius of disk
    - segments - Number of segments (default: 32)
  - Returns: ref<TriangleMesh> - Disk triangle mesh
  - Implementation: Center vertex + ring vertices, triangular fan topology

- [`createCube(float3 size = float3(1.f))`](Source/Falcor/Scene/TriangleMesh.cpp:102) - Creates a cube mesh, centered at origin
  - Parameters: size - Size of cube in each dimension (default: (1, 1, 1))
  - Returns: ref<TriangleMesh> - Cube triangle mesh
  - Implementation: 6 faces, 24 vertices, 36 indices, frontFaceCW determined by sign of size.x * size.y * size.z

- [`createSphere(float radius = 0.5f, uint32_t segmentsU = 32, uint32_t segmentsV = 16)`](Source/Falcor/Scene/TriangleMesh.cpp:151) - Creates a UV sphere mesh, centered at origin with poles in positive/negative Y direction
  - Parameters:
    - radius - Radius of sphere (default: 0.5)
    - segmentsU - Number of segments along parallels (default: 32)
    - segmentsV - Number of segments along meridians (default: 16)
  - Returns: ref<TriangleMesh> - Sphere triangle mesh
  - Implementation: (segmentsU+1)*(segmentsV+1) vertices, quad topology converted to triangles

- [`createFromFile(const std::filesystem::path& path, ImportFlags flags)`](Source/Falcor/Scene/TriangleMesh.cpp:196) - Creates a triangle mesh from a file using ASSIMP
  - Parameters:
    - path - File path to load mesh from (absolute or relative to working directory)
    - flags - Flags controlling ASSIMP mesh import options
  - Returns: ref<TriangleMesh> - Triangle mesh or nullptr if mesh failed to load
  - Implementation: Uses ASSIMP to load mesh, supports gzipped files, merges all geometry into single mesh

- [`createFromFile(const std::filesystem::path& path, bool smoothNormals = false)`](Source/Falcor/Scene/TriangleMesh.cpp:276) - Creates a triangle mesh from a file using ASSIMP
  - Parameters:
    - path - File path to load mesh from (absolute or relative to working directory)
    - smoothNormals - If no normals are defined in the model, generate smooth instead of facet normals (default: false)
  - Returns: ref<TriangleMesh> - Triangle mesh or nullptr if mesh failed to load
  - Implementation: Converts smoothNormals to ImportFlags and calls createFromFile(path, flags)

#### Name Methods

- [`getName()`](Source/Falcor/Scene/TriangleMesh.h:133) - Get name of triangle mesh
  - Returns: const std::string - Name

- [`setName(const std::string& name)`](Source/Falcor/Scene/TriangleMesh.h:138) - Set name of triangle mesh
  - Parameters: name - Name to set

#### Vertex and Triangle Methods

- [`addVertex(float3 position, float3 normal, float2 texCoord)`](Source/Falcor/Scene/TriangleMesh.cpp:282) - Adds a vertex to vertex list
  - Parameters:
    - position - Vertex position
    - normal - Vertex normal
    - texCoord - Vertex texture coordinate
  - Returns: uint32_t - Vertex index
  - Implementation: Emplaces vertex to mVertices, asserts size < uint32_t max

- [`addTriangle(uint32_t i0, uint32_t i1, uint32_t i2)`](Source/Falcor/Scene/TriangleMesh.cpp:289) - Adds a triangle to index list
  - Parameters:
    - i0 - First index
    - i1 - Second index
    - i2 - Third index
  - Implementation: Emplaces indices to mIndices

#### Vertex and Index List Methods

- [`getVertices()`](Source/Falcor/Scene/TriangleMesh.h:157) - Get vertex list
  - Returns: const VertexList& - Vertex list

- [`setVertices(const VertexList& vertices)`](Source/Falcor/Scene/TriangleMesh.h:161) - Set vertex list
  - Parameters: vertices - Vertex list to set

- [`getIndices()`](Source/Falcor/Scene/TriangleMesh.h:165) - Get index list
  - Returns: const IndexList& - Index list

- [`setIndices(const IndexList& indices)`](Source/Falcor/Scene/TriangleMesh.h:169) - Set index list
  - Parameters: indices - Index list to set

#### Front Face Winding Methods

- [`getFrontFaceCW()`](Source/Falcor/Scene/TriangleMesh.h:173) - Get triangle winding
  - Returns: bool - True if front face is clockwise

- [`setFrontFaceCW(bool frontFaceCW)`](Source/Falcor/Scene/TriangleMesh.h:177) - Set triangle winding
  - Parameters: frontFaceCW - Triangle winding

#### Transform Methods

- [`applyTransform(const Transform& transform)`](Source/Falcor/Scene/TriangleMesh.cpp:296) - Applies a transform to the triangle mesh
  - Parameters: transform - Transform to apply
  - Implementation: Calls applyTransform(transform.getMatrix())

- [`applyTransform(const float4x4& transform)`](Source/Falcor/Scene/TriangleMesh.cpp:301) - Applies a transform to the triangle mesh
  - Parameters: transform - Transform matrix to apply
  - Implementation: Transforms vertex positions and normals, checks if triangle winding has flipped and adjusts winding order accordingly

### Private Members

- [`mName`](Source/Falcor/Scene/TriangleMesh.h:193) - `std::string` - Name of triangle mesh
- [`mVertices`](Source/Falcor/Scene/TriangleMesh.h:194) - `std::vector<Vertex>` - Vertex list
- [`mIndices`](Source/Falcor/Scene/TriangleMesh.h:195) - `std::vector<uint32_t>` - Index list
- [`mFrontFaceCW`](Source/Falcor/Scene/TriangleMesh.h:196) - `bool` - Triangle winding, initialized to false

### Private Constructors

- [`TriangleMesh()`](Source/Falcor/Scene/TriangleMesh.cpp:316) - Default constructor
  - Initializes all members to default values

- [`TriangleMesh(const VertexList& vertices, const IndexList& indices, bool frontFaceCW)`](Source/Falcor/Scene/TriangleMesh.cpp:319) - Constructor with parameters
  - Parameters:
    - vertices - Vertex list
    - indices - Index list
    - frontFaceCW - Triangle winding
  - Initializes members from parameters

## Technical Details

### Primitive Creation

**Quad Creation**:
- 4 vertices at corners of quad
- 6 indices (2 triangles)
- Normal pointing in positive Y direction
- Texture coordinates (0,0), (1,0), (0,1), (1,1)
- FrontFaceCW determined by sign of size.x * size.y

**Disk Creation**:
- Center vertex at origin
- Ring vertices around center
- Triangular fan topology
- Normal pointing in positive Y direction
- Texture coordinates mapped from circle to square

**Cube Creation**:
- 6 faces (bottom, top, front, back, left, right)
- 24 vertices (4 per face)
- 36 indices (12 triangles)
- Normals pointing outward
- Texture coordinates (0,0), (1,0), (1,1), (0,1) per face
- FrontFaceCW determined by sign of size.x * size.y * size.z

**Sphere Creation**:
- UV sphere topology
- (segmentsU+1)*(segmentsV+1) vertices
- Poles at positive/negative Y direction
- Normal = position / radius
- Texture coordinates = (u/segmentsU, v/segmentsV)
- Quad topology converted to triangles

### File Loading

**ASSIMP Integration**:
- Uses ASSIMP to support wide variety of asset formats
- All geometry pre-transformed and merged into single triangle mesh
- Supports gzipped files
- Post-processing flags:
  - aiProcess_FlipUVs - Flip UV coordinates
  - aiProcess_Triangulate - Triangulate polygons
  - aiProcess_PreTransformVertices - Pre-transform vertices
  - aiProcess_GenSmoothNormals - Generate smooth normals (optional)
  - aiProcess_GenNormals - Generate facet normals (optional)
  - aiProcess_JoinIdenticalVertices - Join identical vertices (optional)

**Error Handling**:
- Returns nullptr if file not found
- Returns nullptr if mesh failed to load
- Logs warnings for errors

### Transform Application

**Position Transformation**:
- Uses transformPoint() for vertex positions
- Supports arbitrary transforms

**Normal Transformation**:
- Uses inverse transpose of transform for normals
- Normalizes transformed normals
- Preserves normal direction

**Winding Adjustment**:
- Checks if triangle winding has flipped using determinant of transform
- Flips frontFaceCW flag if winding changed
- Ensures correct front/back face detection

### Python Bindings

**Enum Bindings**:
- TriangleMeshImportFlags enum
- Values: Default, GenSmoothNormals, JoinIdenticalVertices
- Binary operators for flags

**Class Bindings**:
- TriangleMesh class
- Vertex class with position, normal, texCoord fields
- Properties: name, frontFaceCW, vertices, indices
- Methods: addVertex, addTriangle
- Static methods: createQuad, createDisk, createCube, createSphere, createFromFile

**Asset Resolution**:
- Uses getActiveAssetResolver().resolvePath() for file paths
- Supports relative and absolute paths

## Integration Points

### SceneBuilder Integration

**Geometry Passing**:
- Used as utility to pass geometry to SceneBuilder
- Supports vertex and index data
- Supports transform application

**Scene Construction**:
- Used for scene construction
- Supports primitive creation
- Supports file loading

### Transform System Integration

**Transform Application**:
- Supports Transform class
- Supports float4x4 matrix
- Handles winding adjustment

### Scripting Integration

**Python Bindings**:
- Full Python support for TriangleMesh class
- Full Python support for Vertex class
- Full Python support for ImportFlags enum
- Supports scripting scene construction

## Architecture Patterns

### Factory Pattern

- Static create methods for different mesh types
- Consistent interface for mesh creation
- Supports both programmatic and file-based creation

### Builder Pattern

- addVertex() and addTriangle() for programmatic mesh construction
- Flexible mesh building
- Supports custom mesh creation

### Transform Pattern

- applyTransform() for applying transforms
- Supports both Transform class and float4x4 matrix
- Handles winding adjustment

### Python Binding Pattern

- Python bindings for scripting support
- Property access and modification
- Method invocation
- Static method support

## Code Patterns

### Quad Creation Pattern

```cpp
ref<TriangleMesh> TriangleMesh::createQuad(float2 size)
{
    float2 hsize = 0.5f * size;
    float3 normal{0.f, 1.f, 0.f};
    bool frontFaceCW = size.x * size.y < 0.f;

    VertexList vertices{
        {{ -hsize.x, 0.f, -hsize.y }, normal, { 0.f, 0.f }},
        {{  hsize.x, 0.f, -hsize.y }, normal, { 1.f, 0.f }},
        {{ -hsize.x, 0.f,  hsize.y }, normal, { 0.f, 1.f }},
        {{  hsize.x, 0.f,  hsize.y }, normal, { 1.f, 1.f }},
    };

    IndexList indices{
        2, 1, 0,
        1, 2, 3,
    };

    return create(vertices, indices, frontFaceCW);
}
```

### Sphere Creation Pattern

```cpp
ref<TriangleMesh> TriangleMesh::createSphere(float radius, uint32_t segmentsU, uint32_t segmentsV)
{
    VertexList vertices;
    IndexList indices;

    // Create vertices.
    for (uint32_t v = 0; v <= segmentsV; ++v)
    {
        for (uint32_t u = 0; u <= segmentsU; ++u)
        {
            float2 uv = float2(u / float(segmentsU), v / float(segmentsV));
            float theta = uv.x * 2.f * (float)M_PI;
            float phi = uv.y * (float)M_PI;
            float3 dir = float3(
                std::cos(theta) * std::sin(phi),
                std::cos(phi),
                std::sin(theta) * std::sin(phi)
            );
            vertices.emplace_back(Vertex{ dir * radius, dir, uv });
        }
    }

    // Create indices.
    for (uint32_t v = 0; v < segmentsV; ++v)
    {
        for (uint32_t u = 0; u < segmentsU; ++u)
        {
            uint32_t i0 = v * (segmentsU + 1) + u;
            uint32_t i1 = v * (segmentsU + 1) + (u + 1) % (segmentsU + 1);
            uint32_t i2 = (v + 1) * (segmentsU + 1) + u;
            uint32_t i3 = (v + 1) * (segmentsU + 1) + (u + 1) % (segmentsU + 1);

            indices.emplace_back(i0);
            indices.emplace_back(i1);
            indices.emplace_back(i2);

            indices.emplace_back(i2);
            indices.emplace_back(i1);
            indices.emplace_back(i3);
        }
    }

    return create(vertices, indices);
}
```

### Transform Application Pattern

```cpp
void TriangleMesh::applyTransform(const float4x4& transform)
{
    auto invTranspose = float3x3(transpose(inverse(transform)));

    for (auto& vertex : mVertices)
    {
        vertex.position = transformPoint(transform, vertex.position);
        vertex.normal = normalize(transformVector(invTranspose, vertex.normal));
    }

    // Check if triangle winding has flipped and adjust winding order accordingly.
    bool flippedWinding = determinant(float3x3(transform)) < 0.f;
    if (flippedWinding) mFrontFaceCW = !mFrontFaceCW;
}
```

## Use Cases

### Scene Construction

- **Primitive Creation**:
  - Create quad, disk, cube, sphere primitives
  - Use for scene construction
  - Use for testing and debugging

- **File Loading**:
  - Load meshes from files using ASSIMP
  - Support for wide variety of asset formats
  - Use for scene construction

### Programmatic Mesh Construction

- **Custom Meshes**:
  - Build meshes programmatically
  - Use addVertex() and addTriangle()
  - Support for custom geometry

- **Mesh Modification**:
  - Modify existing meshes
  - Add vertices and triangles
  - Support for dynamic mesh construction

### Transform Application

- **Mesh Transforms**:
  - Apply transforms to meshes
  - Support for both Transform class and float4x4 matrix
  - Handle winding adjustment

- **Mesh Placement**:
  - Place meshes in scene
  - Apply transforms for positioning
  - Support for scene graph

### Scripting

- **Python Scripting**:
  - Create meshes from Python
  - Modify meshes from Python
  - Access mesh properties from Python

- **Scene Creation**:
  - Create scene graphs from Python
  - Create primitives from Python
  - Load meshes from files from Python

## Performance Considerations

### Memory Performance

**Vertex Storage**:
- Efficient vertex storage using std::vector
- Compact vertex structure (position, normal, texCoord)
- Minimal memory overhead

**Index Storage**:
- Efficient index storage using std::vector<uint32_t>
- Compact index representation
- Minimal memory overhead

### Computation Performance

**Primitive Creation**:
- Efficient primitive creation algorithms
- Optimized for common primitives
- Minimal overhead for mesh construction

**Transform Application**:
- Efficient transform application
- Optimized for large meshes
- Minimal overhead for transform operations

**File Loading**:
- Efficient file loading using ASSIMP
- Optimized for large meshes
- Minimal overhead for file parsing

## Limitations

### Feature Limitations

- **Vertex Attributes**:
- Limited to position, normal, texCoord
- No support for tangents
- No support for vertex colors

- **Primitive Types**:
- Limited to quad, disk, cube, sphere
- No support for other primitives
- No support for custom primitives

- **File Format Support**:
- Limited to ASSIMP-supported formats
- May not support all formats
- May have format-specific limitations

### Performance Limitations

- **Transform Overhead**:
- Transform application may be expensive for large meshes
- Normal transformation may be expensive
- Winding adjustment may add overhead

- **File Loading Overhead**:
- File loading may be expensive for large meshes
- ASSIMP overhead may be significant
- May not be optimal for real-time loading

### Integration Limitations

- **SceneBuilder Coupling**:
- Designed as utility for SceneBuilder
- Not suitable for standalone use
- Requires specific data structures

## Best Practices

### Mesh Creation

- **Use Appropriate Primitives**:
- Use quad for planar geometry
- Use disk for circular geometry
- Use cube for box geometry
- Use sphere for spherical geometry

- **Use Appropriate Segments**:
- Use appropriate segments for disk and sphere
- Balance quality and performance
- Consider use case requirements

### File Loading

- **Use Appropriate Import Flags**:
- Use GenSmoothNormals for smooth shading
- Use JoinIdenticalVertices for optimization
- Consider use case requirements

- **Handle Errors**:
- Check for nullptr return value
- Log warnings for errors
- Handle missing files gracefully

### Transform Application

- **Use Appropriate Transform**:
- Use Transform class for complex transforms
- Use float4x4 matrix for simple transforms
- Consider winding adjustment

- **Batch Transforms**:
- Apply transforms to multiple meshes at once
- Minimize transform overhead
- Optimize for performance

### Scripting

- **Use Python Bindings**:
- Use Python bindings for scripting
- Use static methods for creation
- Use properties for attribute access

## Progress Log

- **2026-01-08T00:56:00Z**: TriangleMesh analysis completed. Analyzed TriangleMesh.h (200 lines) and TriangleMesh.cpp (364 lines) containing simple indexed triangle mesh utility. Documented TriangleMesh class (Object base class, FALCOR_API, FALCOR_OBJECT macro), ImportFlags enumeration (None, GenSmoothNormals, JoinIdenticalVertices, Default), Vertex structure (position, normal, texCoord), VertexList and IndexList type definitions, public methods (create, create with vertices/indices, createDummy, createQuad, createDisk, createCube, createSphere, createFromFile with flags, createFromFile with smoothNormals, getName, setName, addVertex, addTriangle, getVertices, setVertices, getIndices, setIndices, getFrontFaceCW, setFrontFaceCW, applyTransform with Transform, applyTransform with float4x4), private members (mName, mVertices, mIndices, mFrontFaceCW), private constructors (default, with vertices/indices/frontFaceCW), primitive creation (quad with 4 vertices and 6 indices, disk with center vertex and ring vertices, cube with 6 faces 24 vertices and 36 indices, UV sphere with (segmentsU+1)*(segmentsV+1) vertices), file loading with ASSIMP integration (supports gzipped files, post-processing flags, error handling), transform application (position transformation with transformPoint, normal transformation with inverse transpose, winding adjustment with determinant check), Python bindings (enum bindings, class bindings, Vertex class bindings, property bindings, method bindings, static method bindings, asset resolution). Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The TriangleMesh module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
