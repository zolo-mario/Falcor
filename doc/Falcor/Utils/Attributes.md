# Attributes - Shader Attribute Tagging

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- None (standalone Slang shader utility)

## Module Overview

Attributes is a Slang shader utility structure for tagging resources that should be bound to a root descriptor. It provides a mechanism to mark shader variables with specific attribute usage targets, enabling efficient resource binding and descriptor management in GPU shaders.

## Component Specifications

### rootAttribute Struct

**Signature**: `[__AttributeUsage(_AttributeTargets.Var)] struct rootAttribute {}`

**Purpose**: Attribute structure for tagging resources that should be bound to a root descriptor

**Attributes**:
- `__AttributeUsage(_AttributeTargets.Var)`: Slang attribute marking this structure for variable target usage
- Empty struct body: No members, pure marker structure

**Usage**:
```slang
[RootSignature]
rootAttribute gMyAttribute;
```

**Integration**:
- Can be applied to global variables
- Can be applied to struct members
- Can be applied to array elements
- Compiler uses attribute for descriptor layout generation

## Technical Details

### Slang Attribute System

**AttributeUsage Macro**:
- Slang built-in macro for attribute marking
- Specifies target usage (Var, Texture, UAV, etc.)
- Used by compiler for descriptor set layout

**AttributeTargets Enum**:
- `Var`: Variable/register usage
- Other targets: Texture, UAV, Sampler, etc.
- Determines binding point type in descriptor set

**Root Descriptor Binding**:
- Resources marked with rootAttribute are bound to root signature
- Enables efficient descriptor table access
- Reduces descriptor set count and switches

### Shader Integration

**Global Variable Tagging**:
```slang
[RootSignature]
rootAttribute gGlobalBuffer;
StructuredBuffer<float> gGlobalBuffer : register(t0);
```

**Struct Member Tagging**:
```slang
[RootSignature]
struct MyData
{
    rootAttribute gMember1;
    rootAttribute gMember2;
};
```

**Array Element Tagging**:
```slang
[RootSignature]
StructuredBuffer<float> gBuffer : register(t0);
rootAttribute gBufferElements[1024];
```

### Descriptor Layout Generation

**Compiler Behavior**:
1. Parses attribute annotations
2. Groups root-marked resources
3. Generates root signature
4. Optimizes descriptor table layout
5. Reduces descriptor set switches

**Runtime Behavior**:
- No runtime overhead
- Compile-time only
- Affects GPU pipeline state setup
- Enables efficient resource access

## Integration Points

### Usage in Falcor Shaders

The rootAttribute structure is used throughout Falcor's Slang shaders:

1. **Constant Buffers**: Tagging global constant buffers
2. **Structured Buffers**: Tagging structured buffers for root binding
3. **Resource Arrays**: Tagging array elements for root descriptor access
4. **Descriptor Optimization**: Reducing descriptor set switches

### Integration Pattern

```slang
[RootSignature]
rootAttribute gMyBuffer;
StructuredBuffer<float> gMyBuffer : register(t0);

// In shader
float value = gMyBuffer[index];
```

## Architecture Patterns

### Attribute Pattern

Slang attribute system for resource tagging:
- Compile-time metadata
- Zero runtime overhead
- Type-safe usage
- Compiler-driven optimization

### Marker Pattern

Empty struct as marker:
- No data members
- Pure tagging mechanism
- Minimal memory footprint
- Efficient compilation

### Descriptor Pattern

Root descriptor optimization:
- Groups resources for root signature
- Reduces descriptor set count
- Minimizes descriptor switches
- Improves GPU efficiency

## Code Patterns

### Global Variable Tagging Pattern

```slang
[RootSignature]
rootAttribute gMyBuffer;
StructuredBuffer<float> gMyBuffer : register(t0);
```

### Struct Member Tagging Pattern

```slang
[RootSignature]
struct MyData
{
    rootAttribute gMember1;
    rootAttribute gMember2;
};
```

### Array Element Tagging Pattern

