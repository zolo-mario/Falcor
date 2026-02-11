# NumericRange - Numeric Range Iterator

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Error (FALCOR_CHECK)
- std::utility
- std::stdexcept

### Dependent Modules

- Falcor/Utils (internal utilities)
- Falcor/Scene (scene utilities)
- Falcor/Rendering (rendering utilities)

## Module Overview

NumericRange provides a simple numeric range iterator for integral types that can be used in range-based for loops. It's designed as a temporary solution until C++20's `std::views::iota` is available. The template class is specialized for integral types only, using SFINAE (Substitution Failure Is Not An Error) to restrict instantiation to integral types.

## Component Specifications

### NumericRange Class (Primary Template)

**File**: `Source/Falcor/Utils/NumericRange.h` (lines 35-37)

```cpp
template<typename T, typename Enable = void>
class NumericRange final
{};
```

**Purpose**: Primary template that is empty

**Behavior**:
- Empty class for non-integral types
- Prevents instantiation for non-integral types
- Uses SFINAE to disable non-integral types

### NumericRange Class (Integral Specialization)

**File**: `Source/Falcor/Utils/NumericRange.h` (lines 43-79)

```cpp
template<typename T>
class NumericRange<T, typename std::enable_if<std::is_integral<T>::value>::type> final
{
    // ... implementation ...
};
```

**Purpose**: Specialization for integral types

**Template Parameters**:
- **T**: Integral type (int, uint, char, etc.)
- **Enable**: SFINAE parameter (std::enable_if<std::is_integral<T>::value>::type)

**Behavior**:
- Only instantiated for integral types
- Provides iterator interface
- Validates range construction

#### Iterator Class

**File**: `Source/Falcor/Utils/NumericRange.h` (lines 47-67)

```cpp
class Iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = T;
    using pointer = const T*;
    using reference = T;

    explicit Iterator(const T& value = T(0)) : mValue(value) {}
    const Iterator& operator++()
    {
        ++mValue;
        return *this;
    }
    bool operator!=(const Iterator& other) const { return other.mValue != mValue; }
    T operator*() const { return mValue; }

private:
    T mValue;
};
```

**Purpose**: Forward iterator for numeric range

**Type Definitions**:
- **iterator_category**: std::forward_iterator_tag (forward iterator)
- **value_type**: T (value type)
- **difference_type**: T (difference type)
- **pointer**: const T* (pointer type)
- **reference**: T (reference type)

**Constructor**:
```cpp
explicit Iterator(const T& value = T(0)) : mValue(value) {}
```

**Purpose**: Construct iterator with initial value

**Parameters**:
- value: Initial value (default: T(0))

**Behavior**:
- Initializes mValue to specified value
- Explicit constructor to prevent implicit conversion

**operator++()**:
```cpp
const Iterator& operator++()
{
    ++mValue;
    return *this;
}
```

**Purpose**: Pre-increment operator

**Returns**: Reference to *this after increment

**Behavior**:
- Increments mValue
- Returns reference to *this

**operator!=()**:
```cpp
bool operator!=(const Iterator& other) const { return other.mValue != mValue; }
```

**Purpose**: Inequality comparison operator

**Returns**: true if values are different, false otherwise

**Behavior**:
- Compares mValue with other.mValue
- Returns true if different

**operator***()**:
```cpp
T operator*() const { return mValue; }
```

**Purpose**: Dereference operator

**Returns**: Current value (mValue)

**Behavior**:
- Returns mValue by value (not reference)
- Returns const value

**Private Member**:
```cpp
T mValue;
```

**mValue**: Current iterator value

#### NumericRange Constructor

```cpp
explicit NumericRange(const T& begin, const T& end) : mBegin(begin), mEnd(end) { FALCOR_CHECK(begin <= end, "Invalid range"); }
```

**Purpose**: Construct numeric range

**Parameters**:
- begin: Start of range (inclusive)
- end: End of range (exclusive)

**Behavior**:
- Initializes mBegin to begin
- Initializes mEnd to end
- Validates that begin <= end
- Throws if range is invalid

**Deleted Constructors**:
```cpp
NumericRange() = delete;
NumericRange(const NumericRange&) = delete;
NumericRange(NumericRange&& other) = delete;
```

**Purpose**: Prevent default, copy, and move construction

**Behavior**:
- Default constructor deleted
- Copy constructor deleted
- Move constructor deleted

#### begin() Method

```cpp
Iterator begin() const { return Iterator(mBegin); }
```

**Purpose**: Returns iterator to beginning of range

**Returns**: Iterator pointing to mBegin

