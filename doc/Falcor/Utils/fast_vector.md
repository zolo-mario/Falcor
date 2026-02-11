# fast_vector - Fast Vector Implementation

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- STL (vector, memory, type_traits)

### Dependent Modules

- Falcor/Scene (scene data structures)
- Falcor/Rendering (rendering data structures)
- Falcor/Utils (internal utilities)

## Module Overview

fast_vector provides a lightweight, lock-free alternative to `std::vector` specifically designed to avoid MSVC's global lock on `push_back()` in debug builds. The implementation is restricted to trivial types (POD types) and uses manual memory management with `std::unique_ptr` for ownership. It's optimized for performance-critical paths where the overhead of MSVC's debug vector locking is unacceptable.

## Component Specifications

### fast_vector Class

**File**: `Source/Falcor/Utils/fast_vector.h` (140 lines)

Template class providing lock-free vector operations for trivial types.

#### Template Parameters

```cpp
template<typename T>
class fast_vector
```

**Constraints**:
```cpp
static_assert(std::is_trivial_v<T>, "Fast vector can only be used on trivial types.");
```

**T**: Element type (must be trivial/POD)

**Trivial Type Requirements**:
- Has trivial default constructor
- Has trivial copy constructor
- Has trivial move constructor
- Has trivial copy assignment
- Has trivial move assignment
- Has trivial destructor

**Examples of Valid Types**:
- `int`, `float`, `double`
- `int2`, `float3`, `float4` (math vectors)
- `uint32_t`, `uint64_t`
- POD structs with trivial members

**Examples of Invalid Types**:
- `std::string` (non-trivial)
- `std::vector` (non-trivial)
- Classes with virtual functions (non-trivial)
- Classes with user-defined constructors (non-trivial)

#### Static Assert

```cpp
static_assert(std::is_trivial_v<T>, "Fast vector can only be used on trivial types.");
```

**Purpose**: Compile-time check for trivial type constraint

**Behavior**:
- Fails compilation if T is not trivial
- Provides descriptive error message
- Enforces performance guarantee

#### Constructors

**Default Constructor**:
```cpp
fast_vector() = default;
```

**Purpose**: Creates empty vector

**Behavior**:
- Default-initializes all members
- m_capacity = 0
- m_size = 0
- m_data = nullptr
- No memory allocation

**Copy Constructor from std::vector**:
```cpp
fast_vector(const std::vector<T>& other) { assign(other.begin(), other.end()); }
```

**Purpose**: Constructs from std::vector

**Behavior**:
- Calls assign() with vector's begin/end iterators
- Copies all elements from other vector
- Parameters:
  - other: std::vector to copy from

**Copy Constructor from fast_vector**:
```cpp
fast_vector(const fast_vector& other) { assign(other.begin(), other.end()); }
```

**Purpose**: Copy constructor

**Behavior**:
- Calls assign() with other's begin/end iterators
- Copies all elements from other fast_vector
- Parameters:
  - other: fast_vector to copy from

**Move Constructor**:
```cpp
fast_vector(fast_vector&& other)
    : m_capacity(std::exchange(other.m_capacity, 0)),
      m_size(std::exchange(other.m_size, 0)),
      m_data(std::move(other.m_data))
{}
```

**Purpose**: Move constructor

**Behavior**:
- Transfers ownership from other
- Uses std::exchange to zero out other's members
- Moves m_data via std::move
- Parameters:
  - other: fast_vector to move from

**After Move**:
- this: Owns other's data
- other: Empty (capacity=0, size=0, data=nullptr)

#### Assignment Operators

**Assignment from std::vector**:
```cpp
fast_vector& operator=(const std::vector<T>& other)
{
    assign(other.begin(), other.end());
    return *this;
}
```

**Purpose**: Assigns from std::vector

**Behavior**:
- Calls assign() with vector's begin/end iterators
- Replaces current contents
- Parameters:
  - other: std::vector to assign from
- Returns:
  - Reference to *this

**Assignment from fast_vector**:
```cpp
fast_vector& operator=(const fast_vector& other)
{
    assign(other.begin(), other.end());
    return *this;
}
```

**Purpose**: Copy assignment

