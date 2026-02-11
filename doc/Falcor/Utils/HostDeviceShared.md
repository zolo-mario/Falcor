# HostDeviceShared - Host/Device Shared Code

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Enum
- Falcor/Utils/Math/ScalarMath
- Falcor/Utils/Math/Vector
- Falcor/Utils/Math/Matrix
- STL (algorithm)

### Dependent Modules

- Falcor/Scene (shared scene data structures)
- Falcor/Rendering (shared rendering data structures)
- Falcor/Utils (internal utilities)

## Module Overview

HostDeviceShared provides a shared header for CPU/GPU compilation, enabling the same code to be compiled for both host (CPU) and device (HLSL) contexts. The header defines platform-specific macros, bit manipulation utilities, and conditional declarations that adapt to the compilation target. It's essential for Falcor's hybrid CPU-GPU architecture where data structures and algorithms need to work on both sides.

## Component Specifications

### Platform Detection

**File**: `Source/Falcor/Utils/HostDeviceShared.slangh` (lines 34-36)

#### HOST_CODE Macro

```cpp
#if (defined(__STDC_HOSTED__) || defined(__cplusplus)) // we're in C-compliant compiler, probably host
#define HOST_CODE 1
#endif
```

**Purpose**: Detects if compiling for host (CPU)

**Behavior**:
- Checks for `__STDC_HOSTED__` or `__cplusplus`
- Defines HOST_CODE to 1 if true
- Used to conditionally compile host vs device code

**Detection Logic**:
- `__STDC_HOSTED__`: C standard hosted implementation (CPU)
- `__cplusplus`: C++ compilation (typically CPU)
- If neither defined, assumes device (HLSL) compilation

### Bit Manipulation Macros

**File**: `Source/Falcor/Utils/HostDeviceShared.slangh` (lines 38-44)

#### EXTRACT_BITS Macro

```cpp
#define EXTRACT_BITS(bits, offset, value) (((value) >> (offset)) & ((1 << (bits)) - 1))
```

**Purpose**: Extracts specified number of bits from a value at given offset

**Parameters**:
- bits: Number of bits to extract
- offset: Bit position to start extraction from
- value: Value to extract bits from

**Returns**: Extracted bits as unsigned integer

**Behavior**:
- Right-shifts value by offset positions
- Masks with ((1 << bits) - 1) to extract only specified bits
- Example: EXTRACT_BITS(4, 8, 0x12345678) extracts bits 8-11

**Usage**:
```cpp
uint32_t value = 0x12345678;
uint32_t nibble = EXTRACT_BITS(4, 8, value);  // Extract 4 bits starting at bit 8
```

#### IS_BIT_SET Macro

```cpp
#define IS_BIT_SET(value, bitOffset) ((value & (1 << bitOffset)) != 0)
```

**Purpose**: Checks if a specific bit is set in a value

**Parameters**:
- value: Value to check
- bitOffset: Bit position to check (0-based)

**Returns**: Non-zero (true) if bit is set, zero (false) otherwise

**Behavior**:
- Creates mask with (1 << bitOffset)
- ANDs with value
- Checks if result is non-zero

**Usage**:
```cpp
uint32_t flags = 0b10101010;
bool bit3 = IS_BIT_SET(flags, 3);  // Check if bit 3 is set
bool bit7 = IS_BIT_SET(flags, 7);  // Check if bit 7 is set
```

#### PACK_BITS Macro

```cpp
#define PACK_BITS(bits, offset, flags, value) \
    ((((value) & ((1 << (bits)) - 1)) << (offset)) | ((flags) & (~(((1 << (bits)) - 1) << (offset))))
```

**Purpose**: Packs specified bits into flags at given offset

**Parameters**:
- bits: Number of bits to pack
- offset: Bit position to pack bits into
- flags: Existing flags to modify
- value: Value to pack (only lower 'bits' bits are used)

**Returns**: Modified flags with bits packed

