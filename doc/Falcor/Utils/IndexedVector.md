# IndexedVector - Indexed Vector Container

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Utils/fast_vector
- STL (unordered_map, vector, fstd/span.h)

### Dependent Modules

- Falcor/Scene (scene data structures)
- Falcor/Rendering (rendering data structures)
- Falcor/Utils (internal utilities)

## Module Overview

IndexedVector provides a container that converts a vector of possibly-duplicate items into a vector of indices into a set of unique data items. It maintains two parallel vectors: one for unique values and one for indices into those values. This is useful for deduplicating data while preserving the original order and mapping each element to its unique index.

## Component Specifications

### IndexedVector Class

**File**: `Source/Falcor/Utils/IndexedVector.h` (100 lines)

Template class for deduplicating items and maintaining index mappings.

#### Template Parameters

```cpp
template<typename T, typename I, typename H, typename E = std::equal_to<T>>
class IndexedVector
```

**T**: Underlying data type (must be hashable)

**I**: Index value type (typically uint32_t)

**H**: Hash object on type T, used to determine data item equivalence

**E**: Equality comparator (defaults to std::equal_to<T>)

#### Member Variables

```cpp
private:
    std::unordered_map<T, I, H, E> mIndexMap;
    std::vector<T> mValues;
    std::vector<I> mIndices;
```

**mIndexMap**: Hash map mapping values to their indices

**mValues**: Vector of unique values

**mIndices**: Vector of indices (parallel to original order)

#### Public Interface

**append(v)**:
```cpp
void append(const T& v)
{
    uint32_t idx;
    append(v, idx);
}
```

**Purpose**: Appends data item (ignores index)

**Behavior**:
- Calls append(v, idx) with local index variable
- Discards the returned index
- Parameters:
  - v: Data item to append

**Usage**:
```cpp
IndexedVector<int> vec;
vec.append(42);
vec.append(42);  // Duplicate, same index
vec.append(24);
```

**append(v, outIdx)**:
```cpp
bool append(const T& v, uint32_t& outIdx)
{
    bool insertedNew = false;
    auto iter = mIndexMap.find(v);
    if (iter == mIndexMap.end())
    {
        iter = mIndexMap.insert(std::make_pair(v, I(mValues.size()))).first;
        outIdx = mValues.size();
        mValues.push_back(v);
        insertedNew = true;
    }
    else
    {
        outIdx = iter->second;
    }
    mIndices.push_back(iter->second);
    return insertedNew;
}
```

**Purpose**: Appends data item and returns index

**Behavior**:
- Searches for value in index map
- If not found:
  - Inserts value into index map with current size as index
  - Sets outIdx to current size
  - Pushes value to mValues
  - Sets insertedNew to true
- If found:
  - Sets outIdx to existing index
  - Sets insertedNew to false
- Always pushes index to mIndices
- Parameters:
  - v: Data item to append
  - outIdx: Output parameter for index
- Returns:
  - true if value was newly inserted, false if already existed

**Algorithm**:
```
1. Search for value in mIndexMap
2. If not found:
   a. Insert value with index = mValues.size()
   b. Set outIdx = mValues.size()
   c. Push value to mValues
   d. Return true
3. If found:
   a. Set outIdx = existing index
   b. Return false
4. Push index to mIndices
```

**Usage**:
```cpp
IndexedVector<int> vec;
uint32_t idx1, idx2, idx3;

bool new1 = vec.append(42, idx1);   // new1 = true, idx1 = 0
bool new2 = vec.append(42, idx2);   // new2 = false, idx2 = 0
bool new3 = vec.append(24, idx3);   // new3 = true, idx3 = 1
```

**getValues()**:
```cpp
fstd::span<const T> getValues() const { return mValues; }
```

**Purpose**: Returns span of unique data items

**Returns**: Span of unique values

**Usage**:
```cpp
IndexedVector<int> vec;
// ... append items ...

auto values = vec.getValues();
for (const auto& val : values) {
    // Process unique values
}
```

**getIndices()**:
```cpp
fstd::span<const I> getIndices() const { return mIndices; }
```

**Purpose**: Returns span of ordered item indices

**Returns**: Span of indices

**Usage**:
```cpp
IndexedVector<int> vec;
// ... append items ...

auto indices = vec.getIndices();
for (const auto& idx : indices) {
    // Process indices
}
```

## Technical Details

### Deduplication Algorithm

The [`append()`](Source/Falcor/Utils/IndexedVector.h:67) method implements deduplication:

```
1. Search for value in mIndexMap (O(1) average)
2. If not found:
   a. Insert value with index = mValues.size()
   b. Push value to mValues
   c. Mark as newly inserted
3. If found:
   a. Use existing index
   b. Mark as not newly inserted
4. Push index to mIndices (preserves order)
```

