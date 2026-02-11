# SlangUtils - Slang Shader Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- None (no external dependencies)

### Dependent Modules

- Falcor/Shaders (Shader utilities)
- Falcor/Rendering (Rendering utilities)
- Falcor/Utils (Internal utilities)

## Module Overview

SlangUtils provides utility templates for Slang shaders. It includes a template struct for ensuring arrays are at least size 1, preventing zero-sized arrays.

### Key Characteristics

- **Template Struct**: Template struct for array size constraints
- **Compile-Time Constant**: Compile-time constant value
- **Zero-Sized Array Prevention**: Prevents zero-sized arrays
- **Minimal Implementation**: Simple implementation with no overhead
- **Slang Integration**: Designed for Slang shader language

### Architecture Principles

1. **Template Pattern**: Template struct for type and size parameters
2. **Compile-Time Computation**: Compile-time constant computation
3. **Zero-Sized Array Prevention**: Prevents zero-sized arrays
4. **Minimal Implementation**: Simple implementation with no overhead
5. **Slang Integration**: Designed for Slang shader language

## Component Specifications

### ArrayMax Struct

**File**: `SlangUtils.slang` (lines 33-36)

```slang
/**
 * Template Max, used to ensure we do not have zero sized arrays.
 * The usage pattern is `float name[ArrayMax<SIZE, 1>.value]` and the array will always be at least 1
 */
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

**Purpose**: Template Max, used to ensure we do not have zero sized arrays

**Template Parameters**:
- **M**: Minimum size (int type)
- **N**: Maximum size (int type)

**Type Definitions**:
```slang
let M : int
let N : int
```

**M**: Minimum size (int type)

**N**: Maximum size (int type)

**Static Constant**:
```slang
static const int value = (M < N) ? N : M;
```

**value**: Compile-time constant value

**Behavior**:
1. If M < N, value = N
2. Otherwise, value = M
3. Ensures value is at least M

**Characteristics**:
- Compile-time constant
- Template struct
- Type-safe
- No runtime overhead

## Technical Details

### Template Pattern

Template struct for type and size parameters:

```slang
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

**Characteristics**:
- Template parameters: M and N
- Type-safe
- Compile-time constant
- No runtime overhead

### Compile-Time Computation

Compile-time constant computation:

```slang
static const int value = (M < N) ? N : M;
```

**Characteristics**:
- Compile-time constant
- Ternary operator
- No runtime computation
- Compiler optimization

### Zero-Sized Array Prevention

Prevents zero-sized arrays:

```slang
/**
 * Template Max, used to ensure we do not have zero sized arrays.
 * The usage pattern is `float name[ArrayMax<SIZE, 1>.value]` and the array will always be at least 1
 */
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

**Characteristics**:
- Ensures array is at least size 1
- Prevents zero-sized arrays
- Compile-time guarantee
- No runtime overhead

### Usage Pattern

Usage pattern for array size constraints:

```slang
float name[ArrayMax<SIZE, 1>.value];
```

**Characteristics**:
- Array size is at least 1
- Compile-time constant
- Type-safe
- No runtime overhead

## Integration Points

### Falcor Shaders Integration

- **Falcor/Shaders**: Shader utilities

### Falcor Rendering Integration

- **Falcor/Rendering**: Rendering utilities

### Falcor Utils Integration

- **Falcor/Utils**: Internal utilities

## Architecture Patterns

### Template Pattern

Template struct for type and size parameters:

```slang
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

Benefits:
- Type-safe
- Compile-time constant
- No runtime overhead
- Flexible parameters

### Compile-Time Computation Pattern

Compile-time constant computation:

```slang
static const int value = (M < N) ? N : M;
```

Benefits:
- Compile-time constant
- No runtime computation
- Compiler optimization
- Zero runtime overhead

### Zero-Sized Array Prevention Pattern

Prevents zero-sized arrays:

