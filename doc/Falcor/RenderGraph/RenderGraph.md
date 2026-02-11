# RenderGraph - Render Graph System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Internal Components (All Complete)

- [x] **RenderGraph** - Main render graph container and DAG management
- [x] **RenderPass** - Base class for render pass implementations
- [x] **RenderPassReflection** - Reflection system for pass I/O requirements
- [x] **RenderGraphCompiler** - Graph compilation and optimization
- [x] **RenderGraphExe** - Graph execution runtime
- [x] **ResourceCache** - Resource allocation and lifetime management
- [x] **RenderGraphIR** - Intermediate representation for graph serialization
- [x] **RenderGraphUI** - Graph visualization and editing UI
- [x] **RenderGraphImportExport** - Graph serialization/deserialization
- [x] **RenderPassHelpers** - Utility functions for render passes
- [x] **RenderPassStandardFlags** - Standard render pass flags

### External Dependencies

- **Core/Object** - Base object class
- **Core/API** - Graphics API (Device, RenderContext, Resource, Texture, Formats)
- **Scene/Scene** - Scene integration
- **Utils/Algorithm/DirectedGraph** - DAG data structure
- **Utils/Properties** - Property system
- **Utils/Dictionary** - Dictionary for inter-pass communication
- **Utils/UI/Gui** - UI rendering
- **Utils/Math/Vector** - Vector types (uint2)
- **Core/Plugin** - Plugin system for render pass registration

## Module Overview

The RenderGraph module implements a directed acyclic graph (DAG) based rendering pipeline system. It provides a flexible, data-driven architecture for composing complex rendering effects from modular render passes. The system supports automatic resource allocation, execution ordering, optimization, and serialization.

## Component Specifications

### RenderGraph

**File**: [`RenderGraph.h`](Source/Falcor/RenderGraph/RenderGraph.h:1)

**Purpose**: Main container for render graph management, providing DAG structure, pass management, and graph execution.

**Key Features**:
- DAG-based render pass organization
- Automatic execution order resolution
- Resource allocation and lifetime management
- Graph compilation and optimization
- Python script loading and execution
- Scene integration
- Input/output marking and management
- Hot reload support

**Core Methods**:
- [`create()`](Source/Falcor/RenderGraph/RenderGraph.h:68) - Factory method for graph creation
- [`createFromFile()`](Source/Falcor/RenderGraph/RenderGraph.h:76) - Load graph from Python script
- [`createFromString()`](Source/Falcor/RenderGraph/RenderGraph.h:77) - Load graph from string
- [`createPass()`](Source/Falcor/RenderGraph/RenderGraph.h:97) - Create and add new render pass
- [`addPass()`](Source/Falcor/RenderGraph/RenderGraph.h:103) - Add existing render pass
- [`removePass()`](Source/Falcor/RenderGraph/RenderGraph.h:114) - Remove render pass and edges
- [`updatePass()`](Source/Falcor/RenderGraph/RenderGraph.h:119) - Update pass properties
- [`addEdge()`](Source/Falcor/RenderGraph/RenderGraph.h:136) - Add data or execution dependency edge
- [`removeEdge()`](Source/Falcor/RenderGraph/RenderGraph.h:141) - Remove edge by name
- [`execute()`](Source/Falcor/RenderGraph/RenderGraph.h:151) - Execute compiled graph
- [`compile()`](Source/Falcor/RenderGraph/RenderGraph.h:307) - Compile graph for execution
- [`setScene()`](Source/Falcor/RenderGraph/RenderGraph.h:88) - Set scene for all passes
- [`setInput()`](Source/Falcor/RenderGraph/RenderGraph.h:163) - Set external input resource
- [`markOutput()`](Source/Falcor/RenderGraph/RenderGraph.h:199) - Mark pass output as graph output
- [`unmarkOutput()`](Source/Falcor/RenderGraph/RenderGraph.h:207) - Unmark graph output
- [`onResize()`](Source/Falcor/RenderGraph/RenderGraph.h:218) - Handle swapchain resize
- [`onSceneUpdates()`](Source/Falcor/RenderGraph/RenderGraph.h:269) - Propagate scene updates to passes
- [`onHotReload()`](Source/Falcor/RenderGraph/RenderGraph.h:287) - Handle shader hot reload