**Characteristics**:
- First occurrence gets index 0
- Subsequent duplicates get same index
- Order preserved in mIndices
- Unique values in mValues

### Hash-Based Lookup

Uses `std::unordered_map` for O(1) average lookup:

```
Value -> Hash Function -> Bucket -> Index
```

**Benefits**:
- Fast average-case lookup
- No ordering requirements
- Efficient for large datasets

**Trade-offs**:
- Hash collisions possible
- Unordered iteration
- Memory overhead for hash table

### Parallel Vector Structure

Maintains two parallel vectors:

```
mValues:  [value0, value1, value2, ...]  (unique values)
mIndices: [idx0, idx1, idx2, ...]        (all indices)
```

**Relationship**:
- mIndices[i] points to mValues[mIndices[i]]
- mIndices preserves original order
- mValues contains unique items only

**Example**:
```cpp
Input:    [A, B, A, C, B, A, D]
mValues:  [A, B, C, D]
mIndices: [0, 1, 0, 2, 1, 0, 3]
```

### Span-Based Access

Uses `fstd::span` for efficient access:

```cpp
fstd::span<const T> getValues() const { return mValues; }
fstd::span<const I> getIndices() const { return mIndices; }
```

**Benefits**:
- Zero-copy access
- Bounds information
- Compatible with range-based for
- No ownership transfer

### Template Parameters

**T**: Data type (must be hashable)

**I**: Index type (typically uint32_t)

**H**: Hash object (defaults to std::hash<T>)

**E**: Equality comparator (defaults to std::equal_to<T>)

**Default Template Arguments**:
```cpp
typename H = std::hash<T>
typename E = std::equal_to<T>
```

## Integration Points

### Falcor Utils Integration

- **fast_vector**: Fast vector implementation (included but not used)

### STL Integration

- **std::unordered_map**: Hash map for deduplication
- **std::vector**: Vector storage for values and indices
- **std::make_pair**: Pair creation for insertion
- **fstd::span**: Span-based access
- **std::equal_to**: Default equality comparator

### Internal Falcor Usage

- **Scene**: Scene data deduplication
- **Rendering**: Rendering data deduplication
- **Utils**: Internal utilities

## Architecture Patterns

### Deduplication Pattern

Automatically deduplicates items on append:

```cpp
bool append(const T& v, uint32_t& outIdx)
{
    auto iter = mIndexMap.find(v);
    if (iter == mIndexMap.end())
    {
        // Insert new value
    }
    else
    {
        // Use existing index
    }
    mIndices.push_back(iter->second);
    return insertedNew;
}
```

Benefits:
- Automatic deduplication
- Preserves order
- Efficient lookup

### Parallel Vector Pattern

Maintains parallel vectors for values and indices:

```cpp
std::vector<T> mValues;
std::vector<I> mIndices;
```

Benefits:
- Preserves order
- Efficient access
- Clear relationship

### Hash Map Pattern

Uses hash map for index lookup:

```cpp
std::unordered_map<T, I, H, E> mIndexMap;
```

Benefits:
- O(1) average lookup
- No ordering requirements
- Efficient for large datasets

### Span-Based Access Pattern

Uses span for efficient access:

```cpp
fstd::span<const T> getValues() const { return mValues; }
```

Benefits:
- Zero-copy access
- Bounds information
- Range-based for support

### Template Pattern

Template-based generic implementation:

```cpp
template<typename T, typename I, typename H, typename E = std::equal_to<T>>
class IndexedVector
```

Benefits:
- Type-safe
- Generic for any hashable type
- Compile-time optimization

## Code Patterns

### Search-Insert Pattern

Search and insert if not found:

```cpp
auto iter = mIndexMap.find(v);
if (iter == mIndexMap.end())
{
    iter = mIndexMap.insert(std::make_pair(v, I(mValues.size()))).first;
    // ...
}
```

Pattern:
- Search for value
- Insert if not found
- Use existing if found

### Index Mapping Pattern

Map values to indices:

```cpp
mIndexMap.insert(std::make_pair(v, I(mValues.size())));
```

Pattern:
- Value as key
- Index as value
- Current size as index

### Parallel Append Pattern

Append to both vectors:

```cpp
mValues.push_back(v);
mIndices.push_back(iter->second);
```

Pattern:
- Append value to mValues
- Append index to mIndices
- Preserve relationship

### Span Return Pattern

Return span for efficient access:

```cpp
fstd::span<const T> getValues() const { return mValues; }
```

Pattern:
- Zero-copy access
- Bounds information
- No ownership transfer

### Default Template Arguments

Default template arguments for flexibility:

```cpp
template<typename T, typename I, typename H, typename E = std::equal_to<T>>
```

Pattern:
- Provide sensible defaults
- Allow customization
- Maintain type safety

## Use Cases

