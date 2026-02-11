# SceneDefines - Scene Defines

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SceneDefines.slangh** - Scene defines shader header (43 lines)

### External Dependencies

- **Utils/HostDeviceShared.slangh** - Host device shared shader definitions

## Module Overview

SceneDefines is a shader header file that defines geometry type flags and helper macros for scene compilation. It provides compile-time flags for checking which geometry types are present in a scene (triangle meshes, displaced triangle meshes, curves, SDF grids, custom primitives) and helper macros for procedural geometry detection. These defines are used throughout the scene system to conditionally compile code based on the geometry types present in the scene, enabling efficient shader compilation and reducing shader code size.

## Component Specifications

### Geometry Type Defines

**Purpose**: Defines for geometry types present in scene.

**Values**:
- [`GEOMETRY_TYPE_NONE`](Source/Falcor/Scene/SceneDefines.slangh:33) - `0` - No geometry
- [`GEOMETRY_TYPE_TRIANGLE_MESH`](Source/Falcor/Scene/SceneDefines.slangh:34) - `1` - Triangle mesh geometry
- [`GEOMETRY_TYPE_DISPLACED_TRIANGLE_MESH`](Source/Falcor/Scene/SceneDefines.slangh:35) - `2` - Displaced triangle mesh geometry
- [`GEOMETRY_TYPE_CURVE`](Source/Falcor/Scene/SceneDefines.slangh:36) - `3` - Curve geometry
- [`GEOMETRY_TYPE_SDF_GRID`](Source/Falcor/Scene/SceneDefines.slangh:37) - `5` - SDF grid geometry
- [`GEOMETRY_TYPE_CUSTOM`](Source/Falcor/Scene/SceneDefines.slangh:38) - `6` - Custom primitive geometry

### Geometry Type Flags

**Purpose**: Bit field for geometry types present in scene.

**Definition**:
- `SCENE_GEOMETRY_TYPES` - Bit field containing all geometry type flags

### Helper Macros

**Purpose**: Helper macros for checking geometry types.

**Geometry Type Presence**:
- [`SCENE_HAS_GEOMETRY_TYPE(_type_)`](Source/Falcor/Scene/SceneDefines.slangh:40) - Check if specific geometry type is present
  - Parameters: `_type_` - Geometry type flag
  - Returns: Non-zero if geometry type is present

**Procedural Geometry Detection**:
- [`SCENE_HAS_PROCEDURAL_GEOMETRY()`](Source/Falcor/Scene/SceneDefines.slangh:41) - Check if any procedural geometry is present
  - Returns: Non-zero if any procedural geometry (displaced triangle meshes, curves, SDF grids, custom primitives) is present

## Technical Details

### Geometry Type System

**Bit Field Layout**:
- Each bit represents a different geometry type
- Bit 0: Triangle mesh
- Bit 1: Displaced triangle mesh
- Bit 2: Curve
- Bit 3: Unused
- Bit 4: Unused
- Bit 5: SDF grid
- Bit 6: Custom primitive

**Geometry Type Values**:
- `GEOMETRY_TYPE_NONE` - `0` - No geometry
- `GEOMETRY_TYPE_TRIANGLE_MESH` - `1` - Triangle mesh geometry
- `GEOMETRY_TYPE_DISPLACED_TRIANGLE_MESH` - `2` - Displaced triangle mesh geometry
- `GEOMETRY_TYPE_CURVE` - `3` - Curve geometry
- `GEOMETRY_TYPE_SDF_GRID` - `5` - SDF grid geometry
- `GEOMETRY_TYPE_CUSTOM` - `6` - Custom primitive geometry

**Geometry Type Flags**:
- `SCENE_GEOMETRY_TYPES` - Bit field containing all geometry type flags
- Used for checking which geometry types are present
- Enables conditional compilation based on geometry types

### Helper Macros

**Geometry Type Checking**:
- `SCENE_HAS_GEOMETRY_TYPE(_type_)` - Macro for checking if specific geometry type is present
  - Compares `SCENE_GEOMETRY_TYPES` with shifted bit
  - Returns non-zero if geometry type is present
  - Used for conditional compilation

**Procedural Geometry Detection**:
- `SCENE_HAS_PROCEDURAL_GEOMETRY()` - Macro for checking if any procedural geometry is present
  - Compares `SCENE_GEOMETRY_TYPES` with mask of procedural geometry types
  - Returns non-zero if any procedural geometry is present
  - Used for conditional compilation

## Integration Points

### Scene Integration

**Scene Compilation**:
- Defines geometry type flags
- Used for conditional compilation
- Reduces shader code size
- Enables efficient shader compilation