**Edge Types**:
1. **Data Dependency Edge**: Connects pass output to another pass input
   - Format: `srcPass.resourceName` → `dstPass.resourceName`
   - Can be optimized out if not influencing requested output

2. **Execution Dependency Edge**: Creates execution order without data flow
   - Format: `srcPass` → `dstPass`
   - Never optimized, always executes

**Data Structures**:
```cpp
struct EdgeData {
    std::string srcField;
    std::string dstField;
};

struct NodeData {
    std::string name;
    ref<RenderPass> pPass;
};

struct GraphOut {
    uint32_t nodeId = kInvalidIndex;
    std::string field;
    std::unordered_set<TextureChannelFlags> masks;
};
```

**Graph Compilation Flow**:
1. Validate graph structure (DAG check)
2. Resolve execution order via topological sort
3. Call [`reflect()`](Source/Falcor/RenderGraph/RenderPass.h:169) on all passes
4. Insert auto-generated passes (e.g., format conversion)
5. Compile passes via [`compile()`](Source/Falcor/RenderGraph/RenderPass.h:174)
6. Allocate resources via [`ResourceCache`](Source/Falcor/RenderGraph/ResourceCache.h:40)
7. Generate execution list

### RenderPass

**File**: [`RenderPass.h`](Source/Falcor/RenderGraph/RenderPass.h:1)

**Purpose**: Abstract base class for all render passes, defining the interface for pass implementation.

**Key Features**:
- Plugin-based registration and creation
- Reflection-based I/O declaration
- Compile-time resource allocation
- Runtime execution
- Scene integration
- UI rendering
- Hot reload support
- Inter-pass communication via dictionary

**Core Virtual Methods**:
- [`reflect()`](Source/Falcor/RenderGraph/RenderPass.h:169) - Declare I/O requirements (must implement)
- [`compile()`](Source/Falcor/RenderGraph/RenderPass.h:174) - Compile pass during graph compilation
- [`execute()`](Source/Falcor/RenderGraph/RenderPass.h:179) - Execute pass (must implement)
- [`setProperties()`](Source/Falcor/RenderGraph/RenderPass.h:184) - Set pass properties
- [`getProperties()`](Source/Falcor/RenderGraph/RenderPass.h:189) - Get pass properties
- [`renderUI()`](Source/Falcor/RenderGraph/RenderPass.h:200) - Render pass UI
- [`renderOverlayUI()`](Source/Falcor/RenderGraph/RenderPass.h:205) - Render overlay UI
- [`setScene()`](Source/Falcor/RenderGraph/RenderPass.h:213) - Set scene for pass
- [`onSceneUpdates()`](Source/Falcor/RenderGraph/RenderPass.h:222) - Handle scene updates
- [`onMouseEvent()`](Source/Falcor/RenderGraph/RenderPass.h:228) - Handle mouse events
- [`onKeyEvent()`](Source/Falcor/RenderGraph/RenderPass.h:234) - Handle keyboard events
- [`onHotReload()`](Source/Falcor/RenderGraph/RenderPass.h:240) - Handle shader hot reload

**Plugin System**:
```cpp
using PluginCreate = std::function<ref<RenderPass>(ref<Device> pDevice, const Properties& props)>;

struct PluginInfo {
    std::string desc;
};

static ref<RenderPass> create(
    std::string_view type,
    ref<Device> pDevice,
    const Properties& props = {},
    PluginManager& pm = PluginManager::instance()
);
```

**CompileData Structure**:
```cpp
struct CompileData {
    uint2 defaultTexDims;              // Default texture dimensions (swapchain size)
    ResourceFormat defaultTexFormat;    // Default texture format (swapchain format)
    RenderPassReflection connectedResources; // Reflection for connected resources
};
```

