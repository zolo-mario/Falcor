# SceneIDs - Scene ID Definitions

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SceneIDs.h** - Scene IDs header (68 lines)

### External Dependencies

- **Utils/ObjectID.h** - Object ID system
- **cstdint** - Standard integer types

## Module Overview

SceneIDs is a comprehensive scene ID definition system that provides type-safe, strongly-typed IDs for all scene objects. It defines specific ID types for nodes, meshes, curves, SDF grids, materials, lights, cameras, volumes, and global geometry. The system uses the ObjectID template from Utils/ObjectID.h to create strongly-typed IDs with scene object kind and index type. SceneIDs are defined in the `scene1` namespace to distinguish from Falcor namespace IDs and ensure compatibility when IDs are directly in the Falcor namespace.

## Component Specifications

### SceneObjectKind Enumeration

**Purpose**: Enumeration of scene object kinds for type-safe ID creation.

**Values**:
- [`kNode`](Source/Falcor/Scene/SceneIDs.h:37) - NodeID, but also for MatrixID for animation
- [`kMesh`](Source/Falcor/Scene/SceneIDs.h:38) - MeshID, also curves that tessellate into triangle mesh
- [`kCurve`](Source/Falcor/Scene/SceneIDs.h:39) - CurveID
- [`kCurveOrMesh`](Source/Falcor/Scene/SceneIDs.h:40) - Used when ID in curves is aliased based on tessellation mode
- [`kSdfDesc`](Source/Falcor/Scene/SceneIDs.h:41) - The user-facing ID
- [`kSdfGrid`](Source/Falcor/Scene/SceneIDs.h:42) - The internal ID, can be deduplicated
- [`kMaterial`](Source/Falcor/Scene/SceneIDs.h:43) - Material ID
- [`kLight`](Source/Falcor/Scene/SceneIDs.h:44) - Light ID
- [`kCamera`](Source/Falcor/Scene/SceneIDs.h:45) - Camera ID
- [`kVolume`](Source/Falcor/Scene/SceneIDs.h:46) - Volume ID
- [`kGlobalGeometry`](Source/Falcor/Scene/SceneIDs.h:47) - The linearized global ID, current in order: mesh, curve, sdf, custom. Not to be confused with geometryID in curves, which is "either Mesh or Curve, depending on tessellation mode"

### Type Definitions

**Purpose**: Strongly-typed ID definitions for scene objects.

**NodeID**:
- [`using NodeID = ObjectID<SceneObjectKind, SceneObjectKind::kNode, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:51)
- Type-safe node ID
- Used for scene graph nodes
- Also used for MatrixID in animation

**MeshID**:
- [`using MeshID = ObjectID<SceneObjectKind, SceneObjectKind::kMesh, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:52)
- Type-safe mesh ID
- Used for triangle meshes
- Also used for curves that tessellate into triangle mesh

**CurveID**:
- [`using CurveID = ObjectID<SceneObjectKind, SceneObjectKind::kCurve, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:53)
- Type-safe curve ID
- Used for curve geometry

**CurveOrMeshID**:
- [`using CurveOrMeshID = ObjectID<SceneObjectKind, SceneObjectKind::kCurveOrMesh, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:54)
- Type-safe curve or mesh ID
- Used when ID in curves is aliased based on tessellation mode

**SdfDescID**:
- [`using SdfDescID = ObjectID<SceneObjectKind, SceneObjectKind::kSdfDesc, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:55)
- Type-safe SDF descriptor ID
- User-facing ID for SDF grids

**SdfGridID**:
- [`using SdfGridID = ObjectID<SceneObjectKind, SceneObjectKind::kSdfGrid, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:56)
- Type-safe SDF grid ID
- Internal ID for SDF grids
- Can be deduplicated

**MaterialID**:
- [`using MaterialID = ObjectID<SceneObjectKind, SceneObjectKind::kMaterial, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:57)
- Type-safe material ID
- Used for material references

**LightID**:
- [`using LightID = ObjectID<SceneObjectKind, SceneObjectKind::kLight, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:58)
- Type-safe light ID
- Used for light references

**CameraID**:
- [`using CameraID = ObjectID<SceneObjectKind, SceneObjectKind::kCamera, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:59)
- Type-safe camera ID
- Used for camera references

**VolumeID**:
- [`using VolumeID = ObjectID<SceneObjectKind, SceneObjectKind::kVolume, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:60)
- Type-safe volume ID
- Used for volume references

**GlobalGeometryID**:
- [`using GlobalGeometryID = ObjectID<SceneObjectKind, SceneObjectKind::kGlobalGeometry, uint32_t>`](Source/Falcor/Scene/SceneIDs.h:61)
- Type-safe global geometry ID
- Linearized global ID in order: mesh, curve, sdf, custom
- Not to be confused with geometryID in curves

## Technical Details

### ObjectID Template

**Purpose**: Type-safe object ID system using ObjectID template from Utils/ObjectID.h.

**Template Parameters**:
- `SceneObjectKind` - Scene object kind (node, mesh, curve, sdf desc, sdf grid, material, light, camera, volume, global geometry)
- `uint32_t` - Index type for ID

