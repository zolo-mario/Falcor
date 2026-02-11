# Math - Mathematical Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **AABB** - Axis-aligned bounding box
- [x] **BitTricks** - Bit manipulation tricks
- [x] **Common** - Common math functions
- [x] **CubicSpline** - Cubic spline interpolation
- [x] **DiffMathHelpers** - Differential math helpers
- [x] **FalcorMath** - Falcor math utilities
- [x] **Float16** - Half-precision floating point
- [x] **FNVHash** - FNV hash function
- [x] **FormatConversion** - Format conversion utilities
- [x] **HalfUtils** - Half-precision utilities
- [x] **HashUtils** - Hash utilities
- [x] **IntervalArithmetic** - Interval arithmetic
- [x] **MathConstants** - Mathematical constants
- [x] **MathHelpers** - Math helper functions
- [x] **Matrix** - Matrix operations
- [x] **MatrixJson** - Matrix JSON serialization
- [x] **MatrixMath** - Matrix math operations
- [x] **MatrixTypes** - Matrix type definitions
- [x] **PackedFormats** - Packed format definitions
- [x] **PackedFormats.slang** - Packed format shader definitions
- [x] **Quaternion** - Quaternion operations
- [x] **QuaternionMath** - Quaternion math operations
- [x] **QuaternionTypes** - Quaternion type definitions
- [x] **Ray** - Ray representation
- [x] **Rectangle** - Rectangle operations
- [x] **ScalarJson** - Scalar JSON serialization
- [x] **ScalarMath** - Scalar math operations
- [x] **ScalarTypes** - Scalar type definitions
- [x] **ShadingFrame** - Shading frame utilities
- [x] **SphericalHarmonics** - Spherical harmonics
- [x] **Vector** - Vector operations
- [x] **VectorJson** - Vector JSON serialization
- [x] **VectorMath** - Vector math operations
- [x] **VectorSwizzle** - Vector swizzling
- [x] **VectorTypes** - Vector type definitions

### External Dependencies

- **Core/API** - Graphics API (RtAABB)
- **Core/Error** - Error handling
- **Utils/Math** - Internal math dependencies

## Module Overview

The Math module provides comprehensive mathematical utilities for the Falcor rendering framework. It includes vector and matrix types and operations, quaternion operations, bounding boxes, rays, rectangles, mathematical constants, helper functions, format conversions, hash utilities, and various specialized math utilities. These utilities are essential for 3D graphics, rendering, and computational tasks.

## Component Specifications

### Vector Types

**Files**:
- [`VectorTypes.h`](Source/Falcor/Utils/Math/VectorTypes.h:1) - Vector type definitions
- [`Vector.h`](Source/Falcor/Utils/Math/Vector.h:1) - Vector operations header
- [`VectorMath.h`](Source/Falcor/Utils/Math/VectorMath.h:1) - Vector math operations
- [`VectorJson.h`](Source/Falcor/Utils/Math/VectorJson.h:1) - Vector JSON serialization
- [`VectorSwizzle2.inl.h`](Source/Falcor/Utils/Math/VectorSwizzle2.inl.h:1) - 2D vector swizzling
- [`VectorSwizzle3.inl.h`](Source/Falcor/Utils/Math/VectorSwizzle3.inl.h:1) - 3D vector swizzling
- [`VectorSwizzle4.inl.h`](Source/Falcor/Utils/Math/VectorSwizzle4.inl.h:1) - 4D vector swizzling

**Purpose**: Vector type definitions and operations.

**Key Features**:
- Template-based vector types (1D, 2D, 3D, 4D)
- Multiple scalar type support (float, int, uint, bool, float16_t)
- Union-based storage for flexible naming
- Element-wise math operators (+, -, *, /)
- Free-standing functions for vector operations (dot, cross, etc.)
- Slang-compatible semantics
- Vector swizzling support

**Data Structures**:

**vector<T, N> Template**:
- [`dimension`](Source/Falcor/Utils/Math/VectorTypes.h:57) - Static dimension constant (1, 2, 3, or 4)
- [`value_type`](Source/Falcor/Utils/Math/VectorTypes.h:58) - Scalar type alias

