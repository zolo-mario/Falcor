# TriangleMesh - Triangle Mesh Data Structures

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **TriangleMesh** - Simple indexed triangle mesh

### External Dependencies

- **Core/Object** - Base object class
- **Utils/Math** - Mathematical utilities (Vector, Matrix)
- **Scene/Transform** - Transform utilities

## Module Overview

The TriangleMesh system provides simple indexed triangle mesh support for passing geometry to SceneBuilder. It includes vertex position, normal, and texture coordinate attributes, with support for various primitive shapes (quad, disk, cube, sphere) and file loading via ASSIMP for wide variety of asset formats.

## Component Specifications

### TriangleMesh

**File**: [`TriangleMesh.h`](Source/Falcor/Scene/TriangleMesh.h:1)

**Purpose**: Simple indexed triangle mesh.

**Key Features**:
- Vertex attributes: position, normal, texture coordinate
- Indexed triangle representation
- Primitive shape creation (quad, disk, cube, sphere)
- File loading via ASSIMP
- Transform application
- Triangle winding control

**Import Flags** (from [`TriangleMesh::ImportFlags`](Source/Falcor/Scene/TriangleMesh.h:49)):
- **None** - No special import flags
- **GenSmoothNormals** - Generate smooth normals
- **JoinIdenticalVertices** - Join identical vertices
- **Default** - Default flags (None)

**Vertex Structure** (from [`TriangleMesh::Vertex`](Source/Falcor/Scene/TriangleMesh.h:58)):
- **position** - Vertex position (float3)
- **normal** - Vertex normal (float3)
- **texCoord** - Vertex texture coordinate (float2)

**Core Methods**:

**Factory Methods**:
- [`create()`](Source/Falcor/Scene/TriangleMesh.h:71) - Creates triangle mesh
- [`create()`](Source/Falcor/Scene/TriangleMesh.h:79) - Creates triangle mesh from vertices and indices
  - Parameters: vertices, indices, frontFaceCW
  - frontFaceCW: Triangle winding
- [`createDummy()`](Source/Falcor/Scene/TriangleMesh.h:84) - Creates dummy mesh (single degenerate triangle)
- [`createQuad()`](Source/Falcor/Scene/TriangleMesh.h:90) - Creates quad mesh
  - Parameters: size (X and Z)
  - Centered at origin with normal pointing in positive Y direction
- [`createDisk()`](Source/Falcor/Scene/TriangleMesh.h:96) - Creates disk mesh
  - Parameters: radius, segments
  - Centered at origin with normal pointing in positive Y direction
- [`createCube()`](Source/Falcor/Scene/TriangleMesh.h:102) - Creates cube mesh
  - Parameters: size (each dimension)
  - Centered at origin
- [`createSphere()`](Source/Falcor/Scene/TriangleMesh.h:110) - Creates UV sphere mesh
  - Parameters: radius, segmentsU, segmentsV
  - Centered at origin with poles in positive/negative Y direction
- [`createFromFile()`](Source/Falcor/Scene/TriangleMesh.h:119) - Creates triangle mesh from file
  - Uses ASSIMP to support wide variety of asset formats
  - All geometry pre-transformed and merged into same triangle mesh
  - Parameters: path, flags
  - Returns: Triangle mesh or nullptr if load failed
- [`createFromFile()`](Source/Falcor/Scene/TriangleMesh.h:128) - Creates triangle mesh from file
  - Uses ASSIMP to support wide variety of asset formats
  - All geometry pre-transformed and merged into same triangle mesh
  - Parameters: path, smoothNormals
  - Returns: Triangle mesh or nullptr if load failed

**Name Management**:
- [`getName()`](Source/Falcor/Scene/TriangleMesh.h:133) - Get name of triangle mesh
- [`setName()`](Source/Falcor/Scene/TriangleMesh.h:138) - Set name of triangle mesh

**Vertex Management**:
- [`addVertex()`](Source/Falcor/Scene/TriangleMesh.h:146) - Adds vertex to vertex list
  - Parameters: position, normal, texCoord
  - Returns: Vertex index
- [`getVertices()`](Source/Falcor/Scene/TriangleMesh.h:157) - Get vertex list
- [`setVertices()`](Source/Falcor/Scene/TriangleMesh.h:161) - Set vertex list

**Index Management**:
- [`addTriangle()`](Source/Falcor/Scene/TriangleMesh.h:153) - Adds triangle to index list
  - Parameters: i0, i1, i2 (vertex indices)
- [`getIndices()`](Source/Falcor/Scene/TriangleMesh.h:165) - Get index list
- [`setIndices()`](Source/Falcor/Scene/TriangleMesh.h:169) - Set index list