**Behavior**:
- Creates iterator with mBegin value
- Returns iterator by value

#### end() Method

```cpp
Iterator end() const { return Iterator(mEnd); }
```

**Purpose**: Returns iterator to end of range

**Returns**: Iterator pointing to mEnd

**Behavior**:
- Creates iterator with mEnd value
- Returns iterator by value

#### Private Members

```cpp
T mBegin, mEnd;
```

**mBegin**: Start of range (inclusive)

**mEnd**: End of range (exclusive)

## Technical Details

### SFINAE Pattern

Uses SFINAE to restrict template instantiation:

```cpp
template<typename T, typename Enable = void>
class NumericRange final
{};  // Empty for non-integral types

template<typename T>
class NumericRange<T, typename std::enable_if<std::is_integral<T>::value>::type> final
{
    // ... implementation for integral types ...
};
```

**Purpose**: Prevents instantiation for non-integral types

**Behavior**:
- Primary template is empty
- Specialization enabled for integral types
- Non-integral types fail substitution

### Iterator Interface

Implements forward iterator interface:

```cpp
using iterator_category = std::forward_iterator_tag;
using value_type = T;
using difference_type = T;
using pointer = const T*;
using reference = T;
```

**Characteristics**:
- Forward iterator (can only move forward)
- Single-pass iterator
- Const iterator (returns const values)
- No random access

### Range-Based For Loop Compatibility

Compatible with range-based for loops:

```cpp
for (auto i : NumericRange<int>(0, 10)) {
    // i = 0, 1, 2, ..., 9
}
```

**Behavior**:
- Calls begin() to get start iterator
- Calls end() to get end iterator
- Iterates from begin to end (exclusive)
- Uses operator!= for comparison
- Uses operator*() for dereferencing

### Range Validation

Validates range on construction:

```cpp
FALCOR_CHECK(begin <= end, "Invalid range");
```

**Purpose**: Ensures valid range

**Behavior**:
- Checks that begin <= end
- Throws if range is invalid
- Prevents infinite loops

### Const Iterator

Iterator returns const values:

```cpp
T operator*() const { return mValue; }
```

**Characteristics**:
- Returns value by copy
- No reference to internal state
- Cannot modify through iterator

### Non-Copyable/Non-Movable

Prevents copy and move:

```cpp
NumericRange(const NumericRange&) = delete;
NumericRange(NumericRange&& other) = delete;
```

**Purpose**: Prevent accidental copying/moving

**Behavior**:
- Copy constructor deleted
- Move constructor deleted
- Forces explicit construction

### Iterator Implementation

Iterator implementation details:

```cpp
explicit Iterator(const T& value = T(0)) : mValue(value) {}

const Iterator& operator++()
{
    ++mValue;
    return *this;
}

bool operator!=(const Iterator& other) const { return other.mValue != mValue; }

T operator*() const { return mValue; }
```

**Characteristics**:
- Pre-increment only (no post-increment)
- Inequality comparison only (no equality)
- Dereference returns value (not reference)
- Const iterator (cannot modify)

### Memory Layout

Memory layout of NumericRange:

```
NumericRange<T>:
    mBegin: T (begin value)
    mEnd: T (end value)
    Total size: 2 * sizeof(T)

Iterator:
    mValue: T (current value)
    Total size: sizeof(T)
```

**Characteristics**:
- Simple POD type
- No dynamic allocation
- No virtual functions
- Minimal memory overhead

## Integration Points

### Falcor Core Integration

- **Core/Error**: FALCOR_CHECK macro for validation

### STL Integration

- **std::utility**: std::enable_if for SFINAE
- **std::stdexcept**: For exception handling

### Internal Falcor Usage

- **Utils**: Internal utilities
- **Scene**: Scene utilities
- **Rendering**: Rendering utilities

## Architecture Patterns

### SFINAE Pattern

Use SFINAE to restrict template instantiation:

```cpp
template<typename T, typename Enable = void>
class NumericRange final
{};  // Empty for non-integral types

template<typename T>
class NumericRange<T, typename std::enable_if<std::is_integral<T>::value>::type> final
{
    // ... implementation ...
};
```

Benefits:
- Compile-time type checking
- Clear error messages
- Type-safe implementation

### Iterator Pattern

Implement iterator interface for range-based for loops:

```cpp
class Iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = T;
    using pointer = const T*;
    using reference = T;

    // ... iterator methods ...
};
```

Benefits:
- Compatible with range-based for loops
- Standard iterator interface
- Easy to use

### Const Iterator Pattern

Iterator returns const values:

```cpp
T operator*() const { return mValue; }
```