**vector<T, 1>**:
- Union storage: `x`, `r`, `s` (single component)
- Constructors: default, copy, explicit from scalar
- Index operator: `operator[](int index)`
- Static `length()` method

**vector<T, 2>**:
- Union storage: `x, y` (XY) or `r, g` (RG)
- Union storage: `s, t` (ST)
- Constructors: default, copy, explicit from components
- Index operator: `operator[](int index)`
- Static `length()` method

**vector<T, 3>**:
- Union storage: `x, y, z` (XYZ) or `r, g, b` (RGB)
- Union storage: `s, t, p` (STP)
- Constructors: default, copy, explicit from components
- Index operator: `operator[](int index)`
- Static `length()` method

**vector<T, 4>**:
- Union storage: `x, y, z, w` (XYZW) or `r, g, b, a` (RGBA)
- Union storage: `s, t, p, q` (STPQ)
- Constructors: default, copy, explicit from components
- Index operator: `operator[](int index)`
- Static `length()` method

**Type Aliases**:
- [`bool1`, [`bool2`](Source/Falcor/Utils/Math/VectorTypes.h:250), [`bool3`](Source/Falcor/Utils/Math/VectorTypes.h:251), [`bool4`](Source/Falcor/Utils/Math/VectorTypes.h:252)
- [`int1`](Source/Falcor/Utils/Math/VectorTypes.h:254), [`int2`](Source/Falcor/Utils/Math/VectorTypes.h:255), [`int3`](Source/Falcor/Utils/Math/VectorTypes.h:256), [`int4`](Source/Falcor/Utils/Math/VectorTypes.h:257)
- [`uint1`](Source/Falcor/Utils/Math/VectorTypes.h:258), [`uint2`](Source/Falcor/Utils/Math/VectorTypes.h:259), [`uint3`](Source/Falcor/Utils/Math/VectorTypes.h:260), [`uint4`](Source/Falcor/Utils/Math/VectorTypes.h:261)
- [`float1`](Source/Falcor/Utils/Math/VectorTypes.h:262), [`float2`](Source/Falcor/Utils/Math/VectorTypes.h:263), [`float3`](Source/Falcor/Utils/Math/VectorTypes.h:264), [`float4`](Source/Falcor/Utils/Math/VectorTypes.h:265)
- [`float16_t1`](Source/Falcor/Utils/Math/VectorTypes.h:266), [`float16_t2`](Source/Falcor/Utils/Math/VectorTypes.h:267), [`float16_t3`](Source/Falcor/Utils/Math/VectorTypes.h:268), [`float16_t4`](Source/Falcor/Utils/Math/VectorTypes.h:269)

**Technical Details**:

**Union-Based Storage**:
- Flexible component naming (x,y,z,w or r,g,b,a or s,t,p,q)
- Memory-efficient storage
- Type-safe access
- Slang-compatible layout

**Slang Semantics**:
- Element-wise math operators (+, -, *, /)
- Free-standing functions (dot, cross, length, normalize, etc.)
- Compatible with shader code generation

**Vector Swizzling**:
- Component extraction and reordering
- Efficient compile-time operations
- Support for 2D, 3D, 4D vectors

**Use Cases**:
- 3D graphics operations
- Vector math
- Color operations
- Position and direction handling
- Shader development

### Matrix Types

**Files**:
- [`MatrixTypes.h`](Source/Falcor/Utils/Math/MatrixTypes.h:1) - Matrix type definitions
- [`Matrix.h`](Source/Falcor/Utils/Math/Matrix.h:1) - Matrix operations header
- [`MatrixMath.h`](Source/Falcor/Utils/Math/MatrixMath.h:1) - Matrix math operations
- [`MatrixJson.h`](Source/Falcor/Utils/Math/MatrixJson.h:1) - Matrix JSON serialization

**Purpose**: Matrix type definitions and operations.

**Key Features**:
- Template-based matrix types (1-4 rows, 1-4 columns)
- Row-major storage
- Float scalar type only
- Element-wise math operators (+, -, *, /)
- Free-standing functions for matrix operations (mul, transpose, inverse, etc.)
- Identity and zero matrix constructors
- Matrix resizing support

**Data Structures**:

**matrix<T, RowCount, ColCount> Template**:
- [`getRowCount()`](Source/Falcor/Utils/Math/MatrixTypes.h:71) - Static row count
- [`getColCount()`](Source/Falcor/Utils/Math/MatrixTypes.h:72) - Static column count
- [`value_type`](Source/Falcor/Utils/Math/MatrixTypes.h:67) - Scalar type alias (T)
- [`RowType`](Source/Falcor/Utils/Math/MatrixTypes.h:68) - Row type (vector<T, ColCount>)
- [`ColType`](Source/Falcor/Utils/Math/MatrixTypes.h:69) - Column type (vector<T, RowCount>)

**Constructors**:
- Default constructor (identity matrix)
- Initializer list constructor
- Copy/move constructors
- Matrix resizing constructor (from different dimensions)

**Accessors**:
- [`operator[](int r)`](Source/Falcor/Utils/Math/MatrixTypes.h:123) - Row access (returns RowType)
- [`getRow(int r)`](Source/Falcor/Utils/Math/MatrixTypes.h:134) - Get row
- [`getCol(int col)`](Source/Falcor/Utils/Math/MatrixTypes.h:151) - Get column (returns ColType)
- [`setRow(int r, const RowType& v)`](Source/Falcor/Utils/Math/MatrixTypes.h:145) - Set row
- [`setCol(int col, const ColType& v)`](Source/Falcor/Utils/Math/MatrixTypes.h:160) - Set column

**Static Methods**:
- [`zeros()`](Source/Falcor/Utils/Math/MatrixTypes.h:115) - Zero matrix
- [`identity()`](Source/Falcor/Utils/Math/MatrixTypes.h:118) - Identity matrix

**Data Access**:
- [`data()`](Source/Falcor/Utils/Math/MatrixTypes.h:120) - Pointer to raw data
- [`const data()`](Source/Falcor/Utils/Math/MatrixTypes.h:121) - Const pointer to raw data

**Type Aliases**:
- [`float2x2`](Source/Falcor/Utils/Math/MatrixTypes.h:202) - 2x2 float matrix
- [`float3x3`](Source/Falcor/Utils/Math/MatrixTypes.h:204) - 3x3 float matrix
- [`float1x4`](Source/Falcor/Utils/Math/MatrixTypes.h:206) - 1x4 float matrix
- [`float2x4`](Source/Falcor/Utils/Math/MatrixTypes.h:207) - 2x4 float matrix
- [`float3x4`](Source/Falcor/Utils/Math/MatrixTypes.h:208) - 3x4 float matrix
- [`float4x4`](Source/Falcor/Utils/Math/MatrixTypes.h:209) - 4x4 float matrix

**Technical Details**:

**Row-Major Storage**:
- Rows stored contiguously
- Column access requires gathering
- Efficient for row-wise operations
- Compatible with DirectX/Direct3D

**Matrix Resizing**:
- Supports copying from smaller matrices
- Larger matrices initialized with identity
- Smaller matrices copied as-is

**Slang Semantics**:
- Element-wise math operators
- Free-standing functions for matrix operations
- Compatible with shader code generation

**Use Cases**:
- 3D transformations
- Projection matrices
- View matrices
- Model matrices
- Matrix operations

### Quaternion Types

**Files**:
- [`QuaternionTypes.h`](Source/Falcor/Utils/Math/QuaternionTypes.h:1) - Quaternion type definitions
- [`Quaternion.h`](Source/Falcor/Utils/Math/Quaternion.h:1) - Quaternion operations header
- [`QuaternionMath.h`](Source/Falcor/Utils/Math/QuaternionMath.h:1) - Quaternion math operations

**Purpose**: Quaternion type definitions and operations.

**Key Features**:
- Template-based quaternion type (float, double)
- (x, y, z, w) storage for GPU interop
- Quaternion normalization support
- Identity quaternion
- Slang-compatible semantics
- Free-standing functions for quaternion operations

**Data Structures**:

**quat<T> Template**:
- [`value_type`](Source/Falcor/Utils/Math/QuaternionTypes.h:57) - Scalar type alias (T)
- [`x, y, z, w`](Source/Falcor/Utils/Math/QuaternionTypes.h:60) - Quaternion components
- Static assertion: T must be float or double

**Constructors**:
- Default constructor: (0, 0, 0, 1) - Identity quaternion
- Explicit constructor from vector<T, 3> and w
- Explicit constructor from x, y, z, w components

**Accessors**:
- [`operator[](size_t i)`](Source/Falcor/Utils/Math/QuaternionTypes.h:71) - Component access

**Static Methods**:
- [`identity()`](Source/Falcor/Utils/Math/QuaternionTypes.h:68) - Identity quaternion

**Type Aliases**:
- [`quatf`](Source/Falcor/Utils/Math/QuaternionTypes.h:75) - Float quaternion alias

**Technical Details**:

**Quaternion Form**:
- q = w + xi + yj + zk
- w, x, y, z are real numbers
- i, j, k are imaginary units

**Normalization**:
- Normalized if: w^2 + x^2 + y^2 + z^2 = 1

**GPU Interop**:
- (x, y, z, w) storage for better GPU interop
- Matches shader quaternion layout

**Use Cases**:
- Rotations
- Orientation representation
- Smooth interpolation
- Camera orientation
- Animation rotations

### AABB

**Files**:
- [`AABB.h`](Source/Falcor/Utils/Math/AABB.h:1) - AABB header
- [`AABB.slang`](Source/Falcor/Utils/Math/AABB.slang:1) - AABB shader

**Purpose**: Axis-aligned bounding box for spatial queries.

**Key Features**:
- Min/max point storage
- Invalid state handling
- Box operations (include, intersect, union)
- Spatial queries (contains, overlaps)
- Geometric properties (center, extent, area, volume, radius)
- Matrix transformation support
- GPU-side implementation in AABB.slang

**Data Structures**:

**AABB Struct**:
- [`minPoint`](Source/Falcor/Utils/Math/AABB.h:45) - Minimum point (initialized to +inf)
- [`maxPoint`](Source/Falcor/Utils/Math/AABB.h:46) - Maximum point (initialized to -inf)

**Constructors**:
- Default constructor: Invalid box (+inf, -inf)
- Single point constructor: Box set to point
- Min/max constructor: Box from min and max points
- RtAABB constructor: From raytracing AABB

**Operations**:
- [`set(const float3& p)`](Source/Falcor/Utils/Math/AABB.h:61) - Set box to single point
- [`set(const float3& pmin, const float3& pmax)`](Source/Falcor/Utils/Math/AABB.h:64) - Set box to min/max points
- [`invalidate()`](Source/Falcor/Utils/Math/AABB.h:71) - Invalidate box to +/-inf
- [`include(const float3& p)`](Source/Falcor/Utils/Math/AABB.h:81) - Grow box to include point
- [`include(const AABB& b)`](Source/Falcor/Utils/Math/AABB.h:89) - Grow box to include another box
- [`intersection(const AABB& b)`](Source/Falcor/Utils/Math/AABB.h:97) - Make box be intersection of two boxes
- [`operator|=(const AABB& rhs)`](Source/Falcor/Utils/Math/AABB.h:194) - Union of two boxes
- [`operator|(const AABB& rhs)`](Source/Falcor/Utils/Math/AABB.h:197) - Union of two boxes
- [`operator&=(const AABB& rhs)`](Source/Falcor/Utils/Math/AABB.h:204) - Intersection of two boxes
- [`operator&(const AABB& rhs)`](Source/Falcor/Utils/Math/AABB.h:207) - Intersection of two boxes

**Queries**:
- [`valid() const`](Source/Falcor/Utils/Math/AABB.h:78) - Check if box is valid (all dimensions >= 0)
- [`overlaps(AABB b)`](Source/Falcor/Utils/Math/AABB.h:105) - Check if two AABBs overlap
- [`contains(const AABB& b)`](Source/Falcor/Utils/Math/AABB.h:112) - Check if AABB is fully contained

**Properties**:
- [`center() const`](Source/Falcor/Utils/Math/AABB.h:122) - Box center
- [`extent() const`](Source/Falcor/Utils/Math/AABB.h:128) - Box extent
- [`area() const`](Source/Falcor/Utils/Math/AABB.h:134) - Surface area
- [`volume() const`](Source/Falcor/Utils/Math/AABB.h:144) - Volume
- [`radius() const`](Source/Falcor/Utils/Math/AABB.h:154) - Radius of minimal enclosing sphere

**Transformation**:
- [`transform(const float4x4& mat) const`](Source/Falcor/Utils/Math/AABB.h:161) - Transform box by matrix

**Comparison**:
- [`operator==(const AABB& rhs) const`](Source/Falcor/Utils/Math/AABB.h:188) - Check equality
- [`operator!=(const AABB& rhs) const`](Source/Falcor/Utils/Math/AABB.h:191) - Check inequality

**Conversion**:
- [`operator RtAABB() const`](Source/Falcor/Utils/Math/AABB.h:214) - Convert to RtAABB

**Technical Details**:

**Invalid State**:
- minPoint = +inf, maxPoint = -inf
- Any minPoint > maxPoint means invalid

**Spatial Operations**:
- Union: Combine boxes to include all points
- Intersection: Find overlapping region
- Include: Grow box to include point/box

**Geometric Properties**:
- Center: (min + max) * 0.5
- Extent: max - min
- Area: 2*(xy + xz + yz)
- Volume: x * y * z
- Radius: 0.5 * length(extent)

**Matrix Transformation**:
- Transforms min and max points separately
- Uses matrix columns for transformation
- Handles invalid boxes

**Use Cases**:
- Bounding volume hierarchies
- Spatial queries
- Frustum culling
- Collision detection
- Octree/BVH construction

### Ray

**Files**:
- [`Ray.h`](Source/Falcor/Utils/Math/Ray.h:1) - Ray header

**Purpose**: Ray type for ray tracing and intersection testing.

**Key Features**:
- Origin, direction, and tMin/tMax range
- Matches DirectX RayDesc layout
- Static assertions for layout verification
- 32-byte size for GPU compatibility

**Data Structures**:

**Ray Struct**:
- [`origin`](Source/Falcor/Utils/Math/Ray.h:39) - Ray origin (float3)
- [`tMin`](Source/Falcor/Utils/Math/Ray.h:40) - Minimum t value (float)
- [`dir`](Source/Falcor/Utils/Math/Ray.h:41) - Ray direction (float3)
- [`tMax`](Source/Falcor/Utils/Math/Ray.h:42) - Maximum t value (float)

**Constructors**:
- Default constructor: origin(0,0,0), dir(0,0,0), tMin(0), tMax(max)
- Explicit constructor: origin, dir, tMin, tMax parameters

**Technical Details**:

**Layout Verification**:
- Static assertions ensure correct memory layout
- Matches DirectX RayDesc exactly
- 32-byte total size

**Ray Parameters**:
- origin: Starting point of ray
- dir: Normalized direction vector
- tMin: Minimum intersection parameter (default: 0)
- tMax: Maximum intersection parameter (default: +inf)

**Use Cases**:
- Ray tracing
- Intersection testing
- Scene traversal
- Picking operations

### Common

**Files**:
- [`Common.h`](Source/Falcor/Utils/Math/Common.h:1) - Common math utilities

**Purpose**: Common mathematical helper functions.

**Key Features**:
- Power of two detection
- Division with rounding up
- Alignment helper

**Functions**:

**Power Detection**:
- [`isPowerOf2<T>(T a)`](Source/Falcor/Utils/Math/Common.h:36) - Check if integer is power of two
  - Returns: (a & (a - 1)) == 0

**Division**:
- [`div_round_up<T>(T a, T b)`](Source/Falcor/Utils/Math/Common.h:43) - Divide with rounding up
  - Returns: (a + b - 1) / b

**Alignment**:
- [`align_to<T>(T alignment, T value)`](Source/Falcor/Utils/Math/Common.h:50) - Align value to alignment
  - Returns: ((value + alignment - 1) / alignment) * alignment

**Technical Details**:

**Power of Two**:
- Bitwise operation: (a & (a - 1)) == 0
- Only works for integer types
- Template-based for any integral type

**Round Up Division**:
- Formula: (a + b - 1) / b
- Ensures result is >= a / b
- Useful for array allocation

**Alignment**:
- Formula: ((value + alignment - 1) / alignment) * alignment
- Rounds up to nearest multiple of alignment
- Useful for memory allocation and GPU resources

**Use Cases**:
- Texture size calculations
- Memory alignment
- Array allocation
- Power-of-two checks

### Scalar Types

**Files**:
- [`ScalarTypes.h`](Source/Falcor/Utils/Math/ScalarTypes.h:1) - Scalar type definitions

**Purpose**: Scalar type definitions for math library.

**Key Features**:
- Basic scalar type definitions
- Used by vector, matrix, and quaternion types

**Technical Details**:

**Scalar Types**:
- float, double, int, uint, bool, float16_t
- Type-safe operations
- Template-based for flexibility

**Use Cases**:
- Scalar operations
- Type safety
- Template instantiation

## Architecture Patterns

### Template-Based Design
- Generic types for flexibility
- Compile-time optimization
- Type safety
- Zero-overhead abstractions

### Union-Based Storage
- Flexible component naming
- Memory efficiency
- Type-safe access
- GPU interop compatibility

### Slang Compatibility
- Shader-compatible semantics
- Element-wise operators
- Free-standing functions
- Direct GPU mapping

## Technical Details

### Vector Implementation

**Storage Layout**:
- Union-based for flexible naming
- Contiguous component storage
- Direct memory access

**Swizzling**:
- Compile-time component extraction
- Efficient reordering
- Type-safe operations

**Math Operators**:
- Element-wise operations (+, -, *, /)
- Comparison operators (==, !=)
- Index operator for access

### Matrix Implementation

**Row-Major Storage**:
- Rows stored contiguously
- Column access requires gathering
- DirectX/Direct3D compatible

**Matrix Operations**:
- Identity and zero matrices
- Matrix multiplication
- Transpose
- Inverse (for square matrices)
- Resizing support

### Quaternion Implementation

**Storage Layout**:
- (x, y, z, w) for GPU interop
- Matches shader quaternion layout

**Quaternion Operations**:
- Normalization
- Multiplication
- Inverse
- Slerp interpolation
- Rotation conversion

### AABB Implementation

**Spatial Operations**:
- Union: max(min1, min2), min(max1, max2)
- Intersection: max(min1, min2), min(max1, max2)
- Include: min(min, point), max(max, point)

**Geometric Properties**:
- Center: average of min and max
- Extent: difference between max and min
- Area: surface area calculation
- Volume: product of extents
- Radius: half of diagonal length

### Ray Implementation

**Layout**:
- Matches DirectX RayDesc exactly
- 32-byte total size
- origin (12 bytes) + tMin (4 bytes) + dir (12 bytes) + tMax (4 bytes)

**Use Cases**:
- Ray tracing
- Intersection testing
- Scene traversal
- Picking operations

## Progress Log

- **2026-01-07T19:13:49Z**: Math sub-module analysis completed. Analyzed VectorTypes, MatrixTypes, QuaternionTypes, AABB, Ray, and Common utilities. Documented vector types (1D-4D with multiple scalar types), matrix types (row-major, 1-4 rows/cols), quaternion types, axis-aligned bounding box operations, ray structure, and common math functions (power of two, round up division, alignment). Created comprehensive technical specification with detailed code patterns, data structures, and use cases.

## Next Steps

Proceed to analyze Sampling sub-module to understand sampling utilities (SampleGenerator, AliasTable, LowDiscrepancy, Pseudorandom).