**Type Safety**:
- Compile-time type checking
- Prevents ID type confusion
- Strongly-typed IDs

**ID Encoding**:
- ObjectID encodes both kind and index
- Efficient storage (typically 32 bits)
- Fast comparison operations

### Scene Object Kinds

**Node Kind**:
- Used for scene graph nodes
- Also used for MatrixID in animation
- Distinguishes nodes from other object types

**Mesh Kind**:
- Used for triangle meshes
- Also used for curves that tessellate into triangle mesh
- Distinguishes meshes from other object types

**Curve Kind**:
- Used for curve geometry
- Distinguishes curves from other object types

**CurveOrMesh Kind**:
- Used when ID in curves is aliased based on tessellation mode
- Provides flexibility for curve tessellation
- Can represent either curve or mesh

**SDF Descriptor Kind**:
- User-facing ID for SDF grids
- Distinguishes SDF descriptors from other object types
- Used for user-facing SDF grid references

**SDF Grid Kind**:
- Internal ID for SDF grids
- Can be deduplicated
- Used for internal SDF grid management

**Material Kind**:
- Used for material references
- Distinguishes materials from other object types

**Light Kind**:
- Used for light references
- Distinguishes lights from other object types

**Camera Kind**:
- Used for camera references
- Distinguishes cameras from other object types

**Volume Kind**:
- Used for volume references
- Distinguishes volumes from other object types

**Global Geometry Kind**:
- Linearized global ID
- Ordered: mesh, curve, sdf, custom
- Used for global geometry indexing
- Prevents confusion with curve geometry IDs

### Namespace Organization

**Scene1 Namespace**:
- [`namespace Falcor::scene1`](Source/Falcor/Scene/SceneIDs.h:33) - Scene1 namespace for ID definitions
- Distinguishes from Falcor namespace
- Ensures compatibility when IDs are directly in Falcor namespace
- Used for backward compatibility

**Using Declarations**:
- [`using namespace scene1;`](Source/Falcor/Scene/SceneIDs.h:67) - Imports scene1 namespace into Falcor
- Makes scene1 IDs available in Falcor namespace
- Ensures everything works when IDs are directly in Falcor namespace

### ID Type Safety

**Compile-Time Checking**:
- ObjectID template provides type safety
- Prevents mixing ID types
- Catches type errors at compile time

**Runtime Safety**:
- Strongly-typed IDs prevent runtime errors
- Type-safe API for all scene objects
- No need for manual type checking

## Integration Points

### ObjectID System Integration

**Utils/ObjectID.h**:
- [`ObjectID`](Source/Falcor/Scene/SceneIDs.h:29) - ObjectID template
- Provides type-safe ID system
- Used for all scene ID definitions

**ObjectID Template**:
- SceneObjectKind parameter for kind
- uint32_t parameter for index
- Encodes both kind and index in single value
- Provides efficient storage and comparison

### Scene Integration

**Scene Object Kinds**:
- All scene objects use SceneObjectKind
- Provides type safety across scene system
- Enables compile-time type checking

**ID Usage**:
- All scene APIs use strongly-typed IDs
- Prevents ID type confusion
- Provides type-safe object references

### Animation Integration

**MatrixID**:
- NodeID also used for MatrixID in animation
- Enables type-safe matrix references
- Supports hierarchical transforms

## Architecture Patterns

### Type-Safe ID Pattern

- ObjectID template with kind and index
- Compile-time type checking
- Strongly-typed IDs for all scene objects
- Prevents ID type confusion

### Namespace Pattern

- scene1 namespace for backward compatibility
- Using declarations to import into Falcor namespace
- Ensures compatibility when IDs are directly in Falcor namespace

### ID Encoding Pattern

- Single 32-bit value for kind and index
- Efficient storage
- Fast comparison operations
- Type-safe API

## Code Patterns

### ID Definition Pattern

```cpp
using NodeID = ObjectID<SceneObjectKind, SceneObjectKind::kNode, uint32_t>;
using MeshID = ObjectID<SceneObjectKind, SceneObjectKind::kMesh, uint32_t>;
using CurveID = ObjectID<SceneObjectKind, SceneObjectKind::kCurve, uint32_t>;
using CurveOrMeshID = ObjectID<SceneObjectKind, SceneObjectKind::kCurveOrMesh, uint32_t>;
using SdfDescID = ObjectID<SceneObjectKind, SceneObjectKind::kSdfDesc, uint32_t>;
using SdfGridID = ObjectID<SceneObjectKind, SceneObjectKind::kSdfGrid, uint32_t>;
using MaterialID = ObjectID<SceneObjectKind, SceneObjectKind::kMaterial, uint32_t>;
using LightID = ObjectID<SceneObjectKind, SceneObjectKind::kLight, uint32_t>;
using CameraID = ObjectID<SceneObjectKind, SceneObjectKind::kCamera, uint32_t>;
using VolumeID = ObjectID<SceneObjectKind, SceneObjectKind::kVolume, uint32_t>;
using GlobalGeometryID = ObjectID<SceneObjectKind, SceneObjectKind::kGlobalGeometry, uint32_t>;
```