Benefits:
- Prevents modification through iterator
- Clear semantics
- Safe iteration

### Validation Pattern

Validate input parameters:

```cpp
explicit NumericRange(const T& begin, const T& end) : mBegin(begin), mEnd(end)
{
    FALCOR_CHECK(begin <= end, "Invalid range");
}
```

Benefits:
- Early error detection
- Clear error messages
- Prevents undefined behavior

### Deleted Functions Pattern

Prevent copy and move:

```cpp
NumericRange(const NumericRange&) = delete;
NumericRange(NumericRange&& other) = delete;
```

Benefits:
- Prevents accidental copying
- Clear ownership semantics
- Forces explicit construction

## Code Patterns

### SFINAE Pattern

Restrict template instantiation:

```cpp
template<typename T, typename Enable = void>
class NumericRange final
{};  // Empty for non-integral types

template<typename T>
class NumericRange<T, typename std::enable_if<std::is_integral<T>::value>::type> final
{
    // ... implementation ...
};
```

Pattern:
- Primary template is empty
- Specialization enabled for specific types
- Non-matching types fail substitution

### Iterator Pattern

Implement iterator interface:

```cpp
class Iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = T;
    using pointer = const T*;
    using reference = T;

    explicit Iterator(const T& value = T(0)) : mValue(value) {}

    const Iterator& operator++()
    {
        ++mValue;
        return *this;
    }

    bool operator!=(const Iterator& other) const { return other.mValue != mValue; }

    T operator*() const { return mValue; }

private:
    T mValue;
};
```

Pattern:
- Define iterator type traits
- Implement increment operator
- Implement comparison operator
- Implement dereference operator

### Validation Pattern

Validate input parameters:

```cpp
explicit NumericRange(const T& begin, const T& end) : mBegin(begin), mEnd(end)
{
    FALCOR_CHECK(begin <= end, "Invalid range");
}
```

Pattern:
- Check invariants in constructor
- Use FALCOR_CHECK for validation
- Provide clear error messages

### Deleted Functions Pattern

Prevent copy and move:

```cpp
NumericRange(const NumericRange&) = delete;
NumericRange(NumericRange&& other) = delete;
```

Pattern:
- Delete copy constructor
- Delete move constructor
- Prevent accidental copying

### Const Iterator Pattern

Return const values:

```cpp
T operator*() const { return mValue; }
```

Pattern:
- Return value by copy
- No reference to internal state
- Cannot modify through iterator

## Use Cases

### Basic Range Iteration

```cpp
// Iterate from 0 to 9
for (auto i : NumericRange<int>(0, 10)) {
    std::cout << i << " ";
}
// Output: 0 1 2 3 4 5 6 7 8 9
```

### Loop Over Array Indices

```cpp
std::vector<int> data = {1, 2, 3, 4, 5};

// Iterate over array indices
for (auto i : NumericRange<size_t>(0, data.size())) {
    std::cout << data[i] << " ";
}
// Output: 1 2 3 4 5
```

### Loop Over Custom Range

```cpp
// Iterate from 5 to 14
for (auto i : NumericRange<int>(5, 15)) {
    std::cout << i << " ";
}
// Output: 5 6 7 8 9 10 11 12 13 14
```

### Unsigned Range

```cpp
// Iterate over unsigned range
for (auto i : NumericRange<uint32_t>(0, 10)) {
    std::cout << i << " ";
}
// Output: 0 1 2 3 4 5 6 7 8 9
```

### Char Range

```cpp
// Iterate over character range
for (auto c : NumericRange<char>('a', 'z' + 1)) {
    std::cout << c;
}
// Output: abcdefghijklmnopqrstuvwxyz
```

### Nested Loops

```cpp
// Nested loops for 2D iteration
for (auto y : NumericRange<int>(0, 10)) {
    for (auto x : NumericRange<int>(0, 10)) {
        std::cout << "(" << x << "," << y << ") ";
    }
    std::cout << std::endl;
}
```

### Algorithm Iteration

```cpp
// Use with algorithms
std::vector<int> result;
std::copy(NumericRange<int>(0, 10).begin(),
          NumericRange<int>(0, 10).end(),
          std::back_inserter(result));
// result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
```

### Range-Based For Loop with Algorithms

```cpp
// Use with range-based for loop and algorithms
for (auto i : NumericRange<int>(0, 10)) {
    if (i % 2 == 0) {
        std::cout << i << " ";
    }
}
// Output: 0 2 4 6 8
```

## Performance Considerations

### Memory Overhead