```slang
/**
 * Template Max, used to ensure we do not have zero sized arrays.
 * The usage pattern is `float name[ArrayMax<SIZE, 1>.value]` and the array will always be at least 1
 */
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

Benefits:
- Ensures array is at least size 1
- Prevents zero-sized arrays
- Compile-time guarantee
- No runtime overhead

## Code Patterns

### Template Struct Pattern

Template struct for type and size parameters:

```slang
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

Pattern:
- Define template struct
- Template parameters: M and N
- Static constant value
- Ternary operator for computation

### Compile-Time Constant Pattern

Compile-time constant computation:

```slang
static const int value = (M < N) ? N : M;
```

Pattern:
- Define static constant
- Use ternary operator
- Compile-time computation
- No runtime overhead

### Zero-Sized Array Prevention Pattern

Prevents zero-sized arrays:

```slang
/**
 * Template Max, used to ensure we do not have zero sized arrays.
 * The usage pattern is `float name[ArrayMax<SIZE, 1>.value]` and the array will always be at least 1
 */
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

Pattern:
- Define template struct
- Static constant value
- Usage pattern: ArrayMax<SIZE, 1>.value
- Ensures array is at least size 1

## Use Cases

### Basic Array Size Constraint

```slang
// In Slang shader
float values[ArrayMax<10, 1>.value];
// Array size is at least 1, at most 10
```

### Minimum Array Size

```slang
// In Slang shader
float values[ArrayMax<SIZE, 1>.value];
// Array size is at least 1
```

### Maximum Array Size

```slang
// In Slang shader
float values[ArrayMax<SIZE, 100>.value];
// Array size is at most 100
```

### Range Constraint

```slang
// In Slang shader
float values[ArrayMax<10, 5>.value];
// Array size is at least 5, at most 10
```

### Dynamic Array Size

```slang
// In Slang shader
float values[ArrayMax<MAX_SIZE, 1>.value];
// Array size is at least 1, at most MAX_SIZE
```

### Zero-Sized Array Prevention

```slang
// In Slang shader
float values[ArrayMax<0, 1>.value];
// Array size is 1 (prevents zero-sized array)
```

## Performance Considerations

### Compilation Overhead

- **Template Instantiation**: Template instantiation overhead (minimal)
- **Compile-Time Computation**: Compile-time computation (no runtime overhead)
- **Static Constant**: Static constant (no runtime overhead)

### Runtime Overhead

- **No Runtime Overhead**: No runtime overhead
- **Compile-Time Only**: Compile-time only
- **Zero Runtime Cost**: Zero runtime cost

### Compiler Optimizations

- **Compile-Time Constant**: Compile-time constant can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead (minimal)
- **Constant Propagation**: Constants can be propagated

### Comparison with Alternatives

**ArrayMax vs std::max**:
- ArrayMax: Compile-time constant, Slang-specific
- std::max: Runtime function, C++ only

**ArrayMax vs Manual Constant**:
- ArrayMax: Flexible, type-safe
- Manual Constant: Inflexible, error-prone

### Optimization Tips

1. **Use Compile-Time Constants**: Use compile-time constants for better performance
2. **Use Template Parameters**: Use template parameters for flexibility
3. **Use Static Constants**: Use static constants for compile-time computation
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

## Limitations

### Feature Limitations

- **No Runtime Computation**: No runtime computation support
- **No Dynamic Values**: No dynamic value support
- **No Validation**: No validation of template parameters
- **No Error Handling**: No error handling
- **No Fallback**: No fallback for invalid parameters

### API Limitations

- **No Runtime Computation**: No runtime computation support
- **No Dynamic Values**: No dynamic value support
- **No Validation**: No validation of template parameters
- **No Error Handling**: No error handling
- **No Fallback**: No fallback for invalid parameters

### Type Limitations

- **Int Type Only**: Only int type supported
- **No Floating Point**: No floating point support
- **No Unsigned**: No unsigned support

### Performance Limitations

- **Template Instantiation**: Template instantiation overhead (minimal)
- **Compile-Time Only**: Compile-time only (no runtime flexibility)

### Platform Limitations

- **Slang Only**: Slang shader language only
- **No C++**: No C++ support

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Array size constraints in Slang shaders
- Zero-sized array prevention
- Compile-time constant computation
- Type-safe array size constraints

**Inappropriate Use Cases**:
- Runtime array size (use dynamic arrays)
- Floating point array size (use floating point types)
- Unsigned array size (use unsigned types)
- C++ code (use C++ std::max)

### Usage Patterns

**Basic Array Size Constraint**:
```slang
float values[ArrayMax<10, 1>.value];
```

**Minimum Array Size**:
```slang
float values[ArrayMax<SIZE, 1>.value];
```

**Maximum Array Size**:
```slang
float values[ArrayMax<SIZE, 100>.value];
```

**Range Constraint**:
```slang
float values[ArrayMax<10, 5>.value];
```

### Performance Tips

1. **Use Compile-Time Constants**: Use compile-time constants for better performance
2. **Use Template Parameters**: Use template parameters for flexibility
3. **Use Static Constants**: Use static constants for compile-time computation
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

### Type Safety

- **Type-Safe**: Type-safe template parameters
- **Compile-Time Checking**: Compile-time type checking
- **No Runtime Type Errors**: No runtime type errors

### Error Handling

- **No Error Handling**: No error handling
- **Compile-Time Errors**: Compile-time errors only
- **No Runtime Errors**: No runtime errors

## Implementation Notes

### Template Struct Implementation

Template struct for type and size parameters:

```slang
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