**RenderData Helper**:
Passed to [`execute()`](Source/Falcor/RenderGraph/RenderPass.h:179) to provide access to resources:
- [`operator[]`](Source/Falcor/RenderGraph/RenderPass.h:59) - Get resource by name
- [`getResource()`](Source/Falcor/RenderGraph/RenderPass.h:66) - Get resource by name
- [`getTexture()`](Source/Falcor/RenderGraph/RenderPass.h:73) - Get texture by name
- [`getDictionary()`](Source/Falcor/RenderGraph/RenderPass.h:78) - Get inter-pass dictionary
- [`getDefaultTextureDims()`](Source/Falcor/RenderGraph/RenderPass.h:83) - Get default texture dimensions
- [`getDefaultTextureFormat()`](Source/Falcor/RenderGraph/RenderPass.h:88) - Get default texture format

### RenderPassReflection

**File**: [`RenderPassReflection.h`](Source/Falcor/RenderGraph/RenderPassReflection.h:1)

**Purpose**: Reflection system for declaring render pass I/O requirements and resource specifications.

**Key Features**:
- Type-safe resource declaration
- Input/output/internal field classification
- Resource format and dimension specification
- Bind flags and visibility control
- Optional and persistent resource marking
- Field merging for aliasing

**Field Class**:
```cpp
class Field {
public:
    enum class Visibility {
        Undefined = 0x0,
        Input = 0x1,      // Input field
        Output = 0x2,     // Output field
        Internal = 0x4,    // Internal field
    };

    enum class Flags {
        None = 0x0,
        Optional = 0x1,      // Field is optional
        Persistent = 0x2,    // Resource doesn't change between execute() calls
    };

    enum class Type {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
        RawBuffer,
    };
};
```

**Field Configuration Methods**:
- [`rawBuffer()`](Source/Falcor/RenderGraph/RenderPassReflection.h:92) - Configure as raw buffer
- [`texture1D()`](Source/Falcor/RenderGraph/RenderPassReflection.h:93) - Configure as 1D texture
- [`texture2D()`](Source/Falcor/RenderGraph/RenderPassReflection.h:94) - Configure as 2D texture
- [`texture3D()`](Source/Falcor/RenderGraph/RenderPassReflection.h:95) - Configure as 3D texture
- [`textureCube()`](Source/Falcor/RenderGraph/RenderPassReflection.h:96) - Configure as cube texture
- [`format()`](Source/Falcor/RenderGraph/RenderPassReflection.h:107) - Set resource format
- [`bindFlags()`](Source/Falcor/RenderGraph/RenderPassReflection.h:108) - Set bind flags
- [`flags()`](Source/Falcor/RenderGraph/RenderPassReflection.h:109) - Set field flags
- [`visibility()`](Source/Falcor/RenderGraph/RenderPassReflection.h:110) - Set visibility
- [`name()`](Source/Falcor/RenderGraph/RenderPassReflection.h:111) - Set field name
- [`desc()`](Source/Falcor/RenderGraph/RenderPassReflection.h:112) - Set field description

**RenderPassReflection Methods**:
- [`addInput()`](Source/Falcor/RenderGraph/RenderPassReflection.h:162) - Add input field
- [`addOutput()`](Source/Falcor/RenderGraph/RenderPassReflection.h:163) - Add output field
- [`addInputOutput()`](Source/Falcor/RenderGraph/RenderPassReflection.h:164) - Add input/output field
- [`addInternal()`](Source/Falcor/RenderGraph/RenderPassReflection.h:165) - Add internal field
- [`getField()`](Source/Falcor/RenderGraph/RenderPassReflection.h:169) - Get field by name
- [`addField()`](Source/Falcor/RenderGraph/RenderPassReflection.h:171) - Add field from Field object

**Default Bind Flags**:
- Output resources: `ResourceBindFlags::RenderTarget`
- Input resources: `ResourceBindFlags::ShaderResource`
- InOut resources: `ResourceBindFlags::RenderTarget | ShaderResource`

### ResourceCache

**File**: [`ResourceCache.h`](Source/Falcor/RenderGraph/ResourceCache.h:1)

**Purpose**: Manages resource allocation, lifetime, and aliasing for render graph resources.

**Key Features**:
- Automatic resource allocation based on reflection
- Resource lifetime tracking
- External resource registration
- Resource aliasing for memory optimization
- Default property fallback