- **Range Size**: 2 * sizeof(T) (mBegin + mEnd)
- **Iterator Size**: sizeof(T) (mValue)
- **No Dynamic Allocation**: No heap allocation
- **No Virtual Functions**: No vtable overhead

### Iteration Performance

- **Increment Cost**: Simple integer increment
- **Comparison Cost**: Simple integer comparison
- **Dereference Cost**: Return by value (copy)
- **No Bounds Checking**: No runtime bounds checking

### Compiler Optimizations

- **Inline Expansion**: Iterator methods can be inlined
- **Loop Unrolling**: Compiler can unroll loops
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated

### Comparison with Alternatives

**NumericRange vs std::views::iota**:
- NumericRange: Custom implementation, C++11
- std::views::iota: Standard library, C++20
- Performance: Similar (both simple integer iteration)

**NumericRange vs Traditional For Loop**:
- NumericRange: More expressive, range-based for loop
- Traditional For Loop: More control, index variable
- Performance: Similar (both compile to same code)

### Optimization Tips

1. **Use Const Range**: Use const NumericRange when possible
2. **Avoid Copying**: Avoid copying iterators unnecessarily
3. **Use Appropriate Type**: Use smallest integral type for range
4. **Compiler Optimizations**: Enable compiler optimizations

## Limitations

### Feature Limitations

- **Integral Types Only**: Works only with integral types
- **Forward Iterator Only**: No random access or bidirectional
- **No Post-Increment**: Only pre-increment operator
- **No Equality Operator**: Only inequality operator
- **No Range Operations**: No range operations (size, empty, etc.)
- **No Reverse Iteration**: No reverse iteration support
- **No Step Control**: No step control (always step by 1)
- **No Negative Step**: No negative step (decrement)

### API Limitations

- **No Size Method**: No method to get range size
- **No Empty Method**: No method to check if range is empty
- **No Contains Method**: No method to check if value is in range
- **No Front/Back Methods**: No front/back methods
- **No Index Access**: No random access by index
- **No Range Operations**: No range operations (union, intersection, etc.)

### Type Limitations

- **Integral Types Only**: Only works with integral types
- **No Floating Point**: No floating point support
- **No Custom Types**: No custom type support
- **No Pointer Types**: No pointer type support

### Performance Limitations

- **Return by Value**: Iterator returns value by copy
- **No Bounds Checking**: No runtime bounds checking
- **No Optimization Hints**: No optimization hints for compiler

### Platform Limitations

- **C++11 Only**: Requires C++11 or later
- **No C++20 Features**: No C++20 features (views, ranges)
- **No Standard Library Views**: No standard library views support

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Simple numeric iteration
- Loop over array indices
- Range-based for loops
- Algorithm iteration
- Nested loops

**Inappropriate Use Cases**:
- Floating point iteration (use other methods)
- Custom step size (use other methods)
- Reverse iteration (use other methods)
- Random access (use other methods)
- Complex range operations (use other methods)

### Usage Patterns

**Basic Iteration**:
```cpp
for (auto i : NumericRange<int>(0, 10)) {
    // Process i
}
```

**Array Index Iteration**:
```cpp
for (auto i : NumericRange<size_t>(0, data.size())) {
    // Process data[i]
}
```

**Nested Loops**:
```cpp
for (auto y : NumericRange<int>(0, height)) {
    for (auto x : NumericRange<int>(0, width)) {
        // Process (x, y)
    }
}
```

**Algorithm Iteration**:
```cpp
std::copy(NumericRange<int>(0, 10).begin(),
          NumericRange<int>(0, 10).end(),
          std::back_inserter(result));
```

### Type Safety

- **Use Appropriate Type**: Use smallest integral type for range
- **Avoid Overflow**: Be aware of integer overflow
- **Use Unsigned for Indices**: Use unsigned types for array indices
- **Use Signed for Values**: Use signed types for values

### Performance Tips

1. **Use Const Range**: Use const NumericRange when possible
2. **Avoid Copying**: Avoid copying iterators unnecessarily
3. **Use Appropriate Type**: Use smallest integral type for range
4. **Enable Optimizations**: Enable compiler optimizations
5. **Use Range-Based For Loop**: Use range-based for loop for clarity

### Error Handling

- **Validate Range**: Ensure begin <= end
- **Handle Exceptions**: Handle exceptions from FALCOR_CHECK
- **Check Type**: Ensure type is integral
- **Avoid Overflow**: Be aware of integer overflow

### Migration to C++20

- **Use std::views::iota**: Use std::views::iota when available
- **Replace NumericRange**: Replace NumericRange with std::views::iota
- **Update Code**: Update code to use C++20 features
- **Test Thoroughly**: Test thoroughly after migration