**Geometry Type Detection**:
- Compile-time geometry type checking
- Runtime geometry type checking
- Procedural geometry detection

### Shader Integration

**Host Device Shared**:
- [`Utils/HostDeviceShared.slangh`](Source/Falcor/Scene/SceneDefines.slangh:29) - Host device shared definitions
- Provides common shader definitions
- Integrates with scene defines

## Architecture Patterns

### Compile-Time Configuration Pattern

- Preprocessor defines for geometry types
- Bit field for efficient checking
- Helper macros for common operations

### Conditional Compilation Pattern

- Compile-time geometry type detection
- Conditional code compilation
- Reduced shader code size

## Code Patterns

### Geometry Type Checking Pattern

```slang
#define SCENE_HAS_GEOMETRY_TYPE(_type_) ((SCENE_GEOMETRY_TYPES & (1u << _type_)) != 0u)
```

### Procedural Geometry Detection Pattern

```slang
#define SCENE_HAS_PROCEDURAL_GEOMETRY() ((SCENE_GEOMETRY_TYPES & ~(1u << GEOMETRY_TYPE_TRIANGLE_MESH)) != 0u)
```

## Use Cases

### Scene Compilation

- **Geometry Type Checking**:
  - Check if triangle meshes are present
  - Check if displaced triangle meshes are present
  - Check if curves are present
  - Check if SDF grids are present
  - Check if custom primitives are present

- **Procedural Geometry Detection**:
  - Check if any procedural geometry is present
  - Optimize for triangle-only scenes
  - Optimize for mixed geometry scenes

### Shader Optimization

- **Conditional Compilation**:
  - Compile only needed code paths
  - Reduce shader code size
  - Improve compilation time

- **Code Generation**:
  - Generate optimized shaders
  - Avoid unnecessary branches
  - Use geometry type flags

## Performance Considerations

### Compilation Performance

**Compile-Time Checking**:
- Zero-cost runtime overhead
- Compile-time constant evaluation
- No runtime branching for geometry type checks

**Shader Code Size**:
- Reduced by conditional compilation
- Only includes needed code paths
- Improves compilation time

### Runtime Performance

**Geometry Type Flags**:
- Bit field for efficient checking
- Single bit test for geometry type presence
- No complex conditional logic

## Limitations

### Feature Limitations

- **Geometry Type Flags**:
- Limited to 7 geometry types
- No support for custom geometry types beyond the 6 defined
- Bit field may overflow if more than 32 geometry types needed

- **Compile-Time Only**:
- No runtime geometry type modification
- Geometry types must be known at compile time
- Cannot add geometry types dynamically

### Performance Limitations

- **Bit Field Size**:
- 32-bit bit field limits to 32 geometry types
- May need expansion for future geometry types
- Bit shifts limited to 32 bits

## Best Practices

### Shader Compilation

- **Geometry Type Flags**:
  - Use SCENE_HAS_GEOMETRY_TYPE macro for checking specific geometry types
  - Use SCENE_HAS_PROCEDURAL_GEOMETRY macro for checking procedural geometry
  - Avoid manual bit manipulation

- **Conditional Compilation**:
  - Use geometry type flags for conditional compilation
  - Reduce shader code size
  - Improve compilation time

### Code Organization

- **Geometry Type Defines**:
  - Group related geometry type defines
  - Use consistent naming convention
  - Document geometry type values

- **Helper Macros**:
  - Provide helper macros for common operations
  - Use clear macro names
  - Document macro usage

### Performance Optimization

- **Compile-Time Optimization**:
  - Use compile-time constants
  - Avoid runtime checks
  - Use bit field for efficient checking

- **Shader Code Size**:
  - Minimize code size with conditional compilation
  - Only include needed code paths
  - Use efficient bit operations

## Progress Log

- **2026-01-08T00:40:00Z**: SceneDefines analysis completed. Analyzed SceneDefines.slangh (43 lines) containing geometry type defines and helper macros for scene compilation. Documented GEOMETRY_TYPE_NONE (0), GEOMETRY_TYPE_TRIANGLE_MESH (1), GEOMETRY_TYPE_DISPLACED_TRIANGLE_MESH (2), GEOMETRY_TYPE_CURVE (3), GEOMETRY_TYPE_SDF_GRID (5), GEOMETRY_TYPE_CUSTOM (6), SCENE_GEOMETRY_TYPES bit field, SCENE_HAS_GEOMETRY_TYPE(_type_) helper macro for checking if specific geometry type is present, and SCENE_HAS_PROCEDURAL_GEOMETRY() helper macro for checking if any procedural geometry is present. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The SceneDefines module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