**Core Methods**:
- [`registerExternalResource()`](Source/Falcor/RenderGraph/ResourceCache.h:59) - Register external resource
- [`registerField()`](Source/Falcor/RenderGraph/ResourceCache.h:69) - Register field requiring allocation
- [`getResource()`](Source/Falcor/RenderGraph/ResourceCache.h:79) - Get resource by name
- [`getResourceReflection()`](Source/Falcor/RenderGraph/ResourceCache.h:84) - Get field reflection
- [`allocateResources()`](Source/Falcor/RenderGraph/ResourceCache.h:90) - Allocate all required resources
- [`reset()`](Source/Falcor/RenderGraph/ResourceCache.h:95) - Clear all registered resources

**DefaultProperties Structure**:
```cpp
struct DefaultProperties {
    uint2 dims;                              // Width, height of swapchain
    ResourceFormat format = ResourceFormat::Unknown; // Default texture format
};
```

**ResourceData Structure**:
```cpp
struct ResourceData {
    RenderPassReflection::Field field;      // Merged properties for aliased resources
    std::pair<uint32_t, uint32_t> lifetime; // Time range where resource is used
    ref<Resource> pResource;                // The resource
    bool resolveBindFlags;                  // Whether to resolve bind flags before creation
    std::string name;                       // Full resource name including pass name
};
```

**Resource Allocation Strategy**:
1. Register all fields with their requirements
2. Track resource lifetime via time points
3. Merge properties for aliased resources
4. Allocate resources with appropriate dimensions and formats
5. Apply default properties for unspecified values

### RenderGraphCompiler

**File**: [`RenderGraphCompiler.h`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:1)

**Purpose**: Compiles render graph into executable form with optimized execution order and resource allocation.

**Key Features**:
- Topological sort for execution order
- Graph validation (DAG check)
- Auto-pass insertion (format conversion, etc.)
- Resource allocation coordination
- Compilation change tracking and restoration

**Core Methods**:
- [`compile()`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:50) - Static factory for compilation
- [`resolveExecutionOrder()`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:75) - Determine pass execution order
- [`compilePasses()`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:76) - Compile individual passes
- [`insertAutoPasses()`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:77) - Insert auto-generated passes
- [`allocateResources()`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:78) - Allocate graph resources
- [`validateGraph()`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:79) - Validate graph structure
- [`restoreCompilationChanges()`](Source/Falcor/RenderGraph/RenderGraphCompiler.h:80) - Restore graph after compilation

**Dependencies Structure**:
```cpp
struct Dependencies {
    ResourceCache::DefaultProperties defaultResourceProps;
    ResourceCache::ResourcesMap externalResources;
};
```

**PassData Structure**:
```cpp
struct PassData {
    uint32_t index;
    ref<RenderPass> pPass;
    std::string name;
    RenderPassReflection reflector;
};
```

**Compilation Flow**:
1. Validate graph is a DAG
2. Resolve execution order via topological sort
3. Call [`reflect()`](Source/Falcor/RenderGraph/RenderPass.h:169) on all passes
4. Insert auto-passes for format conversions
5. Call [`compile()`](Source/Falcor/RenderGraph/RenderPass.h:174) on all passes
6. Allocate resources via [`ResourceCache`](Source/Falcor/RenderGraph/ResourceCache.h:40)
7. Generate [`RenderGraphExe`](Source/Falcor/RenderGraph/RenderGraphExe.h:46) instance
8. Track compilation changes for potential rollback

### RenderGraphExe

**File**: [`RenderGraphExe.h`](Source/Falcor/RenderGraph/RenderGraphExe.h:1)

**Purpose**: Runtime execution engine for compiled render graphs.

**Key Features**:
- Pass execution in compiled order
- Resource management via cache
- UI rendering for all passes
- Event handling (mouse, keyboard, hot reload)
- External input resource binding