```slang
[RootSignature]
StructuredBuffer<float> gBuffer : register(t0);
rootAttribute gBufferElements[1024];
```

## Use Cases

### Root Descriptor Binding

Primary use case is marking resources for root descriptor:
- Global constant buffers
- Structured buffers accessed frequently
- Resource arrays with many elements
- Performance-critical shader resources

### Descriptor Optimization

Reducing descriptor set switches:
- Minimizes GPU state changes
- Improves rendering performance
- Reduces CPU overhead
- Enables efficient resource access

### Resource Grouping

Grouping related resources:
- Frequently accessed buffers
- Global shader data
- Performance-critical resources
- Large resource arrays

## Performance Considerations

### Compilation Performance

- **Zero Runtime Overhead**: Pure compile-time attribute
- **Fast Compilation**: Simple structure, no complex logic
- **Minimal Code Generation**: No additional instructions generated
- **Type Checking**: Compile-time type validation

### GPU Performance

- **Descriptor Optimization**: Reduces descriptor set switches
- **Root Signature Efficiency**: Groups resources for optimal access
- **Binding Point Reduction**: Fewer binding points to manage
- **Cache Efficiency**: Better descriptor cache utilization

### Memory Efficiency

- **Zero Runtime Memory**: No runtime storage
- **Zero CPU Overhead**: No runtime processing
- **Minimal Binary Size**: Empty structure adds no code
- **Compile-Time Only**: All processing at compilation time

## Limitations

### Functional Limitations

- **Shader-Only Only**: Cannot be used from C++ code
- **Slang Specific**: Only works with Slang compiler
- **No Runtime Control**: Cannot be toggled at runtime
- **No Data Storage**: Cannot store any data

### Attribute Limitations

- **Single Target**: Only supports _AttributeTargets.Var
- **No Multiple Attributes**: Cannot apply multiple attributes
- **No Custom Targets**: Limited to Slang's predefined targets
- **No Runtime Configuration**: Cannot be configured dynamically

### Integration Limitations

- **No C++ Integration**: Cannot be used from host code
- **No Reflection**: Cannot query attributes at runtime
- **No Dynamic Binding**: Cannot change binding at runtime
- **No Validation**: No runtime validation of attribute usage

## Best Practices

### When to Use rootAttribute

1. **Frequently Accessed Resources**: Mark resources accessed every frame
2. **Global Shader Data**: Mark global constant buffers
3. **Large Resource Arrays**: Mark array elements accessed in loops
4. **Performance Critical**: Mark performance-critical resources
5. **Root Descriptor Targets**: Use for resources bound to root signature

### When to Avoid rootAttribute

1. **Infrequently Accessed Resources**: Overhead may not be justified
2. **Small Resources**: Overhead may exceed benefit
3. **Per-Frame Resources**: Resources that change every frame
4. **Descriptor Set Resources**: Resources in regular descriptor sets
5. **Non-Performance Critical**: Resources not on critical paths

### Usage Guidelines

1. **Consistent Tagging**: Apply consistently across related resources
2. **Group Related Resources**: Tag resources accessed together
3. **Profile First**: Use profiling to identify candidates
4. **Minimize Usage**: Only tag when performance benefit is clear
5. **Document Usage**: Comment on why attribute is applied

### Optimization Tips

1. **Batch Resources**: Group resources accessed together
2. **Prioritize Critical Path**: Tag resources on critical rendering paths
3. **Profile GPU Performance**: Measure impact of attribute usage
4. **Reduce Descriptor Switches**: Use rootAttribute to minimize switches
5. **Consider Cache Locality**: Tag resources with good cache behavior

## Notes

- Pure Slang shader utility structure
- Empty struct with no data members
- Uses __AttributeUsage(_AttributeTargets.Var) attribute
- Marks resources for root descriptor binding
- Zero runtime overhead
- Compile-time only optimization
- Enables efficient descriptor table access
- Reduces descriptor set count and switches
- Type-safe through Slang's attribute system
- No C++ integration possible
- No runtime configuration or validation
- Best for frequently accessed global resources
