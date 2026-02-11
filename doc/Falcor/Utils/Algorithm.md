# Algorithm - GPU and CPU Algorithms

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **BitonicSort** - GPU-based bitonic sorting
- [x] **DirectedGraph** - Directed graph data structure
- [x] **DirectedGraphTraversal** - Graph traversal algorithms
- [x] **ParallelReduction** - GPU parallel reduction
- [x] **PrefixSum** - GPU prefix sum (scan)
- [x] **UnionFind** - Union-find data structure

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture)
- **Core/State** - Compute state management (ComputeState)
- **Core/Program** - Shader program management (Program, ProgramVars)
- **Utils/Logger** - Logging utilities

## Module Overview

The Algorithm module provides a collection of GPU and CPU algorithms used throughout the Falcor framework. It includes GPU-based parallel algorithms for sorting, reduction, and prefix sum operations, as well as CPU-based data structures for graph operations and union-find. These algorithms are optimized for performance and are used in various rendering and computation tasks.

## Component Specifications

### BitonicSort

**Files**:
- [`BitonicSort.h`](Source/Falcor/Utils/Algorithm/BitonicSort.h:1) - Bitonic sort header
- [`BitonicSort.cs.slang`](Source/Falcor/Utils/Algorithm/BitonicSort.cs.slang:1) - Bitonic sort shader
- [`BitonicSort.cpp`](Source/Falcor/Utils/Algorithm/BitonicSort.cpp:1) - Bitonic sort implementation

**Purpose**: In-place bitonic sort on GPU for short sequences.

**Key Features**:
- GPU-based in-place sorting
- Chunk-based sorting (sorts chunks of N elements independently)
- Efficient for short sequences
- Time complexity: O(N*log^2(N))
- Excellent parallelization with minimal branching
- Horizontal operations within warps
- Shared memory across warps
- Requires NVIDIA GPU and NVAPI

**Core Methods**:

**Constructor**:
- [`BitonicSort(ref<Device> pDevice)`](Source/Falcor/Utils/Algorithm/BitonicSort.h:52) - Constructor, throws exception on error