### Namespace Pattern

```cpp
namespace Falcor::scene1
{
    enum class SceneObjectKind
    {
        kNode,
        kMesh,
        kCurve,
        kCurveOrMesh,
        kSdfDesc,
        kSdfGrid,
        kMaterial,
        kLight,
        kCamera,
        kVolume,
        kGlobalGeometry,
    };
}

namespace Falcor
{
    using namespace scene1;
}
```

## Use Cases

### Scene Object Identification

- **Node Identification**:
  - Type-safe node IDs
  - Used for scene graph nodes
  - Also used for animation matrices

- **Mesh Identification**:
  - Type-safe mesh IDs
  - Used for triangle meshes
  - Also used for tessellated curves

- **Curve Identification**:
  - Type-safe curve IDs
  - Used for curve geometry
  - Distinguished from mesh IDs

- **SDF Grid Identification**:
  - Type-safe SDF grid IDs
  - Internal ID for SDF grids
  - Can be deduplicated

- **Material Identification**:
  - Type-safe material IDs
  - Used for material references
  - Distinguished from other object types

- **Light Identification**:
  - Type-safe light IDs
  - Used for light references
  - Distinguished from other object types

- **Camera Identification**:
  - Type-safe camera IDs
  - Used for camera references
  - Distinguished from other object types

- **Volume Identification**:
  - Type-safe volume IDs
  - Used for volume references
  - Distinguished from other object types

### Global Geometry Indexing

- **Linearized Global IDs**:
  - Ordered: mesh, curve, sdf, custom
  - Used for global geometry indexing
  - Prevents confusion with curve geometry IDs

### Type Safety

- **Compile-Time Checking**:
  - ObjectID template provides type safety
  - Prevents mixing ID types
  - Catches type errors at compile time

- **Runtime Safety**:
  - Strongly-typed IDs prevent runtime errors
  - Type-safe API for all scene objects

## Performance Considerations

### ID Storage

**Memory Efficiency**:
- 32-bit ID storage
- Efficient bit packing
- Single value for kind and index
- Minimal memory overhead

**Comparison Performance**:
- Fast integer comparison
- No string comparison needed
- Compile-time type checking

### Type Safety Performance

**Compile-Time Overhead**:
- Zero runtime overhead
- Compile-time type checking
- No runtime type checking needed

**Runtime Overhead**:
- Minimal overhead for strongly-typed IDs
- No dynamic type checking
- Efficient API calls

## Limitations

### Feature Limitations

- **ID Space**:
- 32-bit ID space limits to 2^32 objects per kind
- May need expansion for very large scenes
- No support for 64-bit IDs

- **Object Kind Limitations**:
- Fixed set of object kinds
- Cannot add new object kinds without modifying ObjectID
- Limited to predefined scene objects

- **Curve Tessellation**:
- CurveOrMeshID provides flexibility
- May be confusing for users
- Requires careful documentation

### Performance Limitations

- **ID Comparison**:
- Integer comparison only
- No support for custom comparison
- Limited to 32-bit comparison

### Integration Limitations

- **ObjectID Coupling**:
- Tightly coupled to ObjectID template
- Requires ObjectID system
- Not suitable for standalone ID systems

## Best Practices

### ID Usage

- **Type Safety**:
  - Always use strongly-typed IDs
  - Avoid manual type casting
  - Use appropriate ID type for each object

- **ID Comparison**:
  - Use ID equality operators
  - Avoid manual kind/index extraction
  - Let ObjectID handle type checking

- **ID Creation**:
  - Use appropriate ID type
  - Use invalid ID constants
  - Document ID validity requirements

### Namespace Usage

- **scene1 Namespace**:
  - Use scene1 namespace for backward compatibility
  - Import using declarations
  - Document namespace usage
  - Ensure compatibility

### Performance Optimization

- **ID Storage**:
  - Use 32-bit IDs when possible
  - Avoid 64-bit IDs unless needed
  - Minimize ID storage overhead

- **Type Safety**:
  - Leverage compile-time type checking
  - Use strongly-typed IDs throughout
  - Avoid runtime type checking

## Progress Log

- **2026-01-08T00:41:00Z**: SceneIDs analysis completed. Analyzed SceneIDs.h (68 lines) containing comprehensive scene ID definition system. Documented SceneObjectKind enumeration with 10 values (kNode, kMesh, kCurve, kCurveOrMesh, kSdfDesc, kSdfGrid, kMaterial, kLight, kCamera, kVolume, kGlobalGeometry), type definitions (NodeID, MeshID, CurveID, CurveOrMeshID, SdfDescID, SdfGridID, MaterialID, LightID, CameraID, VolumeID, GlobalGeometryID) using ObjectID template with SceneObjectKind and uint32_t parameters, scene1 namespace for backward compatibility, and using declarations to import scene1 namespace into Falcor namespace. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The SceneIDs module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