**Characteristics**:
- Template parameters: M and N
- Type-safe
- Compile-time constant
- No runtime overhead

### Compile-Time Computation Implementation

Compile-time constant computation:

```slang
static const int value = (M < N) ? N : M;
```

**Characteristics**:
- Compile-time constant
- Ternary operator
- No runtime computation
- Compiler optimization

### Zero-Sized Array Prevention Implementation

Prevents zero-sized arrays:

```slang
/**
 * Template Max, used to ensure we do not have zero sized arrays.
 * The usage pattern is `float name[ArrayMax<SIZE, 1>.value]` and the array will always be at least 1
 */
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
};
```

**Characteristics**:
- Ensures array is at least size 1
- Prevents zero-sized arrays
- Compile-time guarantee
- No runtime overhead

## Future Enhancements

### Potential Improvements

1. **More Types**: Add support for more types (float, unsigned, etc.)
2. **Runtime Computation**: Add runtime computation support
3. **Validation**: Add validation of template parameters
4. **Error Handling**: Add error handling
5. **Fallback**: Add fallback for invalid parameters
6. **More Constraints**: Add more constraint templates (Min, Clamp, etc.)

### API Extensions

1. **More Types**: Add support for more types
2. **Runtime Computation**: Add runtime computation support
3. **Validation**: Add validation of template parameters
4. **Error Handling**: Add error handling
5. **Fallback**: Add fallback for invalid parameters
6. **More Constraints**: Add more constraint templates (Min, Clamp, etc.)

### Performance Enhancements

1. **Compiler Optimizations**: Better compiler optimizations
2. **Template Instantiation**: Better template instantiation
3. **Constant Propagation**: Better constant propagation

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from manual constants
5. **API Reference**: Add complete API reference

## References

### Slang Documentation

- **Slang Language**: Slang shader language documentation
- **Slang Templates**: Slang template documentation

### Falcor Documentation

- **Falcor Shaders**: Shaders module documentation
- **Falcor Rendering**: Rendering module documentation

### Related Technologies

- **Template Programming**: Template programming techniques
- **Compile-Time Computation**: Compile-time computation techniques
- **Shader Programming**: Shader programming techniques
- **Slang**: Slang shader language