**Core Methods**:
- [`execute()`](Source/Falcor/RenderGraph/RenderGraphExe.h:60) - Execute all passes
- [`renderUI()`](Source/Falcor/RenderGraph/RenderGraphExe.h:65) - Render pass UI
- [`renderOverlayUI()`](Source/Falcor/RenderGraph/RenderGraphExe.h:70) - Render overlay UI
- [`onMouseEvent()`](Source/Falcor/RenderGraph/RenderGraphExe.h:76) - Handle mouse events
- [`onKeyEvent()`](Source/Falcor/RenderGraph/RenderGraphExe.h:82) - Handle keyboard events
- [`onHotReload()`](Source/Falcor/RenderGraph/RenderGraphExe.h:88) - Handle hot reload
- [`getResource()`](Source/Falcor/RenderGraph/RenderGraphExe.h:93) - Get resource from cache
- [`setInput()`](Source/Falcor/RenderGraph/RenderGraphExe.h:100) - Set external input resource

**Context Structure**:
```cpp
struct Context {
    RenderContext* pRenderContext;
    Dictionary& passesDictionary;
    uint2 defaultTexDims;
    ResourceFormat defaultTexFormat;
};
```

**Execution Flow**:
1. Create [`RenderData`](Source/Falcor/RenderGraph/RenderPass.h:51) for each pass
2. Call [`execute()`](Source/Falcor/RenderGraph/RenderPass.h:179) on each pass in order
3. Provide resources via [`RenderData`](Source/Falcor/RenderGraph/RenderPass.h:51)
4. Handle events through pass callbacks
5. Manage resource lifetime via [`ResourceCache`](Source/Falcor/RenderGraph/ResourceCache.h:40)

### RenderGraphIR

**File**: [`RenderGraphIR.h`](Source/Falcor/RenderGraph/RenderGraphIR.h:1)

**Purpose**: Intermediate representation for render graph serialization and Python code generation.

**Key Features**:
- Python script generation
- Graph structure serialization
- Indentation management
- Function name generation

**Core Methods**:
- [`createPass()`](Source/Falcor/RenderGraph/RenderGraphIR.h:44) - Create pass in IR
- [`updatePass()`](Source/Falcor/RenderGraph/RenderGraphIR.h:45) - Update pass in IR
- [`removePass()`](Source/Falcor/RenderGraph/RenderGraphIR.h:46) - Remove pass from IR
- [`addEdge()`](Source/Falcor/RenderGraph/RenderGraphIR.h:47) - Add edge in IR
- [`removeEdge()`](Source/Falcor/RenderGraph/RenderGraphIR.h:48) - Remove edge from IR
- [`markOutput()`](Source/Falcor/RenderGraph/RenderGraphIR.h:49) - Mark output in IR
- [`unmarkOutput()`](Source/Falcor/RenderGraph/RenderGraphIR.h:50) - Unmark output in IR
- [`getIR()`](Source/Falcor/RenderGraph/RenderGraphIR.h:52) - Get generated Python code
- [`getFuncName()`](Source/Falcor/RenderGraph/RenderGraphIR.h:54) - Get function name for graph

**IR Generation**:
- Builds Python code string
- Manages indentation for readability
- Generates function wrapper for graph
- Returns complete Python script

## Architecture Patterns

### Plugin Pattern
- [`RenderPass::create()`](Source/Falcor/RenderGraph/RenderPass.h:144) uses plugin manager
- Dynamic loading of render pass types
- Registration via [`FALCOR_PLUGIN_BASE_CLASS`](Source/Falcor/RenderGraph/RenderPass.h:129)

### Reflection Pattern
- [`RenderPassReflection`](Source/Falcor/RenderGraph/RenderPassReflection.h:39) declares I/O requirements
- Type-safe resource specification
- Compile-time validation
- Runtime resource allocation

### DAG Pattern
- [`DirectedGraph`](Source/Falcor/Utils/Algorithm/DirectedGraph.h:1) for topology management
- Topological sort for execution order
- Cycle detection for validation
- Dependency tracking

### Factory Pattern
- [`RenderGraph::create()`](Source/Falcor/RenderGraph/RenderGraph.h:68) - Graph creation
- [`RenderPass::create()`](Source/Falcor/RenderGraph/RenderPass.h:144) - Pass creation
- Plugin-based instantiation

### Visitor Pattern
- [`RenderGraphExe`](Source/Falcor/RenderGraph/RenderGraphExe.h:46) visits passes for execution
- Event propagation through pass callbacks
- UI rendering for all passes