**Execute**:
- [`execute(RenderContext* pRenderContext, ref<Buffer> pData, uint32_t totalSize, uint32_t chunkSize, uint32_t groupSize = 256)`](Source/Falcor/Utils/Algorithm/BitonicSort.h:65) - Execute bitonic sort
  - Parameters:
    - `pRenderContext` - The render context
    - `pData` - Data buffer to sort in-place
    - `totalSize` - Total number of elements (doesn't need to be multiple of chunkSize)
    - `chunkSize` - Number of elements per chunk (must be power-of-two in [1, groupSize])
    - `groupSize` - Thread group size (must be power-of-two in [1, 1024], default 256)
  - Returns: True if successful, false if error occurred

**Data Structures**:

**BitonicSort Class**:
- [`mpDevice`](Source/Falcor/Utils/Algorithm/BitonicSort.h:68) - Device reference
- [`mSort.pState`](Source/Falcor/Utils/Algorithm/BitonicSort.h:72) - Compute state
- [`mSort.pProgram`](Source/Falcor/Utils/Algorithm/BitonicSort.h:73) - Shader program
- [`mSort.pVars`](Source/Falcor/Utils/Algorithm/BitonicSort.h:74) - Program variables

**Technical Details**:

**Algorithm**:
- Bitonic sort is a comparison-based sorting algorithm
- Works by recursively building bitonic sequences and then sorting them
- Particularly well-suited for GPU parallelization
- Each chunk is sorted independently in ascending order

**Performance Characteristics**:
- Time complexity: O(N*log^2(N))
- Excellent parallelization
- Minimal branching
- Uses warp-level operations
- Shared memory for inter-warp communication

**Requirements**:
- NVIDIA GPU
- NVAPI support
- Power-of-two chunk sizes
- Power-of-two group sizes

**Use Cases**:
- Sorting small to medium-sized arrays
- Per-pixel or per-vertex sorting
- Parallel sorting tasks
- Real-time applications requiring GPU sorting

### ParallelReduction

**Files**:
- [`ParallelReduction.h`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:1) - Parallel reduction header
- [`ParallelReduction.cs.slang`](Source/Falcor/Utils/Algorithm/ParallelReduction.cs.slang:1) - Parallel reduction shader
- [`ParallelReduction.cpp`](Source/Falcor/Utils/Algorithm/ParallelReduction.cpp:1) - Parallel reduction implementation
- [`ParallelReductionType.slangh`](Source/Falcor/Utils/Algorithm/ParallelReductionType.slangh:1) - Reduction type definitions

**Purpose**: GPU parallel reduction over texture pixels.

**Key Features**:
- Recursive reduction on blocks of 1024 elements
- Multiple reduction operations (Sum, MinMax)
- Type-safe template interface
- Support for various texture formats
- Optional GPU result buffer for async readback
- Numerical error between pairwise and naive summation

**Core Methods**:

**Constructor**:
- [`ParallelReduction(ref<Device> pDevice)`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:58) - Constructor, throws exception on failure

**Execute** (Template):
- [`execute<T>(RenderContext* pRenderContext, const ref<Texture>& pInput, Type operation, T* pResult = nullptr, ref<Buffer> pResultBuffer = nullptr, uint64_t resultOffset = 0)`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:85) - Execute parallel reduction
  - Parameters:
    - `pRenderContext` - The render context
    - `pInput` - Input texture
    - `operation` - Reduction operation (Sum or MinMax)
    - `pResult` - Optional CPU result pointer (requires GPU flush)
    - `pResultBuffer` - Optional GPU result buffer (16B for Sum, 32B for MinMax)
    - `resultOffset` - Optional byte offset into result buffer
  - Template parameter T must be compatible with texture format:
    - `float4` for floating-point formats (float, snorm, unorm)
    - `uint4` for unsigned integer formats
    - `int4` for signed integer formats

**Get Memory Usage**:
- [`getMemoryUsageInBytes()`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:94) - Get memory usage in bytes

**Data Structures**:

**Reduction Types**:
- [`Type::Sum`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:53) - Summation operation
- [`Type::MinMax`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:54) - Minimum and maximum operation

**ParallelReduction Class**:
- [`mpDevice`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:99) - Device reference
- [`mpState`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:101) - Compute state
- [`mpInitialProgram`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:102) - Initial reduction program
- [`mpFinalProgram`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:103) - Final reduction program
- [`mpVars`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:104) - Program variables
- [`mpBuffers[2]`](Source/Falcor/Utils/Algorithm/ParallelReduction.h:106) - Intermediate buffers for reduction iterations

**Technical Details**:

**Algorithm**:
- Recursive reduction on blocks of n = 1024 elements
- Total iterations: ceil(log2(N)/10), where N is total elements
- Uses ping-pong buffers for intermediate results
- Two-phase reduction: initial reduction and final reduction

**Performance Characteristics**:
- Time complexity: O(N)
- Excellent GPU parallelization
- Minimal synchronization
- Efficient memory access patterns

**Numerical Accuracy**:
- Sum operation error between pairwise and naive summation
- Blocks of size 1024 provide good balance
- Suitable for most rendering applications

**Result Buffer Sizes**:
- Sum operation: 16 bytes (float4)
- MinMax operation: 32 bytes (2 x float4 for min and max)

**Use Cases**:
- Computing texture statistics (sum, min, max)
- Image analysis
- Histogram computation
- Variance calculation
- Luminance computation

### PrefixSum

**Files**:
- [`PrefixSum.h`](Source/Falcor/Utils/Algorithm/PrefixSum.h:1) - Prefix sum header
- [`PrefixSum.cs.slang`](Source/Falcor/Utils/Algorithm/PrefixSum.cs.slang:1) - Prefix sum shader
- [`PrefixSum.cpp`](Source/Falcor/Utils/Algorithm/PrefixSum.cpp:1) - Prefix sum implementation

**Purpose**: GPU parallel prefix sum (exclusive scan) for uint32_t arrays.

**Key Features**:
- In-place computation
- Exclusive scan (y[i] = x[0] + ... + x[i-1], y[0] = 0)
- Efficient GPU implementation
- Optional total sum output
- Support for async GPU result buffer

**Core Methods**:

**Constructor**:
- [`PrefixSum(ref<Device> pDevice)`](Source/Falcor/Utils/Algorithm/PrefixSum.h:50) - Constructor, throws exception if creation failed

**Execute**:
- [`execute(RenderContext* pRenderContext, ref<Buffer> pData, uint32_t elementCount, uint32_t* pTotalSum = nullptr, ref<Buffer> pTotalSumBuffer = nullptr, uint64_t pTotalSumOffset = 0)`](Source/Falcor/Utils/Algorithm/PrefixSum.h:61) - Execute prefix sum
  - Parameters:
    - `pRenderContext` - The render context
    - `pData` - Buffer to compute prefix sum over (modified in-place)
    - `elementCount` - Number of elements to compute prefix sum over
    - `pTotalSum` - Optional CPU pointer for total sum (requires GPU sync)
    - `pTotalSumBuffer` - Optional GPU buffer for total sum (uint32_t)
    - `pTotalSumOffset` - Optional byte offset into total sum buffer

**Data Structures**:

**PrefixSum Class**:
- [`mpDevice`](Source/Falcor/Utils/Algorithm/PrefixSum.h:71) - Device reference
- [`mpComputeState`](Source/Falcor/Utils/Algorithm/PrefixSum.h:73) - Compute state
- [`mpPrefixSumGroupProgram`](Source/Falcor/Utils/Algorithm/PrefixSum.h:75) - Group prefix sum program
- [`mpPrefixSumGroupVars`](Source/Falcor/Utils/Algorithm/PrefixSum.h:76) - Group program variables
- [`mpPrefixSumFinalizeProgram`](Source/Falcor/Utils/Algorithm/PrefixSum.h:78) - Finalize program
- [`mpPrefixSumFinalizeVars`](Source/Falcor/Utils/Algorithm/PrefixSum.h:79) - Finalize program variables
- [`mpPrefixGroupSums`](Source/Falcor/Utils/Algorithm/PrefixSum.h:81) - Temporary buffer for prefix sum computation
- [`mpTotalSum`](Source/Falcor/Utils/Algorithm/PrefixSum.h:82) - Temporary buffer for total sum
- [`mpPrevTotalSum`](Source/Falcor/Utils/Algorithm/PrefixSum.h:83) - Temporary buffer for previous total sum

**Technical Details**:

**Algorithm**:
- Exclusive scan (prefix sum)
- In-place computation
- Two-phase approach: group-level prefix sum and global finalize
- Efficient GPU parallelization

**Exclusive Scan Definition**:
- y[0] = 0
- y[i] = x[0] + x[1] + ... + x[i-1] for i = 1..N

**Performance Characteristics**:
- Time complexity: O(N)
- Excellent GPU parallelization
- Efficient memory access patterns
- Minimal synchronization

**Use Cases**:
- Stream compaction
- Radix sort
- Histogram computation
- Work queue management
- Particle system updates

### DirectedGraph

**Files**:
- [`DirectedGraph.h`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:1) - Directed graph header
- [`DirectedGraphTraversal.h`](Source/Falcor/Utils/Algorithm/DirectedGraphTraversal.h:1) - Graph traversal header

**Purpose**: CPU-based directed graph data structure.

**Key Features**:
- Node and edge management
- Unique IDs for nodes and edges
- Incoming and outgoing edge tracking
- Efficient node/edge lookup
- Automatic edge cleanup on node removal

**Core Methods**:

**Node Management**:
- [`addNode()`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:50) - Add a node, returns unique ID
- [`removeNode(uint32_t id)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:60) - Remove a node and all associated edges
- [`doesNodeExist(uint32_t nodeId)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:160) - Check if node exists
- [`getNode(uint32_t nodeId)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:170) - Get node by ID

**Edge Management**:
- [`addEdge(uint32_t srcNode, uint32_t dstNode)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:88) - Add an edge, returns unique ID
- [`removeEdge(uint32_t edgeId)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:112) - Remove an edge
- [`doesEdgeExist(uint32_t edgeId)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:165) - Check if edge exists
- [`getEdge(uint32_t edgeId)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:183) - Get edge by ID

**ID Management**:
- [`getCurrentNodeId()`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:193) - Get current node ID counter
- [`getCurrentEdgeId()`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:194) - Get current edge ID counter

**Data Structures**:

**DirectedGraph Class**:
- [`mNodes`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:197) - Map of node ID to Node
- [`mEdges`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:198) - Map of edge ID to Edge
- [`mCurrentNodeId`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:199) - Current node ID counter
- [`mCurrentEdgeId`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:200) - Current edge ID counter

**Node Class**:
- [`mIncomingEdges`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:139) - List of incoming edge IDs
- [`mOutgoingEdges`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:140) - List of outgoing edge IDs
- [`getIncomingEdgeCount()`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:132) - Get incoming edge count
- [`getOutgoingEdgeCount()`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:131) - Get outgoing edge count
- [`getIncomingEdge(uint32_t i)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:134) - Get incoming edge by index
- [`getOutgoingEdge(uint32_t i)`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:135) - Get outgoing edge by index

**Edge Class**:
- [`mSrc`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:153) - Source node ID
- [`mDst`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:154) - Destination node ID
- [`getSourceNode()`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:147) - Get source node ID
- [`getDestNode()`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:148) - Get destination node ID

**Constants**:
- [`kInvalidID`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:41) - Invalid ID value ((uint32_t)-1)

**Technical Details**:

**Node Management**:
- Unique sequential IDs for nodes
- Automatic ID assignment
- Efficient lookup using unordered_map
- Automatic edge cleanup on node removal

**Edge Management**:
- Unique sequential IDs for edges
- Bidirectional edge tracking (incoming/outgoing)
- Efficient edge removal
- Automatic node reference cleanup

**Memory Management**:
- Hash-based storage for nodes and edges
- O(1) average lookup time
- Efficient memory usage

**Use Cases**:
- Render graph dependency tracking
- Task scheduling
- Resource dependency management
- Scene graph representation
- Animation state machine

### DirectedGraphTraversal

**Files**:
- [`DirectedGraphTraversal.h`](Source/Falcor/Utils/Algorithm/DirectedGraphTraversal.h:1) - Graph traversal header

**Purpose**: Graph traversal algorithms for directed graphs.

**Key Features**:
- Topological sort
- Cycle detection
- Depth-first search
- Breadth-first search
- Dependency resolution

**Technical Details**:

**Traversal Algorithms**:
- Topological sort for dependency ordering
- Cycle detection for validation
- DFS/BFS for graph exploration
- Efficient traversal with visited tracking

**Use Cases**:
- Render graph execution order
- Task scheduling
- Dependency resolution
- Scene graph traversal

### UnionFind

**Files**:
- [`UnionFind.h`](Source/Falcor/Utils/Algorithm/UnionFind.h:1) - Union-find header

**Purpose**: CPU-based union-find (disjoint-set) data structure.

**Key Features**:
- Template-based for any unsigned integral type
- Path compression optimization
- Union by size optimization
- Efficient set operations
- Set count tracking

**Core Methods**:

**Constructor**:
- [`UnionFind()`](Source/Falcor/Utils/Algorithm/UnionFind.h:45) - Default constructor
- [`UnionFind(size_t size)`](Source/Falcor/Utils/Algorithm/UnionFind.h:46) - Constructor with initial size

**Initialization**:
- [`reset(size_t size)`](Source/Falcor/Utils/Algorithm/UnionFind.h:48) - Reset to initial state with given size

**Set Operations**:
- [`findSet(T v)`](Source/Falcor/Utils/Algorithm/UnionFind.h:57) - Find the root of the set containing v (with path compression)
- [`connectedSets(T v0, T v1)`](Source/Falcor/Utils/Algorithm/UnionFind.h:67) - Check if v0 and v1 are in the same set
- [`unionSet(T v0, T v1)`](Source/Falcor/Utils/Algorithm/UnionFind.h:69) - Union the sets containing v0 and v1 (union by size)

**Query**:
- [`getSetCount()`](Source/Falcor/Utils/Algorithm/UnionFind.h:86) - Get the number of disjoint sets

**Data Structures**:

**UnionFind Class**:
- [`mParent`](Source/Falcor/Utils/Algorithm/UnionFind.h:89) - Parent array (each element points to its parent)
- [`mSetSize`](Source/Falcor/Utils/Algorithm/UnionFind.h:90) - Size of each set (only valid for roots)
- [`mSetCount`](Source/Falcor/Utils/Algorithm/UnionFind.h:91) - Number of disjoint sets

**Technical Details**:

**Algorithm**:
- Disjoint-set data structure
- Path compression: flattens the tree structure during find operations
- Union by size: always attaches smaller tree under larger tree
- Near-constant time complexity: α(n) (inverse Ackermann function)

**Path Compression**:
- During findSet, relink nodes directly to root
- Shortens future find operations
- Improves amortized performance

**Union by Size**:
- Always attach smaller set under larger set
- Balances tree depth
- Reduces find operation cost

**Time Complexity**:
- findSet: O(α(n)) amortized
- unionSet: O(α(n)) amortized
- connectedSets: O(α(n)) amortized
- Where α(n) is the inverse Ackermann function (practically constant)

**Use Cases**:
- Connected component detection
- Kruskal's algorithm for MST
- Image segmentation
- Mesh partitioning
- Geometry processing

## Architecture Patterns

### GPU Algorithm Pattern
- Shader-based implementation
- Compute state management
- Program variable binding
- Efficient GPU parallelization
- Minimal CPU-GPU synchronization

### CPU Data Structure Pattern
- Template-based design
- Efficient memory management
- Hash-based lookup
- Optimized algorithms
- Minimal dependencies

### Template Pattern
- Type-safe interfaces
- Compile-time optimization
- Generic algorithms
- Zero-overhead abstractions

## Technical Details

### GPU Algorithm Implementation

**Bitonic Sort**:
- Compute shader implementation
- Group shared memory
- Warp-level operations
- Efficient for short sequences

**Parallel Reduction**:
- Recursive block reduction
- Ping-pong buffers
- Two-phase approach
- Efficient memory access

**Prefix Sum**:
- Exclusive scan algorithm
- Group-level computation
- Global finalize pass
- In-place operation

### CPU Data Structure Implementation

**Directed Graph**:
- Hash-based storage
- Bidirectional edge tracking
- Automatic cleanup
- Efficient lookup

**Union Find**:
- Path compression
- Union by size
- Near-constant time operations
- Template-based design

## Progress Log

- **2026-01-07T19:08:27Z**: Algorithm sub-module analysis completed. Analyzed BitonicSort, ParallelReduction, PrefixSum, DirectedGraph, and UnionFind classes. Documented GPU-based algorithms (bitonic sort, parallel reduction, prefix sum) and CPU-based data structures (directed graph, union-find). Created comprehensive technical specification with detailed code patterns, data structures, and use cases.

## Next Steps

Proceed to analyze Color sub-module to understand color and spectrum utilities.