**Behavior**:
- Calls assign() with other's begin/end iterators
- Replaces current contents
- Parameters:
  - other: fast_vector to copy from
- Returns:
  - Reference to *this

**Move Assignment**:
```cpp
fast_vector& operator=(fast_vector&& other)
{
    m_size = std::exchange(other.m_size, 0);
    m_capacity = std::exchange(other.m_capacity, 0);
    m_data = std::move(other.m_data);
    return *this;
}
```

**Purpose**: Move assignment

**Behavior**:
- Transfers ownership from other
- Uses std::exchange to zero out other's members
- Moves m_data via std::move
- Parameters:
  - other: fast_vector to move from
- Returns:
  - Reference to *this

**After Move**:
- this: Owns other's data
- other: Empty (capacity=0, size=0, data=nullptr)

#### Capacity Management

**reserve()**:
```cpp
void reserve(size_t capacity) { grow(capacity); }
```

**Purpose**: Reserves capacity for future growth

**Behavior**:
- Calls grow() with requested capacity
- Grows if required_size > m_capacity
- Does not change size
- Parameters:
  - capacity: Minimum capacity to reserve

**Usage**:
```cpp
fast_vector<int> vec;
vec.reserve(100);  // Reserve space for 100 elements
```

**resize(size)**:
```cpp
void resize(size_t size)
{
    grow(size);
    m_size = size;
}
```

**Purpose**: Resizes vector to specified size

**Behavior**:
- Calls grow() with requested size
- Sets m_size to requested size
- New elements are uninitialized (trivial types)
- Parameters:
  - size: New size

**Usage**:
```cpp
fast_vector<int> vec;
vec.resize(10);  // Resize to 10 elements
```

**resize(size, value)**:
```cpp
void resize(size_t capacity, const T& value)
{
    grow(capacity);
    for (size_t i = m_size; i < capacity; ++i)
        m_data[i] = value;
    m_size = capacity;
}
```

**Purpose**: Resizes vector and fills new elements with value

**Behavior**:
- Calls grow() with requested capacity
- Fills new elements (from m_size to capacity) with value
- Sets m_size to capacity
- Parameters:
  - capacity: New size
  - value: Value to fill new elements with

**Usage**:
```cpp
fast_vector<int> vec;
vec.resize(10, 42);  // Resize to 10 elements, fill with 42
```

#### Element Access

**push_back()**:
```cpp
void push_back(const T& v)
{
    if (m_size + 1 > m_capacity)
        grow(m_size + 1);
    m_data[m_size++] = v;
}
```

**Purpose**: Adds element to end of vector

**Behavior**:
- Checks if capacity needs to grow
- Calls grow() if needed
- Copies element to m_data[m_size]
- Increments m_size
- Parameters:
  - v: Element to add