**Behavior**:
- Masks value to extract only 'bits' bits: ((value) & ((1 << bits) - 1))
- Shifts masked value to offset position
- Clears target bits in flags: ((flags) & (~(((1 << bits) - 1) << offset))
- ORs shifted value with cleared flags

**Usage**:
```cpp
uint32_t flags = 0x12345678;
uint32_t newValue = 0xF;
flags = PACK_BITS(4, 8, flags, newValue);  // Pack 4 bits of newValue at offset 8
```

**TODO Comment**:
```cpp
// TODO: Add check that "value" fits into "bits"
```

#### PACK_BITS_UNSAFE Macro

```cpp
#define PACK_BITS_UNSAFE(bits, offset, flags, value) (((value) << (offset)) | ((flags) & (~(((1 << (bits)) - 1) << (offset))))
```

**Purpose**: Packs bits into flags without masking value (unsafe version)

**Parameters**:
- bits: Number of bits to pack
- offset: Bit position to pack bits into
- flags: Existing flags to modify
- value: Value to pack (all bits used, not masked)

**Returns**: Modified flags with bits packed

**Behavior**:
- Shifts entire value to offset position (no masking)
- Clears target bits in flags
- ORs shifted value with cleared flags

**Difference from PACK_BITS**:
- Does not mask value before shifting
- Assumes value already fits in 'bits' bits
- Faster but unsafe if value has bits outside range

**Usage**:
```cpp
uint32_t flags = 0x12345678;
uint32_t newValue = 0xF;  // Assumes only lower 4 bits are valid
flags = PACK_BITS_UNSAFE(4, 8, flags, newValue);  // Pack 4 bits at offset 8
```

### Constants

**File**: `Source/Falcor/Utils/HostDeviceShared.slangh` (line 46)

#### FALCOR_GPU_CACHE_SIZE

```cpp
#define FALCOR_GPU_CACHE_SIZE 128
```

**Purpose**: Defines GPU cache line size in bytes

**Value**: 128 bytes

**Usage**:
- Used for memory alignment
- Used for cache-friendly data structures
- Used for optimizing memory access patterns

### CPU-Specific Declarations

**File**: `Source/Falcor/Utils/HostDeviceShared.slangh` (lines 48-82)

#### Namespace Macros

```cpp
#define BEGIN_NAMESPACE(name) \
    namespace name            \
    {

#define END_NAMESPACE }
```

**Purpose**: Defines namespace begin/end macros

**Behavior**:
- BEGIN_NAMESPACE(name): Opens namespace with given name
- END_NAMESPACE: Closes namespace

**Usage**:
```cpp
BEGIN_NAMESPACE(MyNamespace)
void myFunction() { }
END_NAMESPACE

// Expands to:
namespace MyNamespace {
void myFunction() { }
}
```

#### Falcor Namespace Macros

```cpp
#define BEGIN_NAMESPACE_FALCOR BEGIN_NAMESPACE(Falcor)
#define END_NAMESPACE_FALCOR END_NAMESPACE
```

**Purpose**: Macros for Falcor namespace

**Behavior**:
- BEGIN_NAMESPACE_FALCOR: Opens Falcor namespace
- END_NAMESPACE_FALCOR: Closes Falcor namespace

**Usage**:
```cpp
BEGIN_NAMESPACE_FALCOR
void falcorFunction() { }
END_NAMESPACE_FALCOR

// Expands to:
namespace Falcor {
void falcorFunction() { }
}
```

#### Alignment Assertion Macro

```cpp
#define FALCOR_ASSERT_ALIGNMENT_FOR(T, ALIGNMENT) \
    static_assert((sizeof(T) >= ALIGNMENT) ? ((sizeof(T) % ALIGNMENT) == 0) : ((ALIGNMENT % sizeof(T)) == 0))
```

**Purpose**: Compile-time assertion for type alignment

**Parameters**:
- T: Type to check alignment for
- ALIGNMENT: Required alignment (in bytes)

**Behavior**:
- Checks if sizeof(T) >= ALIGNMENT:
  - If true: Asserts sizeof(T) % ALIGNMENT == 0
  - If false: Asserts ALIGNMENT % sizeof(T) == 0
- Ensures type either fits exactly into alignment or is multiple of alignment

**Logic**:
- If sizeof(T) >= ALIGNMENT: Type is at least alignment size
  - Check if sizeof(T) is multiple of ALIGNMENT
- If sizeof(T) < ALIGNMENT: Type is smaller than alignment
  - Check if ALIGNMENT is multiple of sizeof(T)

**Usage**:
```cpp
FALCOR_ASSERT_ALIGNMENT_FOR(float4, 16);  // Asserts float4 is 16-byte aligned
FALCOR_ASSERT_ALIGNMENT_FOR(uint32_t, 4);  // Asserts uint32_t is 4-byte aligned
```

#### Other CPU Macros

```cpp
#define SETTER_DECL
#define CONST_FUNCTION const
#define STD_NAMESPACE std::
```

**SETTER_DECL**: Placeholder for setter declarations

**CONST_FUNCTION**: Defines const function keyword (CPU uses 'const')

**STD_NAMESPACE**: Defines std namespace prefix (CPU uses 'std::')

#### CPU Includes

```cpp
#include "Core/Enum.h"
#include "Utils/Math/ScalarMath.h"
#include "Utils/Math/Vector.h"
#include "Utils/Math/Matrix.h"
#include <algorithm>
```

**Purpose**: Includes required headers for CPU compilation

#### CPU Namespace Aliases

```cpp
namespace Falcor
{
    using math::sign;
    using math::f16tof32;
    using math::f32tof16;
    using math::asfloat;
    using math::asfloat16;
    using math::asint;
    using math::asuint;
    using math::asuint16;
}
```

**Purpose**: Imports math functions into Falcor namespace for CPU

**Behavior**:
- Pulls math functions into Falcor namespace
- Simplifies code by not requiring math:: prefix

### GPU/HLSL-Specific Declarations

**File**: `Source/Falcor/Utils/HostDeviceShared.slangh` (lines 84-102)

#### Keyword Macros

```cpp
#define inline
#define constexpr const
```

**Purpose**: Redefines HLSL keywords for CPU compatibility

**Behavior**:
- `inline`: Redefined to nothing (HLSL uses 'inline' keyword)
- `constexpr`: Redefined to 'const' (HLSL doesn't have constexpr)

#### Namespace Macros

```cpp
#define BEGIN_NAMESPACE_FALCOR
#define END_NAMESPACE_FALCOR
#define BEGIN_NAMESPACE(name)
#define END_NAMESPACE
```

**Purpose**: Defines namespace macros for HLSL

**Behavior**:
- BEGIN_NAMESPACE_FALCOR: Empty (HLSL has different namespace syntax)
- END_NAMESPACE_FALCOR: Empty
- BEGIN_NAMESPACE(name): Empty
- END_NAMESPACE: Empty

#### Other GPU Macros

```cpp
#define SETTER_DECL [mutating]
#define CONST_FUNCTION
#define STD_NAMESPACE
```

**SETTER_DECL**: Defines as '[mutating]' (HLSL syntax for mutating functions)

**CONST_FUNCTION**: Empty (HLSL doesn't need const qualifier)

**STD_NAMESPACE**: Empty (HLSL doesn't use std namespace)

#### Alignment Assertion Macro

```cpp
#define FALCOR_ASSERT_ALIGNMENT_FOR(T, ALIGNMENT)
```

**Purpose**: Empty macro (alignment checking not needed in HLSL)

#### Enum Macros

```cpp
#define FALCOR_ENUM_INFO(T, ...)
#define FALCOR_ENUM_REGISTER(T)
```

**Purpose**: Placeholder macros for enum reflection

**Behavior**:
- FALCOR_ENUM_INFO: Empty (HLSL doesn't support enum reflection)
- FALCOR_ENUM_REGISTER: Empty

## Technical Details

### Platform Detection Strategy

Uses preprocessor macros to detect compilation target:

```
__STDC_HOSTED__ || __cplusplus -> HOST_CODE = 1 (CPU)
Otherwise                     -> HOST_CODE = 0 (GPU/HLSL)
```

**Detection Logic**:
- C standard hosted implementation: CPU
- C++ compilation: CPU
- Otherwise: GPU/HLSL

### Bit Manipulation Algorithms

**EXTRACT_BITS**:
```
1. Right-shift value by offset
2. Mask with (1 << bits) - 1
3. Return result
```

**Example**:
```cpp
value = 0b1100110010101110 (0xC97E)
EXTRACT_BITS(4, 8, value)
= (value >> 8) & ((1 << 4) - 1)
= 0b11001100 & 0b1111
= 0b0100 (0x4)
```

**IS_BIT_SET**:
```
1. Create mask with (1 << bitOffset)
2. AND with value
3. Check if non-zero
```

**Example**:
```cpp
value = 0b1100110010101110 (0xC97E)
IS_BIT_SET(value, 3)
= (value & (1 << 3)) != 0
= (0b1100110010101110 & 0b1000) != 0
= 0b0000 != 0
= false
```

**PACK_BITS**:
```
1. Mask value to extract only 'bits' bits
2. Shift masked value to offset position
3. Clear target bits in flags
4. OR shifted value with cleared flags
```

**Example**:
```cpp
flags = 0b1100110010101110 (0xC97E)
value = 0b1111 (0xF)
PACK_BITS(4, 8, flags, value)
= ((value & 0b1111) << 8) | (flags & ~(0b1111 << 8))
= (0b1111 << 8) | (0b1100110010101110 & ~0b111100000000)
= 0b111100000000 | 0b1100000010101110
= 0b1101111010101110 (0x6EAE)
```

### Alignment Checking Algorithm

**FALCOR_ASSERT_ALIGNMENT_FOR**:
```
if (sizeof(T) >= ALIGNMENT) {
    assert(sizeof(T) % ALIGNMENT == 0);
} else {
    assert(ALIGNMENT % sizeof(T) == 0);
}
```

**Logic**:
- If type is larger than alignment: Type must be multiple of alignment
- If type is smaller than alignment: Alignment must be multiple of type

**Examples**:
```cpp
FALCOR_ASSERT_ALIGNMENT_FOR(float4, 16)
// sizeof(float4) = 16, ALIGNMENT = 16
// 16 >= 16, so check 16 % 16 == 0 -> true

FALCOR_ASSERT_ALIGNMENT_FOR(uint32_t, 4)
// sizeof(uint32_t) = 4, ALIGNMENT = 4
// 4 >= 4, so check 4 % 4 == 0 -> true

FALCOR_ASSERT_ALIGNMENT_FOR(float3, 16)
// sizeof(float3) = 12, ALIGNMENT = 16
// 12 < 16, so check 16 % 12 == 0 -> false (fails!)
```

### Namespace Abstraction Strategy

**CPU**:
```cpp
BEGIN_NAMESPACE(Falcor)
void function() { }
END_NAMESPACE

// Expands to:
namespace Falcor {
void function() { }
}
```

**GPU/HLSL**:
```cpp
BEGIN_NAMESPACE_FALCOR
void function() { }
END_NAMESPACE_FALCOR

// Expands to:
// (nothing - HLSL uses different syntax)
```

### Conditional Compilation

Uses `#ifdef HOST_CODE` for platform-specific code:

```cpp
#ifdef HOST_CODE
// CPU-specific code
#else
// GPU/HLSL-specific code
#endif
```

## Integration Points

### Falcor Core Integration

- **Core/Enum**: Enum definitions and reflection
- **Utils/Math**: Math functions and types

### STL Integration

- **algorithm**: Standard algorithms (CPU only)

### Internal Falcor Usage

- **Scene**: Shared scene data structures
- **Rendering**: Shared rendering data structures
- **Utils**: Internal utilities

## Architecture Patterns

### Preprocessor Detection Pattern

Uses preprocessor for platform detection:

```cpp
#if (defined(__STDC_HOSTED__) || defined(__cplusplus))
#define HOST_CODE 1
#endif
```

Benefits:
- Compile-time platform detection
- Zero runtime overhead
- Platform-specific optimizations

### Macro Abstraction Pattern

Abstracts platform differences with macros:

```cpp
#ifdef HOST_CODE
#define CONST_FUNCTION const
#else
#define CONST_FUNCTION
#endif
```

Benefits:
- Single codebase for multiple platforms
- Platform-specific optimizations
- Clear abstraction

### Bit Manipulation Pattern

Provides bit manipulation utilities:

```cpp
#define EXTRACT_BITS(bits, offset, value) (((value) >> (offset)) & ((1 << (bits)) - 1))
```

Benefits:
- Efficient bit operations
- Compile-time evaluation
- No function call overhead

### Namespace Abstraction Pattern

Abstracts namespace differences:

```cpp
#define BEGIN_NAMESPACE(name) \
    namespace name            \
    {
```

Benefits:
- Consistent namespace syntax
- Platform-specific adaptations
- Simplifies cross-platform code

### Compile-Time Assertion Pattern

Uses static_assert for compile-time checking:

```cpp
#define FALCOR_ASSERT_ALIGNMENT_FOR(T, ALIGNMENT) \
    static_assert((sizeof(T) >= ALIGNMENT) ? ((sizeof(T) % ALIGNMENT) == 0) : ((ALIGNMENT % sizeof(T)) == 0))
```

Benefits:
- Compile-time error detection
- Zero runtime overhead
- Clear error messages

## Code Patterns

### Macro Definition Pattern

Consistent macro definition pattern:

```cpp
#define MACRO_NAME(parameters) \
    macro_body_continued_on_next_line
```

Pattern:
- Multi-line macros use backslash continuation
- Parenthesized parameters
- Clear macro names

### Conditional Compilation Pattern

Uses preprocessor for conditional code:

```cpp
#ifdef HOST_CODE
// CPU code
#else
// GPU code
#endif
```

Pattern:
- Platform detection
- Conditional compilation
- Platform-specific code paths

### Bit Manipulation Pattern

Efficient bit manipulation macros:

```cpp
#define EXTRACT_BITS(bits, offset, value) (((value) >> (offset)) & ((1 << (bits)) - 1))
```

Pattern:
- Bitwise operations
- Compile-time evaluation
- No function call overhead

### Namespace Macro Pattern

Abstracts namespace syntax:

```cpp
#define BEGIN_NAMESPACE(name) \
    namespace name            \
    {
```

Pattern:
- Macro for namespace opening
- Macro for namespace closing
- Platform-specific definitions

### Static Assert Pattern

Compile-time assertion macros:

```cpp
#define FALCOR_ASSERT_ALIGNMENT_FOR(T, ALIGNMENT) \
    static_assert((sizeof(T) >= ALIGNMENT) ? ((sizeof(T) % ALIGNMENT) == 0) : ((ALIGNMENT % sizeof(T)) == 0))
```

Pattern:
- Compile-time checking
- Conditional logic in macro
- Descriptive macro names

## Use Cases

### Bit Field Extraction

```cpp
uint32_t packedData = 0x12345678;

// Extract 8-bit field at offset 16
uint32_t field1 = EXTRACT_BITS(8, 16, packedData);

// Extract 4-bit field at offset 24
uint32_t field2 = EXTRACT_BITS(4, 24, packedData);
```

### Bit Flag Checking

```cpp
uint32_t flags = 0b10101010;

// Check if bit 3 is set
if (IS_BIT_SET(flags, 3)) {
    // Bit 3 is set
}

// Check if bit 7 is set
if (IS_BIT_SET(flags, 7)) {
    // Bit 7 is set
}
```

### Bit Field Packing

```cpp
uint32_t flags = 0x12345678;
uint32_t newValue = 0xAB;

// Pack 8-bit value at offset 8
flags = PACK_BITS(8, 8, flags, newValue);

// Pack 4-bit value at offset 24 (unsafe - assumes value fits)
flags = PACK_BITS_UNSAFE(4, 24, flags, newValue);
```

### Alignment Checking

```cpp
// Check alignment for float4
FALCOR_ASSERT_ALIGNMENT_FOR(float4, 16);

// Check alignment for uint32_t
FALCOR_ASSERT_ALIGNMENT_FOR(uint32_t, 4);
```

### Cross-Platform Code

```cpp
#ifdef HOST_CODE
// CPU-specific code
void function() {
    // CPU implementation
}
#else
// GPU-specific code
void function() {
    // GPU implementation
}
#endif
```

### Namespace Usage

```cpp
#ifdef HOST_CODE
BEGIN_NAMESPACE(Falcor)
void myFunction() { }
END_NAMESPACE
#else
BEGIN_NAMESPACE_FALCOR
void myFunction() { }
END_NAMESPACE_FALCOR
#endif
```

## Performance Considerations

### Macro Overhead

- **Zero Runtime Overhead**: All macros evaluated at compile time
- **No Function Calls**: Macros are text substitution
- **Inline Expansion**: Macros expanded inline

### Bit Manipulation Performance

- **Bitwise Operations**: Fast CPU instructions
- **Compile-Time Evaluation**: No runtime overhead
- **No Branching**: Pure arithmetic operations

### Conditional Compilation

- **Dead Code Elimination**: Unused code paths eliminated
- **Platform Optimization**: Platform-specific optimizations
- **Code Size**: Only relevant code included

### Alignment Checking

- **Compile-Time Checking**: Zero runtime overhead
- **Static Assert**: Compile-time error detection
- **No Runtime Checks**: No runtime alignment checks

### Cache Line Alignment

- **FALCOR_GPU_CACHE_SIZE**: 128-byte cache line alignment
- **Cache-Friendly**: Optimized for GPU cache
- **Memory Alignment**: Properly aligned data structures

## Limitations

### Macro Limitations

- **No Type Safety**: Macros don't provide type checking
- **No Debugging**: Difficult to debug macro expansions
- **Side Effects**: Macros can have unexpected side effects
- **Name Conflicts**: Macro names can conflict with other code

### Platform Limitations

- **Limited Platforms**: Only supports CPU and HLSL
- **Detection Issues**: May not detect all platforms correctly
- **HLSL Specific**: GPU code assumes HLSL syntax
- **No Vulkan**: No Vulkan shader support

### Bit Manipulation Limitations

- **No Bounds Checking**: EXTRACT_BITS doesn't check bounds
- **Unsafe Variants**: PACK_BITS_UNSAFE doesn't mask value
- **Signed Values**: Assumes unsigned values
- **Endianness**: No endianness handling

### Alignment Limitations

- **Static Only**: Only compile-time checking
- **HLSL Disabled**: Alignment checking disabled in HLSL
- **No Runtime Checking**: No runtime alignment verification
- **Limited Flexibility**: Only checks multiples/exact fit

### Feature Limitations

- **Minimal Feature Set**: Only provides basic utilities
- **No Advanced Features**: No advanced bit manipulation
- **No Reflection**: Limited enum reflection
- **No Serialization**: No serialization support

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Shared CPU/GPU data structures
- Bit field manipulation
- Alignment checking
- Cross-platform code
- Bit flag operations

**Inappropriate Use Cases**:
- Type-safe operations (use templates)
- Complex bit manipulation (use specialized libraries)
- Runtime alignment checking (use different approach)
- Non-bit operations (use direct operations)

### Usage Patterns

**Bit Field Extraction**:
```cpp
uint32_t field = EXTRACT_BITS(bits, offset, value);
```

**Bit Flag Checking**:
```cpp
if (IS_BIT_SET(flags, bitOffset)) {
    // Bit is set
}
```

**Bit Field Packing**:
```cpp
flags = PACK_BITS(bits, offset, flags, value);
```

**Unsafe Packing**:
```cpp
flags = PACK_BITS_UNSAFE(bits, offset, flags, value);  // Use carefully!
```

### Alignment Checking

- **Use Static Assert**: Use FALCOR_ASSERT_ALIGNMENT_FOR for compile-time checking
- **Check Power of Two**: Ensure alignment is power of two
- **Check Type Size**: Ensure type size is compatible with alignment

### Macro Safety

- **Parenthesize Parameters**: Always parenthesize macro parameters
- **Avoid Side Effects**: Don't use expressions with side effects in macros
- **Use Constexpr**: Prefer constexpr over macros when possible
- **Document Macros**: Document macro behavior clearly

### Platform-Specific Code

- **Use HOST_CODE**: Use HOST_CODE macro for platform detection
- **Minimize Platform Code**: Minimize platform-specific code
- **Abstract Differences**: Use macros to abstract platform differences
- **Test All Platforms**: Test on all supported platforms

### Performance Tips

1. **Use Bitwise Operations**: Use bit manipulation for flags
2. **Avoid Branching**: Use bitwise operations instead of branches
3. **Align Data**: Align data structures to cache lines
4. **Use Compile-Time**: Use compile-time operations when possible
5. **Profile**: Profile bit manipulation operations

## Implementation Notes

### Preprocessor Behavior

**Macro Expansion**:
- Macros are text substitution
- No type checking
- No scope (global)

**Conditional Compilation**:
- `#ifdef` checks if macro is defined
- `#if` evaluates constant expressions
- `#else` and `#elif` for alternatives

### Bit Manipulation Safety

**EXTRACT_BITS**:
- Safe: Always masks result
- No overflow: Right-shift is safe
- No undefined behavior: Well-defined for unsigned types

**PACK_BITS**:
- Safe: Masks value before shifting
- Clears target bits: Prevents accidental modification
- OR operation: Combines correctly

**PACK_BITS_UNSAFE**:
- Unsafe: Doesn't mask value
- Assumes value fits: Caller must ensure
- Faster: One less operation

### Alignment Checking

**Compile-Time Only**:
- static_assert: Compile-time assertion
- No runtime cost
- Fails compilation if assertion fails

**Logic**:
- Checks if type fits alignment or is multiple
- Handles both cases correctly
- Provides clear error messages

### Namespace Macros

**CPU**:
- Defines actual namespace syntax
- Works with C++ compilers
- Supports nested namespaces

**GPU/HLSL**:
- Empty definitions (HLSL syntax different)
- HLSL has its own namespace syntax
- No namespace macros needed

### TODO Comments

**Bit Packing**:
```cpp
// TODO: Add check that "value" fits into "bits"
```

**Purpose**: Reminder to add safety check

**Recommendation**: Add compile-time check for value range

## Future Enhancements

### Potential Improvements

1. **Value Range Checking**: Add compile-time check for PACK_BITS
2. **Endianness Support**: Add endianness handling
3. **More Bit Operations**: Add rotate, count bits, etc.
4. **Signed Type Support**: Add support for signed types
5. **Runtime Alignment**: Add runtime alignment checking

### API Extensions

1. **More Alignment Macros**: Add more alignment utilities
2. **Bit Field Struct**: Add bit field structure macros
3. **Enum Reflection**: Add enum reflection support
4. **Type Traits**: Add type trait macros
5. **Platform Detection**: Improve platform detection

### Safety Enhancements

1. **Bounds Checking**: Add bounds checking to EXTRACT_BITS
2. **Value Validation**: Add value validation to PACK_BITS
3. **Type Safety**: Add type safety to macros
4. **Static Analysis**: Add static analysis support
5. **Warnings**: Add compile-time warnings

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Troubleshooting**: Add troubleshooting guide
5. **Reference**: Add complete reference documentation

## References

### C++ Standard

- **Preprocessor**: C preprocessor directives
- **Bitwise Operations**: C bitwise operators
- **Static Assert**: C++11 static_assert
- **Type Traits**: C++11 type traits

### HLSL Documentation

- **HLSL Syntax**: High-Level Shading Language syntax
- **HLSL Data Types**: HLSL data types
- **HLSL Keywords**: HLSL keywords and qualifiers

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Math**: Falcor math library documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **Bit Manipulation**: Bit manipulation techniques
- **Cross-Platform Code**: Cross-platform programming
- **Shared Code**: Code sharing between platforms
- **Shader Languages**: GPU shader languages