**Triangle Winding**:
- [`getFrontFaceCW()`](Source/Falcor/Scene/TriangleMesh.h:173) - Get triangle winding
- [`setFrontFaceCW()`](Source/Falcor/Scene/TriangleMesh.h:177) - Set triangle winding

**Transform Application**:
- [`applyTransform()`](Source/Falcor/Scene/TriangleMesh.h:182) - Applies transform to triangle mesh
  - Parameters: Transform
- [`applyTransform()`](Source/Falcor/Scene/TriangleMesh.h:187) - Applies transform to triangle mesh
  - Parameters: float4x4 transform matrix

**Type Definitions**:
- **VertexList** - std::vector<Vertex>
- **IndexList** - std::vector<uint32_t>

**Protected Members**:
- [`mName`](Source/Falcor/Scene/TriangleMesh.h:193) - Triangle mesh name
- [`mVertices`](Source/Falcor/Scene/TriangleMesh.h:194) - Vertex list
- [`mIndices`](Source/Falcor/Scene/TriangleMesh.h:195) - Index list
- [`mFrontFaceCW`](Source/Falcor/Scene/TriangleMesh.h:196) - Front face clockwise flag

## Technical Details

### Vertex Attributes

**Position**:
- float3 vertex position
- World-space coordinates
- Used for rendering and ray tracing

**Normal**:
- float3 vertex normal
- Used for shading
- Can be generated or loaded from file

**Texture Coordinate**:
- float2 texture coordinate
- UV mapping for texturing
- Used for material sampling

### Primitive Shapes

**Quad**:
- Size in X and Z dimensions
- Centered at origin
- Normal pointing in positive Y direction
- Two triangles

**Disk**:
- Radius parameter
- Segment count (default: 32)
- Centered at origin
- Normal pointing in positive Y direction
- Approximated with triangles

**Cube**:
- Size in each dimension
- Centered at origin
- 12 triangles (2 per face)

**Sphere**:
- Radius parameter (default: 0.5)
- SegmentsU: Number of segments along parallels (default: 32)
- SegmentsV: Number of segments along meridians (default: 16)
- Centered at origin
- Poles in positive/negative Y direction
- UV sphere mapping

**Dummy Mesh**:
- Single degenerate triangle
- Used for placeholder geometry

### File Loading

**ASSIMP Integration**:
- Wide variety of asset formats
- All geometry pre-transformed
- Merged into same triangle mesh
- Path resolution: Absolute or relative to working directory

**Import Flags**:
- **GenSmoothNormals**: Generate smooth normals instead of facet normals
- **JoinIdenticalVertices**: Join identical vertices to reduce vertex count
- **Default**: No special import flags

### Triangle Winding

**Front Face CW**:
- Controls triangle winding order
- Affects backface culling
- False by default (counter-clockwise front face)

### Transform Application

**Transform Support**:
- Apply Transform object to mesh
- Apply float4x4 matrix to mesh
- Transforms vertex positions
- Transforms vertex normals

## Integration Points

### Scene Builder Integration

**Geometry Passing**:
- Simple indexed triangle mesh
- Used to pass geometry to SceneBuilder
- Vertex attributes: position, normal, texture coordinate
- Indexed triangle representation

### Asset Loading

**File Format Support**:
- Wide variety of asset formats via ASSIMP
- Pre-transformed geometry
- Merged into single triangle mesh

### Transform System

**Transform Application**:
- Transform object support
- Matrix transform support
- Consistent with scene transform system

## Architecture Patterns

### Factory Pattern
- Static [`create()`](Source/Falcor/Scene/TriangleMesh.h:71) methods for mesh creation
- Consistent initialization
- Type-safe factory methods

### Builder Pattern
- [`addVertex()`](Source/Falcor/Scene/TriangleMesh.h:146) for vertex construction
- [`addTriangle()`](Source/Falcor/Scene/TriangleMesh.h:153) for triangle construction
- Incremental mesh building

### Strategy Pattern
- Different primitive types with common interface
- Polymorphic mesh creation
- File loading and primitive creation

## Progress Log

- **2026-01-07T20:10:00Z**: Scene TriangleMesh subsystem analysis completed. Analyzed TriangleMesh class with vertex attributes, primitive shape creation, file loading via ASSIMP, transform application, and triangle winding control. Documented vertex structure, import flags, factory methods, and integration points. Ready to proceed to remaining Scene subsystems analysis.

## Next Steps

Proceed to analyze remaining Scene subsystems (SceneCache, Transform, VertexAttrib, SceneTypes, SceneIDs) to complete Scene module analysis.