## Technical Details

### Graph Compilation Pipeline

The compilation process transforms a declarative graph specification into an executable form:

1. **Validation Phase**
   - Check for cycles (DAG validation)
   - Verify all required inputs are satisfied
   - Validate resource specifications

2. **Reflection Phase**
   - Call [`reflect()`](Source/Falcor/RenderGraph/RenderPass.h:169) on all passes
   - Build complete I/O specification
   - Identify unsatisfied inputs

3. **Auto-Pass Insertion**
   - Insert format conversion passes as needed
   - Add intermediate passes for resource transformations
   - Ensure compatibility between connected passes

4. **Compilation Phase**
   - Call [`compile()`](Source/Falcor/RenderGraph/RenderPass.h:174) on all passes
   - Prepare shader programs
   - Validate pass configurations

5. **Resource Allocation**
   - Analyze resource lifetimes
   - Merge aliased resources
   - Allocate textures and buffers
   - Apply default properties

6. **Execution Order Resolution**
   - Perform topological sort
   - Generate execution list
   - Handle execution dependencies

### Resource Lifetime Management

Resources are allocated with explicit lifetime tracking:

```cpp
struct ResourceData {
    std::pair<uint32_t, uint32_t> lifetime; // [start_time, end_time]
};
```

This enables:
- Memory reuse via aliasing
- Automatic resource deallocation
- Optimization of intermediate resources
- Reduction of memory footprint

### Edge Semantics

**Data Dependency Edges**:
- Format: `srcPass.output` → `dstPass.input`
- Creates data flow between passes
- Can be optimized out if not affecting output
- Drives automatic resource allocation

**Execution Dependency Edges**:
- Format: `srcPass` → `dstPass`
- Enforces execution order without data flow
- Never optimized out
- Useful for side-effect passes

### Inter-Pass Communication

Passes communicate via a shared dictionary:

```cpp
Dictionary& getDictionary() const { return mDictionary; }
```

This allows:
- Global state sharing
- Parameter passing
- Coordination between passes
- Avoiding explicit dependencies

## Performance Considerations

### Compilation Optimization
- Topological sort is O(V + E) where V = vertices, E = edges
- Reflection is cached when graph doesn't change
- Auto-pass insertion minimizes redundant passes
- Resource aliasing reduces memory allocation

### Execution Optimization
- Dead code elimination for unused passes
- Resource reuse via aliasing
- Minimal state changes between passes
- Efficient resource transitions

### Memory Management
- Lifetime tracking enables precise allocation
- Aliasing reduces peak memory usage
- External resources avoid unnecessary copies
- Persistent resources avoid reallocation

## Integration Points

### Scene Integration
- [`setScene()`](Source/Falcor/RenderGraph/RenderGraph.h:88) propagates scene to all passes
- [`onSceneUpdates()`](Source/Falcor/RenderGraph/RenderGraph.h:269) handles scene changes
- Passes can request scene-specific resources
- Scene updates trigger recompilation if needed

### Python Integration
- [`createFromFile()`](Source/Falcor/RenderGraph/RenderGraph.h:76) loads Python scripts
- [`createFromString()`](Source/Falcor/RenderGraph/RenderGraph.h:77) parses Python code
- [`RenderGraphIR`](Source/Falcor/RenderGraph/RenderGraphIR.h:39) generates Python
- Dynamic graph modification via Python

### UI Integration
- [`renderUI()`](Source/Falcor/RenderGraph/RenderGraph.h:257) renders graph editor
- [`renderOverlayUI()`](Source/Falcor/RenderGraph/RenderGraph.h:262) renders overlay
- Pass UI rendered via [`RenderPass::renderUI()`](Source/Falcor/RenderGraph/RenderPass.h:200)
- Interactive graph editing

## Progress Log

- **2026-01-07T18:46:12Z**: RenderGraph module analysis completed. All 11 components analyzed and documented. Module marked as Complete.

## Next Steps

Proceed to analyze Rendering module to understand rendering subsystems (Lights, Materials, RTXDI, Volumes).