### Basic Deduplication

```cpp
IndexedVector<int> vec;

vec.append(42);
vec.append(42);  // Duplicate
vec.append(24);
vec.append(42);  // Duplicate

// Get unique values
auto values = vec.getValues();
// values = [42, 24]

// Get indices
auto indices = vec.getIndices();
// indices = [0, 0, 1, 0]
```

### String Deduplication

```cpp
IndexedVector<std::string> vec;

vec.append("hello");
vec.append("world");
vec.append("hello");  // Duplicate

auto values = vec.getValues();
// values = ["hello", "world"]

auto indices = vec.getIndices();
// indices = [0, 1, 0]
```

### Custom Type Deduplication

```cpp
struct Item {
    int id;
    std::string name;
    bool operator==(const Item& other) const {
        return id == other.id;
    }
};

IndexedVector<Item, uint32_t, std::hash<Item>, std::equal_to<Item>> vec;

vec.append({1, "A"});
vec.append({2, "B"});
vec.append({1, "C"});  // Duplicate (same id)

auto values = vec.getValues();
// values = [{1, "A"}, {2, "B"}]

auto indices = vec.getIndices();
// indices = [0, 1, 0]
```

### Index Tracking

```cpp
IndexedVector<int> vec;

uint32_t idx1, idx2, idx3;

bool new1 = vec.append(42, idx1);   // new1 = true, idx1 = 0
bool new2 = vec.append(42, idx2);   // new2 = false, idx2 = 0
bool new3 = vec.append(24, idx3);   // new3 = true, idx3 = 1

if (new1) {
    // First occurrence of 42
}
if (!new2) {
    // Duplicate of 42
}
```

### Iteration Over Values

```cpp
IndexedVector<int> vec;
// ... append items ...

auto values = vec.getValues();
for (const auto& val : values) {
    // Process unique values
}
```

### Iteration Over Indices

```cpp
IndexedVector<int> vec;
// ... append items ...

auto indices = vec.getIndices();
for (const auto& idx : indices) {
    // Process indices
}
```

### Combined Access

```cpp
IndexedVector<int> vec;
// ... append items ...

auto values = vec.getValues();
auto indices = vec.getIndices();

for (size_t i = 0; i < indices.size(); ++i) {
    const auto& val = values[indices[i]];
    // Process value corresponding to index
}
```

## Performance Considerations

### Lookup Performance

- **Average Case**: O(1) hash-based lookup
- **Worst Case**: O(n) with hash collisions
- **Hash Function**: std::hash<T> for type T
- **Bucket Count**: Grows as number of unique values grows

### Insertion Performance

- **New Value**: O(1) average (hash lookup + insertion)
- **Duplicate**: O(1) average (hash lookup only)
- **Vector Push**: O(1) amortized
- **Index Map Insert**: O(1) average

### Memory Overhead

- **Hash Table**: Overhead for buckets
- **Vector Overhead**: Two vectors (values and indices)
- **Index Map**: Stores mapping for each unique value
- **Total Memory**: O(n) where n is number of unique values

### Iteration Performance

- **Values Iteration**: O(m) where m is number of unique values
- **Indices Iteration**: O(n) where n is total number of appends
- **Span Access**: Zero overhead
- **Cache Locality**: Depends on hash distribution

### Comparison with Alternatives

**Advantages**:
- Automatic deduplication
- Preserves order
- Efficient lookup
- Simple API

**Disadvantages**:
- Two vectors (more memory)
- Hash table overhead
- Unordered iteration of values
- No removal support

**When to Use**:
- Need deduplication
- Need order preservation
- Need index mapping
- Large datasets

**When NOT to Use**:
- Need ordered iteration of values
- Need removal operations
- Need range queries
- Need sorting

## Limitations

### Type Limitations

- **Hashable Types Only**: T must be hashable
- **Copyable Types Only**: T must be copyable
- **Equality Comparable**: T must support equality comparison
- **No References**: Cannot store references

### Feature Limitations

- **No Removal**: No erase() method
- **No Clear**: No clear() method
- **No Reserve**: No reserve() method
- **No Size Query**: No size() method (use getIndices().size())
- **No Empty Check**: No empty() method

### Thread Safety

- **Not Thread-Safe**: Not safe for concurrent access
- **No Locking**: No internal synchronization
- **External Sync**: Requires external synchronization
- **Read-Write**: Concurrent read-write not safe

### Performance Limitations

- **Hash Collisions**: Possible with poor hash function
- **Memory Overhead**: Two vectors + hash table
- **Copy Overhead**: Deep copy on copy
- **Cache Misses**: Potential cache misses with hash table

### API Limitations

- **Minimal Interface**: Only append and get methods
- **No Modification**: Cannot modify existing values
- **No Random Access**: Cannot access by index directly
- **No Range Queries**: No range-based queries

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Deduplicating data
- Preserving order
- Index mapping
- Large datasets
- Hashable types