## Implementation Notes

### SFINAE Implementation

Uses SFINAE to restrict template instantiation:

```cpp
template<typename T, typename Enable = void>
class NumericRange final
{};  // Empty for non-integral types

template<typename T>
class NumericRange<T, typename std::enable_if<std::is_integral<T>::value>::type> final
{
    // ... implementation ...
};
```

**Characteristics**:
- Primary template is empty
- Specialization enabled for integral types
- Non-integral types fail substitution
- Compile-time type checking

### Iterator Implementation

Iterator implementation details:

```cpp
class Iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = T;
    using pointer = const T*;
    using reference = T;

    explicit Iterator(const T& value = T(0)) : mValue(value) {}

    const Iterator& operator++()
    {
        ++mValue;
        return *this;
    }

    bool operator!=(const Iterator& other) const { return other.mValue != mValue; }

    T operator*() const { return mValue; }

private:
    T mValue;
};
```

**Characteristics**:
- Forward iterator only
- Pre-increment only
- Inequality comparison only
- Dereference returns value by copy
- Const iterator

### Range Validation

Validates range on construction:

```cpp
explicit NumericRange(const T& begin, const T& end) : mBegin(begin), mEnd(end)
{
    FALCOR_CHECK(begin <= end, "Invalid range");
}
```

**Characteristics**:
- Checks that begin <= end
- Throws if range is invalid
- Uses FALCOR_CHECK macro
- Prevents infinite loops

### Deleted Functions

Prevents copy and move:

```cpp
NumericRange() = delete;
NumericRange(const NumericRange&) = delete;
NumericRange(NumericRange&& other) = delete;
```

**Characteristics**:
- Default constructor deleted
- Copy constructor deleted
- Move constructor deleted
- Forces explicit construction

### Memory Layout

Memory layout of NumericRange:

```
NumericRange<T>:
    mBegin: T (begin value)
    mEnd: T (end value)
    Total size: 2 * sizeof(T)

Iterator:
    mValue: T (current value)
    Total size: sizeof(T)
```

**Characteristics**:
- Simple POD type
- No dynamic allocation
- No virtual functions
- Minimal memory overhead

### C++20 Replacement

Intended to be replaced with C++20's std::views::iota:

```cpp
// C++11 (current)
for (auto i : NumericRange<int>(0, 10)) {
    // Process i
}

// C++20 (future)
for (auto i : std::views::iota(0, 10)) {
    // Process i
}
```

**Characteristics**:
- Temporary solution
- Will be replaced with std::views::iota
- Similar functionality
- Standard library implementation

## Future Enhancements

### Potential Improvements

1. **C++20 Migration**: Migrate to std::views::iota
2. **Step Support**: Add support for custom step size
3. **Reverse Iteration**: Add reverse iteration support
4. **Bidirectional Iterator**: Implement bidirectional iterator
5. **Random Access Iterator**: Implement random access iterator
6. **Range Operations**: Add range operations (size, empty, contains, etc.)
7. **Floating Point Support**: Add floating point support
8. **Custom Type Support**: Add custom type support
9. **Range Algorithms**: Add range algorithms (union, intersection, etc.)
10. **Optimization Hints**: Add optimization hints for compiler

### API Extensions

1. **size() Method**: Add method to get range size
2. **empty() Method**: Add method to check if range is empty
3. **contains() Method**: Add method to check if value is in range
4. **front() Method**: Add method to get first value
5. **back() Method**: Add method to get last value
6. **operator[]()**: Add random access by index
7. **step() Constructor**: Add constructor with step parameter
8. **reverse() Method**: Add method to get reverse range
9. **intersect() Method**: Add method to intersect ranges
10. **union() Method**: Add method to union ranges

### Performance Enhancements

1. **Return by Reference**: Consider returning by reference instead of by value
2. **Bounds Checking**: Add optional bounds checking
3. **Compiler Hints**: Add optimization hints for compiler
4. **SIMD Optimization**: Consider SIMD optimization for iteration
5. **Loop Unrolling**: Consider loop unrolling for small ranges

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide to C++20
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::enable_if**: C++11 type trait for SFINAE
- **std::is_integral**: C++11 type trait for integral types
- **std::forward_iterator_tag**: C++11 iterator tag for forward iterators

### C++20 Features

- **std::views::iota**: C++20 view for numeric ranges
- **std::ranges**: C++20 ranges library
- **std::views**: C++20 views library

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **Iterator Pattern**: Iterator design pattern
- **SFINAE**: Substitution Failure Is Not An Error
- **Range-Based For Loop**: C++11 range-based for loop