**Key Feature**: No global lock (unlike MSVC's debug std::vector)

**Usage**:
```cpp
fast_vector<int> vec;
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);
```

**assign()**:
```cpp
template<typename FwdIterator>
void assign(FwdIterator b, FwdIterator e)
{
    resize(std::distance(b, e));
    for (size_t i = 0; i < size(); ++i, ++b)
        m_data[i] = *b;
}
```

**Purpose**: Assigns elements from iterator range

**Behavior**:
- Resizes to iterator distance
- Copies elements from iterator range
- Parameters:
  - b: Begin iterator
  - e: End iterator

**Usage**:
```cpp
std::vector<int> src = {1, 2, 3, 4, 5};
fast_vector<int> vec;
vec.assign(src.begin(), src.end());
```

#### Conversion

**Conversion to std::vector**:
```cpp
operator std::vector<T>() { return std::vector<T>(begin(), end()); }
```

**Purpose**: Converts to std::vector

**Behavior**:
- Creates std::vector from begin/end iterators
- Returns new std::vector
- Returns:
  - std::vector with copy of data

**Usage**:
```cpp
fast_vector<int> fastVec = {1, 2, 3};
std::vector<int> stdVec = fastVec;  // Implicit conversion
```

#### Size Queries

**size()**:
```cpp
inline size_t size() const { return m_size; }
```

**Purpose**: Returns number of elements

**Returns**: Current size (number of elements)

**capacity()**:
```cpp
inline size_t capacity() const { return m_capacity; }
```

**Purpose**: Returns allocated capacity

**Returns**: Current capacity (number of elements that can be stored without reallocation)

**empty()**:
```cpp
inline bool empty() const { return m_size == 0; }
```

**Purpose**: Checks if vector is empty

**Returns**: true if size is 0, false otherwise

**clear()**:
```cpp
inline void clear() { m_size = 0; }
```

**Purpose**: Clears vector (sets size to 0)

**Behavior**:
- Sets m_size to 0
- Does not free memory (capacity unchanged)
- Does not destroy elements (trivial types)

#### Iterators

**begin()**:
```cpp
inline T* begin() { return data(); }
```

**Purpose**: Returns iterator to first element

**Returns**: Pointer to first element (or data() if empty)

**end()**:
```cpp
inline T* end() { return begin() + size(); }
```

**Purpose**: Returns iterator to past-the-end element

**Returns**: Pointer past the last element

**const begin()**:
```cpp
inline const T* begin() const { return data(); }
```

**Purpose**: Returns const iterator to first element

**Returns**: Const pointer to first element

**const end()**:
```cpp
inline const T* end() const { return begin() + size(); }
```

**Purpose**: Returns const iterator to past-the-end element

**Returns**: Const pointer past the last element

#### Data Access

**data()**:
```cpp
inline T* data() { return m_data.get(); }
```

**Purpose**: Returns pointer to underlying array

**Returns**: Raw pointer to data (may be null if empty)

**const data()**:
```cpp
inline const T* data() const { return m_data.get(); }
```

**Purpose**: Returns const pointer to underlying array

**Returns**: Const raw pointer to data (may be null if empty)

**operator[]**:
```cpp
inline T& operator[](size_t pos) { return m_data[pos]; }
```

**Purpose**: Access element by position

**Behavior**:
- Returns reference to element at position
- No bounds checking (undefined behavior if pos >= size)
- Parameters:
  - pos: Element position
- Returns:
  - Reference to element

**const operator[]**:
```cpp
inline const T& operator[](size_t pos) const { return m_data[pos]; }
```

**Purpose**: Access element by position (const)

**Behavior**:
- Returns const reference to element at position
- No bounds checking (undefined behavior if pos >= size)
- Parameters:
  - pos: Element position
- Returns:
  - Const reference to element

**Usage**:
```cpp
fast_vector<int> vec = {1, 2, 3};
int first = vec[0];      // 1
int second = vec[1];     // 2
vec[0] = 10;           // Modify first element
```

#### Private Methods

**grow()**:
```cpp
private:
void grow(size_t required_size)
{
    if (required_size <= m_capacity)
        return;
    size_t new_size = std::max(m_capacity * 2, required_size);
    std::unique_ptr<T[]> new_data = std::make_unique<T[]>(new_size);
    memcpy(new_data.get(), m_data.get(), m_size * sizeof(T));
    m_data = std::move(new_data);
    m_capacity = new_size;
}
```

**Purpose**: Grows capacity to accommodate required size

**Behavior**:
- Returns early if required_size <= m_capacity
- Calculates new_size = max(m_capacity * 2, required_size)
- Allocates new array with new_size
- Copies existing data via memcpy (trivial types)
- Moves new_data into m_data
- Updates m_capacity to new_size
- Parameters:
  - required_size: Minimum required capacity

**Growth Strategy**:
- Exponential growth (doubles capacity)
- At least required_size
- Typical: 0 -> 1 -> 2 -> 4 -> 8 -> 16 -> 32 -> ...

#### Member Variables

```cpp
private:
    size_t m_capacity{0};
    size_t m_size{0};
    std::unique_ptr<T[]> m_data;
```

**m_capacity**: Allocated capacity (number of elements that can be stored)

**m_size**: Current size (number of elements stored)

**m_data**: Underlying data array (owned via std::unique_ptr<T[]>)

## Technical Details

### Trivial Type Constraint

The static_assert enforces trivial type constraint:

```cpp
static_assert(std::is_trivial_v<T>, "Fast vector can only be used on trivial types.");
```

**Trivial Type Properties**:
- Can be safely copied with memcpy
- No user-defined constructors/destructors
- No virtual functions
- No non-trivial members

**Benefits**:
- Allows use of memcpy for copying
- No need to call constructors/destructors
- Predictable memory layout
- Optimizable by compiler

**Examples**:
```cpp
fast_vector<int> v1;           // OK (int is trivial)
fast_vector<float3> v2;         // OK (float3 is trivial)
fast_vector<std::string> v3;    // Error (std::string is not trivial)
```

### Exponential Growth Strategy

The grow() function uses exponential growth:

```cpp
size_t new_size = std::max(m_capacity * 2, required_size);
```

**Growth Pattern**:
- Initial: capacity = 0
- After 1st push: capacity = 1
- After 2nd push: capacity = 2
- After 3rd push: capacity = 4
- After 5th push: capacity = 8
- After 9th push: capacity = 16

**Benefits**:
- Amortized O(1) push_back
- Fewer reallocations
- Better cache locality

**Trade-offs**:
- Potential memory waste (up to 2x used)
- Larger peak memory usage

### Memory Management

Uses `std::unique_ptr<T[]>` for ownership:

```cpp
std::unique_ptr<T[]> m_data;
```

**Benefits**:
- Automatic cleanup on destruction
- No manual delete[]
- Exception-safe
- Moveable

**Allocation**:
```cpp
std::unique_ptr<T[]> new_data = std::make_unique<T[]>(new_size);
```

**Deallocation**:
- Automatic when m_data is destroyed or moved
- No manual cleanup needed

### memcpy for Copying

Uses memcpy for copying trivial types:

```cpp
memcpy(new_data.get(), m_data.get(), m_size * sizeof(T));
```

**Benefits**:
- Fast memory copy
- Optimized by compiler/CPU
- No per-element constructor calls

**Safety**:
- Safe for trivial types only
- Preserves bit pattern
- No type conversion

### MSVC Debug Vector Locking

**Problem**: MSVC's std::vector has a global lock on push_back() in debug builds

**Impact**:
- Severe performance degradation in multithreaded debug builds
- Contention on global lock
- Scalability issues

**Solution**: fast_vector avoids this lock

**Benefits**:
- No global locking
- Better multithreaded performance
- Scalable push_back operations

**Trade-offs**:
- Trivial types only
- Minimal feature set
- Manual memory management

### Iterator Interface

Provides pointer-based iterators:

```cpp
inline T* begin() { return data(); }
inline T* end() { return begin() + size(); }
```

**Characteristics**:
- Random access iterators (pointers)
- No bounds checking
- Invalidated on reallocation
- Valid until next grow()

**Usage**:
```cpp
fast_vector<int> vec = {1, 2, 3};

// Range-based for loop
for (int& val : vec) {
    // Process val
}

// Pointer arithmetic
for (int* it = vec.begin(); it != vec.end(); ++it) {
    // Process *it
}
```

### STL Compatibility

Supports conversion to/from std::vector:

**From std::vector**:
```cpp
fast_vector(const std::vector<T>& other) { assign(other.begin(), other.end()); }
fast_vector& operator=(const std::vector<T>& other)
{
    assign(other.begin(), other.end());
    return *this;
}
```

**To std::vector**:
```cpp
operator std::vector<T>() { return std::vector<T>(begin(), end()); }
```

**Benefits**:
- Seamless integration with STL code
- Easy conversion when needed
- No data copying overhead in some cases

## Integration Points

### STL Integration

- **std::vector**: Conversion to/from std::vector
- **std::unique_ptr**: Memory ownership
- **std::make_unique**: Memory allocation
- **std::exchange**: Move semantics
- **std::max**: Capacity calculation
- **memcpy**: Fast memory copying

### Falcor Integration

- **Scene**: Scene data structures
- **Rendering**: Rendering data structures
- **Utils**: Internal utilities

## Architecture Patterns

### Template Pattern

Template class for type safety:

```cpp
template<typename T>
class fast_vector
```

Benefits:
- Type-safe interface
- Compile-time optimization
- No runtime type checking

### RAII Pattern

Uses RAII for memory management:

```cpp
std::unique_ptr<T[]> m_data;
```

Benefits:
- Automatic cleanup
- Exception safety
- Clear ownership

### Move Semantics

Efficient move operations:

```cpp
fast_vector(fast_vector&& other)
    : m_capacity(std::exchange(other.m_capacity, 0)),
      m_size(std::exchange(other.m_size, 0)),
      m_data(std::move(other.m_data))
{}
```

Benefits:
- O(1) move operations
- No copying
- Efficient returning from functions

### Exponential Growth Pattern

Doubles capacity on growth:

```cpp
size_t new_size = std::max(m_capacity * 2, required_size);
```

Benefits:
- Amortized O(1) push_back
- Fewer reallocations

### Iterator Pattern

Pointer-based iterators:

```cpp
inline T* begin() { return data(); }
inline T* end() { return begin() + size(); }
```

Benefits:
- Range-based for loop support
- STL algorithm compatibility
- Zero overhead

## Code Patterns

### Static Assert Pattern

Compile-time type checking:

```cpp
static_assert(std::is_trivial_v<T>, "Fast vector can only be used on trivial types.");
```

Pattern:
- Enforces type constraints at compile time
- Descriptive error messages
- Zero runtime overhead

### Member Initialization

In-class member initialization:

```cpp
size_t m_capacity{0};
size_t m_size{0};
```

Pattern:
- Default initialization in declaration
- Clear initial values
- No constructor needed for initialization

### Exchange for Move

Uses std::exchange for move semantics:

```cpp
m_capacity = std::exchange(other.m_capacity, 0);
```

Pattern:
- Extract value and replace with new value
- Useful for move operations
- Atomic-like behavior

### Unique Pointer Array

Uses std::unique_ptr<T[]>:

```cpp
std::unique_ptr<T[]> m_data;
```

Pattern:
- Array ownership
- Automatic cleanup
- Exception-safe

### memcpy for Trivial Types

Uses memcpy for copying:

```cpp
memcpy(new_data.get(), m_data.get(), m_size * sizeof(T));
```

Pattern:
- Fast memory copy
- Safe for trivial types
- Optimized by compiler

## Use Cases

### Basic Vector Operations

```cpp
fast_vector<int> vec;
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);

// Access elements
int first = vec[0];  // 1
int last = vec[2];   // 3

// Size and capacity
size_t size = vec.size();       // 3
size_t capacity = vec.capacity(); // 4 (doubled from 2)
```

### Resizing

```cpp
fast_vector<int> vec;

// Resize without initialization
vec.resize(10);  // 10 uninitialized elements

// Resize with value
vec.resize(10, 42);  // 10 elements, all set to 42
```

### Reserving Capacity

```cpp
fast_vector<int> vec;
vec.reserve(100);  // Reserve space for 100 elements

// No reallocation until 101st element
for (int i = 0; i < 100; ++i) {
    vec.push_back(i);
}
```

### Conversion to/from std::vector

```cpp
// From std::vector
std::vector<int> stdVec = {1, 2, 3, 4, 5};
fast_vector<int> fastVec(stdVec);

// To std::vector
std::vector<int> stdVec2 = fastVec;  // Implicit conversion
```

### Iteration

```cpp
fast_vector<int> vec = {1, 2, 3, 4, 5};

// Range-based for loop
for (int& val : vec) {
    std::cout << val << " ";
}

// Iterator-based loop
for (int* it = vec.begin(); it != vec.end(); ++it) {
    std::cout << *it << " ";
}
```

### Move Semantics

```cpp
fast_vector<int> createVector() {
    fast_vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    return vec;  // Move (no copy)
}

fast_vector<int> vec = createVector();  // Move assignment
```

### Clearing

```cpp
fast_vector<int> vec = {1, 2, 3, 4, 5};

// Clear (size = 0, capacity unchanged)
vec.clear();

// Capacity still allocated
size_t capacity = vec.capacity();  // Still 8
```

## Performance Considerations

### Push Back Performance

- **Amortized O(1)**: Exponential growth ensures amortized constant time
- **No Global Lock**: Avoids MSVC's debug vector locking
- **Cache Friendly**: Sequential memory access
- **Reallocation Cost**: O(n) on reallocation (rare)

### Memory Overhead

- **Capacity vs Size**: May use up to 2x more memory than needed
- **Exponential Growth**: Typical capacity is 2^k
- **No Small Optimization**: No small vector optimization
- **Pointer Overhead**: std::unique_ptr has minimal overhead

### Copy Performance

- **Copy Constructor**: O(n) - copies all elements
- **Copy Assignment**: O(n) - copies all elements
- **Move Constructor**: O(1) - just moves pointers
- **Move Assignment**: O(1) - just moves pointers

### Iteration Performance

- **Pointer Arithmetic**: Fast pointer-based iteration
- **Cache Friendly**: Sequential memory layout
- **No Bounds Checking**: No bounds checking overhead
- **Range-Based For**: Zero overhead range-based for loop

### Comparison with std::vector

**Advantages**:
- No global lock in debug builds (MSVC)
- Faster push_back in multithreaded debug builds
- Simpler implementation (easier to understand)

**Disadvantages**:
- Trivial types only
- Minimal feature set
- No small vector optimization
- No bounds checking

**When to Use**:
- Trivial types only
- Performance-critical paths
- Multithreaded debug builds (MSVC)
- Need for lock-free push_back

**When NOT to Use**:
- Non-trivial types
- Need full std::vector features
- Single-threaded release builds
- Need bounds checking

## Limitations

### Type Limitations

- **Trivial Types Only**: Cannot store non-trivial types
- **No Polymorphism**: Cannot store polymorphic types
- **No References**: Cannot store references
- **No Pointers**: Pointers stored as values (not recommended)

### Feature Limitations

- **Minimal Interface**: Only essential vector operations
- **No Erase**: No erase() method
- **No Insert**: No insert() method
- **No Pop Back**: No pop_back() method
- **No Emplace Back**: No emplace_back() method

### Safety Limitations

- **No Bounds Checking**: operator[] has no bounds checking
- **Uninitialized Memory**: resize() leaves elements uninitialized
- **No Exception Safety**: Limited exception safety guarantees
- **Undefined Behavior**: Out-of-bounds access is UB

### Platform Limitations

- **MSVC Specific**: Designed to avoid MSVC's debug vector locking
- **No Benefit on Other Compilers**: May not benefit other compilers
- **Debug Build Only**: Benefit only in debug builds

### Performance Limitations

- **Memory Waste**: May use up to 2x more memory than needed
- **Reallocation Overhead**: O(n) reallocation cost
- **No Small Optimization**: No small vector optimization
- **Cache Misses**: Potential cache misses on large vectors

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Trivial types (int, float, math vectors)
- Performance-critical paths
- Multithreaded debug builds (MSVC)
- Need for lock-free push_back
- Simple vector operations only

**Inappropriate Use Cases**:
- Non-trivial types (std::string, std::vector)
- Need full std::vector features
- Single-threaded release builds
- Need bounds checking
- Need erase/insert operations

### Usage Patterns

**Initialization**:
```cpp
fast_vector<int> vec;
vec.reserve(expectedSize);  // Reserve if size known
```

**Push Back**:
```cpp
vec.push_back(value);  // Lock-free (MSVC debug)
```

**Resize**:
```cpp
vec.resize(size);  // Uninitialized
vec.resize(size, defaultValue);  // Initialized
```

**Clear**:
```cpp
vec.clear();  // Size = 0, capacity unchanged
```

### Performance Tips

1. **Reserve Capacity**: Reserve if size known
2. **Avoid Frequent Resizes**: Minimize resize operations
3. **Use Move Semantics**: Use move for large vectors
4. **Avoid Unnecessary Copies**: Pass by reference
5. **Use Range-Based For**: Zero overhead iteration

### Type Safety

- **Check Trivial**: Ensure type is trivial before use
- **Use Appropriate Types**: Use trivial types only
- **Avoid Type Conversions**: Avoid implicit conversions
- **Check Bounds**: Ensure indices are valid

### Memory Management

- **Reserve Capacity**: Reserve if size known
- **Clear Instead of Reallocate**: Use clear() instead of reallocation
- **Be Aware of Capacity**: Capacity may be larger than size
- **Avoid Frequent Reallocations**: Minimize reallocations

### Debugging

- **Use std::vector**: Use std::vector for debugging
- **Enable Bounds Checking**: Use std::vector for bounds checking
- **Check Capacity**: Be aware of capacity vs size
- **Check Trivial**: Ensure type is trivial

## Implementation Notes

### Trivial Type Check

Uses `std::is_trivial_v<T>`:

```cpp
static_assert(std::is_trivial_v<T>, "Fast vector can only be used on trivial types.");
```

**Trivial Type Definition**:
- Trivially default constructible
- Trivially copyable
- Trivially movable
- Trivially destructible

### Memory Allocation

Uses `std::make_unique<T[]>`:

```cpp
std::unique_ptr<T[]> new_data = std::make_unique<T[]>(new_size);
```

**Benefits**:
- Exception-safe
- Automatic cleanup
- No manual delete[]

### Copying Strategy

Uses memcpy for copying:

```cpp
memcpy(new_data.get(), m_data.get(), m_size * sizeof(T));
```

**Safety**:
- Safe for trivial types only
- Preserves bit pattern
- No type conversion

### Growth Strategy

Exponential growth with minimum:

```cpp
size_t new_size = std::max(m_capacity * 2, required_size);
```

**Characteristics**:
- Doubles capacity
- At least required_size
- Amortized O(1) push_back

### Iterator Stability

**Invalidation**:
- Insertion: Invalidates all iterators
- Reallocation: Invalidates all iterators
- Clear: Invalidates all iterators

**No Invalidation**:
- Access: No invalidation
- Size query: No invalidation

### Exception Safety

**Strong Guarantee**:
- Copy constructor: Strong guarantee
- Copy assignment: Strong guarantee
- Move constructor: No-throw guarantee
- Move assignment: No-throw guarantee

**Basic Guarantee**:
- push_back(): Basic guarantee (may throw std::bad_alloc)
- resize(): Basic guarantee (may throw std::bad_alloc)
- reserve(): Basic guarantee (may throw std::bad_alloc)

## Future Enhancements

### Potential Improvements

1. **Small Vector Optimization**: Add small buffer for small vectors
2. **Bounds Checking**: Add optional bounds checking
3. **Erase Method**: Add erase() method
4. **Pop Back Method**: Add pop_back() method
5. **Emplace Back Method**: Add emplace_back() method
6. **Insert Method**: Add insert() method

### API Extensions

1. **Back Method**: Add back() method
2. **Front Method**: Add front() method
3. **At Method**: Add at() method with bounds checking
4. **ShrinkToFit Method**: Add shrink_to_fit() method
5. **Swap Method**: Add swap() method

### Performance Enhancements

1. **Custom Allocator**: Support custom allocators
2. **Alignment**: Support custom alignment
3. **SIMD Optimization**: Optimize for SIMD operations
4. **Cache Optimization**: Optimize for cache performance
5. **Prefetching**: Add prefetching support

### Safety Enhancements

1. **Bounds Checking**: Add optional bounds checking
2. **Debug Mode**: Add debug mode with assertions
3. **Validation**: Add validation methods
4. **Sanitizers**: Add sanitizer support
5. **Testing**: Add comprehensive tests

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Comparison Guide**: Add comparison with std::vector
4. **Best Practices**: Add best practices guide
5. **Troubleshooting**: Add troubleshooting guide

## References

### C++ Standard

- **std::vector**: C++11 sequence container
- **std::unique_ptr**: C++11 smart pointer
- **std::is_trivial**: C++11 type trait
- **std::make_unique**: C++14 utility function
- **std::exchange**: C++14 utility function

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **MSVC STL**: Microsoft Visual C++ STL
- **Debug Containers**: Debug container implementations
- **Lock-Free Data Structures**: Lock-free programming
- **Performance Optimization**: Performance optimization techniques

### Memory Management

- **Dynamic Arrays**: Dynamic array allocation
- **Smart Pointers**: Smart pointer patterns
- **RAII**: Resource Acquisition Is Initialization
- **Exception Safety**: Exception safety guarantees