**Inappropriate Use Cases**:
- Need ordered iteration of values
- Need removal operations
- Need range queries
- Need sorting
- Non-hashable types

### Usage Patterns

**Basic Usage**:
```cpp
IndexedVector<int> vec;
vec.append(42);
vec.append(24);
```

**Index Tracking**:
```cpp
uint32_t idx;
bool isNew = vec.append(42, idx);
```

**Accessing Values**:
```cpp
auto values = vec.getValues();
for (const auto& val : values) {
    // Process
}
```

**Accessing Indices**:
```cpp
auto indices = vec.getIndices();
for (const auto& idx : indices) {
    // Process
}
```

### Performance Tips

1. **Reserve Capacity**: Reserve if size known (not supported)
2. **Batch Appends**: Append multiple items at once (not supported)
3. **Use Span**: Use span for efficient access
4. **Avoid Copies**: Pass by reference when possible
5. **Choose Good Hash**: Use good hash function for custom types

### Type Safety

- **Hashable Types**: Ensure T is hashable
- **Equality Comparable**: Ensure T supports ==
- **Copyable Types**: Ensure T is copyable
- **Index Type**: Choose appropriate index type (uint32_t, uint64_t)

### Memory Management

- **Avoid Copies**: Minimize copying of IndexedVector
- **Use References**: Pass by reference when possible
- **Be Aware of Overhead**: Two vectors + hash table
- **Monitor Memory**: Monitor memory usage for large datasets

## Implementation Notes

### Hash Map Usage

Uses `std::unordered_map` for index mapping:

```cpp
std::unordered_map<T, I, H, E> mIndexMap;
```

**Characteristics**:
- Hash-based lookup
- Unordered iteration
- Average O(1) operations
- Custom hash and equality

### Vector Storage

Uses `std::vector` for storage:

```cpp
std::vector<T> mValues;
std::vector<I> mIndices;
```

**Characteristics**:
- Dynamic growth
- Contiguous memory
- Efficient iteration
- Amortized O(1) push_back

### Span Access

Uses `fstd::span` for access:

```cpp
fstd::span<const T> getValues() const { return mValues; }
```

**Benefits**:
- Zero-copy access
- Bounds information
- Range-based for support
- No ownership transfer

### Insertion Algorithm

Insertion algorithm:

```
1. Search for value in mIndexMap
2. If not found:
   a. Insert value with index = mValues.size()
   b. Push value to mValues
   c. Return true
3. If found:
   a. Use existing index
   b. Return false
4. Push index to mIndices
```

### Exception Safety

**Strong Guarantee**:
- append(): Strong guarantee (may throw on allocation)
- getValues(): No-throw guarantee (returns span)
- getIndices(): No-throw guarantee (returns span)

**Copy Operations**:
- Copy constructor: Strong guarantee
- Copy assignment: Strong guarantee
- May throw std::bad_alloc

## Future Enhancements

### Potential Improvements

1. **Erase Method**: Add erase() for value removal
2. **Clear Method**: Add clear() for clearing all data
3. **Reserve Method**: Add reserve() for capacity management
4. **Size Method**: Add size() method for convenience
5. **Empty Method**: Add empty() method for convenience
6. **Contains Method**: Add contains() method for checking existence

### API Extensions

1. **Random Access**: Add operator[] for direct access
2. **Range Queries**: Add range-based query methods
3. **Iteration Methods**: Add iterator support
4. **Merge Method**: Add merge() for combining IndexedVectors
5. **Sort Method**: Add sort() for sorting values

### Performance Enhancements

1. **Custom Allocator**: Support custom allocators
2. **Small Optimization**: Optimize for small datasets
3. **Cache Optimization**: Optimize for cache performance
4. **Hash Function**: Support custom hash functions
5. **Memory Pool**: Use memory pool for allocations

### Thread Safety Enhancements

1. **Concurrent Access**: Add thread-safe variant
2. **Read-Write Lock**: Add read-write locking
3. **Atomic Operations**: Use atomic operations where possible
4. **Lock-Free**: Consider lock-free implementation

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Type Safety Guide**: Add type safety guide
4. **Best Practices**: Add best practices guide
5. **Troubleshooting**: Add troubleshooting guide

## References

### C++ Standard

- **std::unordered_map**: C++11 unordered associative container
- **std::vector**: C++11 sequence container
- **std::make_pair**: C++11 utility function
- **std::equal_to**: C++11 comparator
- **fstd::span**: C++20 span (or library implementation)

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **Deduplication**: Data deduplication techniques
- **Hash Maps**: Hash-based data structures
- **Index Mapping**: Index-based data structures
- **Span**: Span-based access patterns
