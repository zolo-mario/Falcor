# Dictionary - Dictionary Data Structure

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Error (FALCOR_CHECK)
- STL (unordered_map, any, memory, string)

### Dependent Modules

- Falcor/Settings (configuration management)
- Falcor/Properties (property system)
- Falcor/Scene (scene loading configuration)
- Falcor/Rendering (render pass configuration)

## Module Overview

Dictionary provides a flexible type-safe key-value store using C++17's `std::any` for type-erased value storage. The Dictionary class wraps an unordered_map with string keys and polymorphic values, enabling storage and retrieval of any type with compile-time type safety through templates. It's used throughout Falcor for configuration management, scene loading, and render pass parameters.

## Component Specifications

### Dictionary::Value Class

**File**: `Source/Falcor/Utils/Dictionary.h` (lines 40-60)

Nested class that wraps `std::any` for type-erased value storage with convenient assignment and conversion operators.

#### Class Definition

```cpp
class Value
{
public:
    Value() = default;
    Value(std::any& value) : mValue(value){};

    template<typename T>
    void operator=(const T& t)
    {
        mValue = t;
    }

    template<typename T>
    operator T() const
    {
        return std::any_cast<T>(mValue);
    }

private:
    std::any mValue;
};
```

#### Constructors

**Default Constructor**:
```cpp
Value() = default;
```
- Creates empty Value with default-constructed `std::any`
- Contains no value until assigned

**Constructor from std::any**:
```cpp
Value(std::any& value) : mValue(value){};
```
- Initializes Value from existing `std::any`
- Copies the any value
- Parameters:
  - value: Reference to std::any to copy

#### Assignment Operator

**Template Assignment**:
```cpp
template<typename T>
void operator=(const T& t)
{
    mValue = t;
}
```

**Purpose**: Assigns any type to the Value

**Behavior**:
- Stores a copy of the provided value in `std::any`
- Type is erased at storage time
- Parameters:
  - t: Value to store (any copyable type)

**Usage**:
```cpp
Value v;
v = 42;           // Store int
v = 3.14f;        // Store float
v = "hello";       // Store const char*
v = std::string("world");  // Store std::string
```

#### Conversion Operator

**Template Conversion**:
```cpp
template<typename T>
operator T() const
{
    return std::any_cast<T>(mValue);
}
```

**Purpose**: Converts Value to specified type

**Behavior**:
- Uses `std::any_cast` to extract value
- Throws `std::bad_any_cast` if type mismatch
- Returns copy of stored value
- Returns:
  - T: Extracted value of specified type

**Usage**:
```cpp
Value v = 42;
int i = v;          // Extract int (works)
float f = v;        // Extract float (throws bad_any_cast)
```

#### Member Variable

```cpp
std::any mValue;
```

**Purpose**: Type-erased value storage

**Type**: `std::any` from C++17

**Behavior**:
- Can store any copyable type
- Type information preserved
- Throws `std::bad_any_cast` on type mismatch

### Dictionary Class

**File**: `Source/Falcor/Utils/Dictionary.h` (lines 37-100)

Main dictionary class providing key-value storage with string keys and type-erased values.

#### Type Definitions

```cpp
using Container = std::unordered_map<std::string, Value>;
```

**Purpose**: Type alias for underlying container

**Container Type**: `std::unordered_map<std::string, Value>`

**Characteristics**:
- Hash-based lookup (O(1) average)
- String keys
- Value values
- Unordered iteration

#### Constructors

**Default Constructor**:
```cpp
Dictionary() = default;
```
- Creates empty dictionary
- Default-constructed unordered_map
- No memory allocation until first insertion

**Copy Constructor**:
```cpp
Dictionary(const Dictionary& d) : mContainer(d.mContainer) {}
```
- Creates copy of dictionary
- Deep copies all key-value pairs
- Parameters:
  - d: Dictionary to copy

#### Subscript Operators

**Non-const Subscript**:
```cpp
Value& operator[](const std::string& key) { return mContainer[key]; }
```

**Purpose**: Access or create value by key

**Behavior**:
- Returns reference to Value for key
- Creates new Value if key doesn't exist
- Allows modification of existing values
- Parameters:
  - key: String key to access
- Returns:
  - Reference to Value (can be assigned to)

**Usage**:
```cpp
Dictionary dict;
dict["name"] = "John";      // Create key
dict["age"] = 30;            // Create key
dict["age"] = 31;            // Modify existing key
```

**const Subscript**:
```cpp
const Value& operator[](const std::string& key) const { return mContainer.at(key); }
```

**Purpose**: Read-only access to value by key

**Behavior**:
- Returns const reference to Value for key
- Throws `std::out_of_range` if key doesn't exist
- Does not create new entries
- Parameters:
  - key: String key to access
- Returns:
  - Const reference to Value

**Usage**:
```cpp
const Dictionary& dict = ...;
const Value& name = dict["name"];  // Read access
// dict["new_key"] = value;  // Compilation error (const)
```

#### Iterator Methods

**const_iterator begin()**:
```cpp
Container::const_iterator begin() const { return mContainer.begin(); }
```

**Purpose**: Returns const iterator to first element

**Returns**: Const iterator to beginning of container

**const_iterator end()**:
```cpp
Container::const_iterator end() const { return mContainer.end(); }
```

**Purpose**: Returns const iterator to past-the-end element

**Returns**: Const iterator to end of container

**iterator begin()**:
```cpp
Container::iterator begin() { return mContainer.begin(); }
```

**Purpose**: Returns iterator to first element

**Returns**: Iterator to beginning of container

**iterator end()**:
```cpp
Container::iterator end() { return mContainer.end(); }
```

**Purpose**: Returns iterator to past-the-end element

**Returns**: Iterator to end of container

**Usage**:
```cpp
Dictionary dict;
// ... populate dict ...

// Range-based for loop (uses begin/end)
for (const auto& [key, value] : dict) {
    // Process key-value pair
}

// Iterator-based loop
for (auto it = dict.begin(); it != dict.end(); ++it) {
    const std::string& key = it->first;
    const Value& value = it->second;
}
```

#### Size Query

**size()**:
```cpp
size_t size() const { return mContainer.size(); }
```

**Purpose**: Returns number of key-value pairs

**Returns**: Number of elements in dictionary

**Usage**:
```cpp
Dictionary dict;
dict["a"] = 1;
dict["b"] = 2;
size_t count = dict.size();  // count = 2
```

#### Key Existence Check

**keyExists()**:
```cpp
bool keyExists(const std::string& key) const { return mContainer.find(key) != mContainer.end(); }
```

**Purpose**: Checks if key exists in dictionary

**Behavior**:
- Uses find() to search for key
- Returns true if found, false otherwise
- Does not throw exceptions
- Parameters:
  - key: String key to check
- Returns:
  - true if key exists, false otherwise

**Usage**:
```cpp
Dictionary dict;
dict["name"] = "John";

if (dict.keyExists("name")) {
    // Key exists
}

if (!dict.keyExists("age")) {
    // Key doesn't exist
}
```

#### Value Retrieval

**getValue<T>(key)**:
```cpp
template<typename T>
T getValue(const std::string& key)
{
    auto it = mContainer.find(key);
    FALCOR_CHECK(it != mContainer.end(), "Key '{}' does not exist", key);
    return it->second;
}
```

**Purpose**: Get value by key with type conversion

**Behavior**:
- Searches for key in dictionary
- Throws exception if key doesn't exist
- Returns Value converted to specified type
- Uses Value's conversion operator
- Parameters:
  - key: String key to retrieve
- Returns:
  - Value converted to type T

**Exceptions**:
- Throws exception if key doesn't exist
- Throws `std::bad_any_cast` if type mismatch

**Usage**:
```cpp
Dictionary dict;
dict["name"] = "John";
dict["age"] = 30;

std::string name = dict.getValue<std::string>("name");  // "John"
int age = dict.getValue<int>("age");                  // 30
// dict.getValue<int>("missing");  // Throws exception
```

**getValue<T>(key, defaultValue)**:
```cpp
template<typename T>
T getValue(const std::string& key, const T& defaultValue)
{
    auto it = mContainer.find(key);
    return it != mContainer.end() ? it->second : defaultValue;
}
```

**Purpose**: Get value by key with default value

**Behavior**:
- Searches for key in dictionary
- Returns value if key exists
- Returns default value if key doesn't exist
- Does not throw exceptions
- Parameters:
  - key: String key to retrieve
  - defaultValue: Value to return if key doesn't exist
- Returns:
  - Value converted to type T (actual value or default)

**Usage**:
```cpp
Dictionary dict;
dict["name"] = "John";

std::string name = dict.getValue<std::string>("name", "Unknown");  // "John"
std::string age = dict.getValue<std::string>("age", "30");      // "30" (default)
```

#### Member Variable

```cpp
Container mContainer;
```

**Purpose**: Underlying unordered_map storage

**Type**: `std::unordered_map<std::string, Value>`

**Behavior**:
- Stores all key-value pairs
- Hash-based lookup
- Unordered iteration

## Technical Details

### Type Erasure with std::any

The Dictionary uses C++17's `std::any` for type-erased storage:

**Storage**:
```cpp
std::any mValue;
mValue = 42;           // Store int
mValue = 3.14f;        // Store float
mValue = "hello";       // Store const char*
```

**Retrieval**:
```cpp
int i = std::any_cast<int>(mValue);      // Extract int
float f = std::any_cast<float>(mValue);  // Extract float
```

**Type Safety**:
```cpp
std::any a = 42;
int i = std::any_cast<int>(a);      // Works
float f = std::any_cast<float>(a);  // Throws std::bad_any_cast
```

### Hash-Based Lookup

The underlying `std::unordered_map` provides O(1) average lookup:

```
Key -> Hash Function -> Bucket -> Value
```

**Advantages**:
- Fast average-case lookup
- No ordering requirements
- Efficient for large dictionaries

**Disadvantages**:
- Unordered iteration
- Hash collisions possible
- Memory overhead for hash table

### Value Conversion

Value conversion uses `std::any_cast`:

```cpp
template<typename T>
operator T() const
{
    return std::any_cast<T>(mValue);
}
```

**Behavior**:
- Checks stored type at runtime
- Throws `std::bad_any_cast` if type mismatch
- Returns copy of stored value

**Type Matching**:
```cpp
Value v = 42;
int i = v;           // Works (exact type match)
long l = v;          // Works (implicit conversion)
float f = v;         // Throws (type mismatch)
```

### Exception Handling

The Dictionary uses `FALCOR_CHECK` for validation:

```cpp
template<typename T>
T getValue(const std::string& key)
{
    auto it = mContainer.find(key);
    FALCOR_CHECK(it != mContainer.end(), "Key '{}' does not exist", key);
    return it->second;
}
```

**Behavior**:
- Throws exception if key doesn't exist
- Provides descriptive error message
- Includes key name in error message

## Integration Points

### Falcor Core Integration

- **Error**: FALCOR_CHECK macro for validation
- **Exception Handling**: Falcor exception types

### STL Integration

- **std::unordered_map**: Underlying container
- **std::any**: Type-erased value storage
- **std::string**: Key type
- **std::bad_any_cast**: Type mismatch exception

### Internal Falcor Usage

- **Settings**: Configuration management
- **Properties**: Property system
- **Scene**: Scene loading configuration
- **Rendering**: Render pass configuration
- **RenderGraph**: Render graph parameters

## Architecture Patterns

### Type Erasure Pattern

Uses `std::any` for type-erased storage:

```cpp
class Value
{
    std::any mValue;  // Type-erased storage
};
```

Benefits:
- Store any type
- Type-safe access
- Compile-time type checking

### Template Method Pattern

Template methods for type-safe access:

```cpp
template<typename T>
T getValue(const std::string& key)
{
    return it->second;  // Uses Value's conversion operator
}
```

Benefits:
- Type-safe access
- Compile-time checking
- No runtime type information

### RAII Pattern

Uses RAII for resource management:

```cpp
std::unordered_map<std::string, Value> mContainer;
```

Benefits:
- Automatic cleanup
- Exception safety
- Clear ownership

### Iterator Pattern

Provides STL-compatible iterators:

```cpp
Container::iterator begin() { return mContainer.begin(); }
Container::iterator end() { return mContainer.end(); }
```

Benefits:
- Range-based for loop support
- STL algorithm compatibility
- Consistent interface

### Default Value Pattern

Provides default value overload:

```cpp
T getValue(const std::string& key, const T& defaultValue)
{
    return it != mContainer.end() ? it->second : defaultValue;
}
```

Benefits:
- No exception throwing
- Graceful handling of missing keys
- Explicit default values

## Code Patterns

### Subscript Operator Pattern

Provides both const and non-const subscript:

```cpp
Value& operator[](const std::string& key) { return mContainer[key]; }
const Value& operator[](const std::string& key) const { return mContainer.at(key); }
```

Pattern:
- Non-const: Creates entries if missing
- const: Throws if missing
- Consistent interface

### Template Conversion Pattern

Value uses template conversion operator:

```cpp
template<typename T>
operator T() const
{
    return std::any_cast<T>(mValue);
}
```

Pattern:
- Implicit conversion to any type
- Runtime type checking
- Exception on mismatch

### Key Existence Check Pattern

Uses find() for key existence:

```cpp
bool keyExists(const std::string& key) const
{
    return mContainer.find(key) != mContainer.end();
}
```

Pattern:
- Efficient O(1) lookup
- No exception throwing
- Boolean result

### Validation Pattern

Uses FALCOR_CHECK for validation:

```cpp
FALCOR_CHECK(it != mContainer.end(), "Key '{}' does not exist", key);
```

Pattern:
- Descriptive error messages
- Exception throwing
- Format string support

## Use Cases

### Configuration Management

```cpp
Dictionary config;
config["width"] = 1920;
config["height"] = 1080;
config["fullscreen"] = true;

int width = config.getValue<int>("width");
int height = config.getValue<int>("height");
bool fullscreen = config.getValue<bool>("fullscreen");
```

### Scene Loading

```cpp
Dictionary sceneData;
sceneData["file"] = "scene.pyscene";
sceneData["scale"] = 1.0f;
sceneData["animate"] = true;

std::string file = sceneData.getValue<std::string>("file");
float scale = sceneData.getValue<float>("scale");
bool animate = sceneData.getValue<bool>("animate");
```

### Render Pass Parameters

```cpp
Dictionary params;
params["samples"] = 4;
params["bounces"] = 3;
params["denoise"] = true;

int samples = params.getValue<int>("samples");
int bounces = params.getValue<int>("bounces");
bool denoise = params.getValue<bool>("denoise");
```

### Default Values

```cpp
Dictionary settings;

// Use default if key doesn't exist
int width = settings.getValue<int>("width", 1920);
int height = settings.getValue<int>("height", 1080);
bool fullscreen = settings.getValue<bool>("fullscreen", false);
```

### Iteration

```cpp
Dictionary dict;
dict["name"] = "John";
dict["age"] = 30;

// Range-based for loop
for (const auto& [key, value] : dict) {
    std::cout << key << ": " << value << std::endl;
}

// Iterator-based loop
for (auto it = dict.begin(); it != dict.end(); ++it) {
    const std::string& key = it->first;
    const Value& value = it->second;
}
```

### Key Existence Check

```cpp
Dictionary dict;
dict["name"] = "John";

if (dict.keyExists("name")) {
    std::string name = dict.getValue<std::string>("name");
}

if (!dict.keyExists("age")) {
    dict["age"] = 30;
}
```

### Type Conversion

```cpp
Dictionary dict;
dict["value"] = 42;

// Extract as int
int i = dict["value"];  // 42

// Extract as long (implicit conversion)
long l = dict["value"];  // 42

// Extract as string (would throw)
// std::string s = dict["value"];  // std::bad_any_cast
```

## Performance Considerations

### Lookup Performance

- **Average Case**: O(1) hash-based lookup
- **Worst Case**: O(n) with hash collisions
- **Hash Function**: std::hash for std::string
- **Bucket Count**: Grows as dictionary grows

### Memory Overhead

- **Hash Table**: Overhead for buckets
- **std::any**: Small object optimization (typically 3 words)
- **String Keys**: String allocation overhead
- **Load Factor**: Typically 1.0 (rehash at 100%)

### Copy Performance

- **Copy Constructor**: O(n) deep copy
- **Copy Assignment**: O(n) deep copy
- **Value Copy**: O(1) for small objects, O(n) for large
- **String Copy**: O(n) for string keys

### Iteration Performance

- **Unordered**: No specific iteration order
- **Cache Locality**: Depends on hash distribution
- **Iterator Overhead**: Minimal (pointer arithmetic)

### Optimization Tips

1. **Reserve Capacity**: Reserve space for expected size
2. **Use String Views**: Consider string_view for temporary keys
3. **Avoid Copies**: Use references when possible
4. **Cache Lookups**: Store frequently accessed values
5. **Use Default Values**: Avoid exception throwing

## Limitations

### Type Limitations

- **Copyable Types Only**: Only copyable types can be stored
- **No References**: Cannot store references
- **No Pointers**: Pointers stored as values (not recommended)
- **Type Safety**: Runtime type checking only

### Thread Safety

- **Not Thread-Safe**: Not safe for concurrent access
- **No Locking**: No internal synchronization
- **External Sync**: Requires external synchronization
- **Read-Write**: Concurrent read-write not safe

### Performance Limitations

- **Hash Collisions**: Possible with poor hash function
- **Memory Overhead**: Hash table overhead
- **Copy Overhead**: Deep copy on dictionary copy
- **Type Erasure**: std::any has overhead

### API Limitations

- **No Removal**: No erase() method
- **No Clear**: No clear() method
- **No Reserve**: No reserve() method
- **No Rehash**: No rehash() method

### Exception Safety

- **Type Mismatch**: Throws std::bad_any_cast
- **Missing Key**: Throws exception (getValue without default)
- **Memory Allocation**: May throw std::bad_alloc
- **Hash Function**: May throw (unlikely)

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Configuration management
- Scene loading parameters
- Render pass settings
- Property storage
- Key-value data with heterogeneous types

**Inappropriate Use Cases**:
- Performance-critical data structures
- Large datasets (use specialized containers)
- Thread-safe access (use concurrent map)
- Ordered iteration (use std::map)

### Usage Patterns

**Initialization**:
```cpp
Dictionary config;
config["width"] = 1920;
config["height"] = 1080;
```

**Access with Default**:
```cpp
int width = config.getValue<int>("width", 1920);
```

**Check Key Existence**:
```cpp
if (config.keyExists("width")) {
    // Key exists
}
```

**Iteration**:
```cpp
for (const auto& [key, value] : config) {
    // Process
}
```

### Error Handling

- **Use Default Values**: Prefer default values over exceptions
- **Check Key Existence**: Check before accessing if unsure
- **Catch Exceptions**: Catch std::bad_any_cast for type errors
- **Validate Types**: Ensure type correctness

### Performance Tips

1. **Use Default Values**: Avoid exception throwing
2. **Cache Values**: Store frequently accessed values
3. **Avoid Copies**: Use references when possible
4. **Use const**: Use const access when possible
5. **Reserve Capacity**: Reserve space if size known

### Type Safety

- **Explicit Types**: Always specify type in getValue<T>()
- **Type Consistency**: Use consistent types for keys
- **Avoid Implicit Conversion**: Be careful with implicit conversions
- **Check Types**: Use keyExists() before accessing

### Memory Management

- **Avoid Large Objects**: Store pointers to large objects
- **Use Smart Pointers**: Store shared_ptr for shared ownership
- **Avoid Deep Copies**: Minimize dictionary copying
- **Use References**: Pass by reference when possible

## Implementation Notes

### std::any Behavior

**Small Object Optimization**:
- Small objects stored inline (typically 3 words)
- Large objects stored on heap
- No allocation for small types

**Type Information**:
- Type information preserved
- Runtime type checking
- Type-safe extraction

**Copy Semantics**:
- Deep copy on copy
- Move on move
- Reference semantics for references

### Hash Function

**std::hash<std::string>**:
- Uses standard string hash function
- Good distribution for typical keys
- Deterministic (same key = same hash)

**Hash Collisions**:
- Possible but unlikely
- Handled by unordered_map
- Linear probing within bucket

### Iterator Stability

**Insertion**: Invalidates iterators
**Deletion**: Invalidates iterators
**Rehash**: Invalidates iterators
**No Iterator Invalidation**: No erase() method

### Exception Safety

**Strong Guarantee**:
- getValue<T>(key): Strong guarantee (throws if missing)
- getValue<T>(key, defaultValue): No-throw guarantee
- operator[]: Strong guarantee (creates if missing)
- keyExists(): No-throw guarantee

**Copy Operations**:
- Copy constructor: Strong guarantee
- Copy assignment: Strong guarantee
- May throw std::bad_alloc

## Future Enhancements

### Potential Improvements

1. **Erase Method**: Add erase() for key removal
2. **Clear Method**: Add clear() for clearing dictionary
3. **Reserve Method**: Add reserve() for capacity management
4. **Merge Method**: Add merge() for combining dictionaries
5. **Contains Method**: Add contains() as alias for keyExists()
6. **TryGetValue Method**: Add tryGetValue() for exception-free access

### API Extensions

1. **Range-Based For**: Already supported via iterators
2. **Structured Binding**: Already supported via iterators
3. **JSON Serialization**: Add to/from JSON methods
4. **Binary Serialization**: Add to/from binary methods
5. **Type Inspection**: Add type inspection methods

### Performance Enhancements

1. **Reserve Support**: Add reserve() method
2. **Custom Allocator**: Support custom allocators
3. **Move Semantics**: Improve move operations
4. **Small Dictionary Optimization**: Optimize for small dictionaries
5. **String Interning**: Intern string keys for efficiency

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

## References

### C++ Standard

- **std::any**: C++17 type-erased storage
- **std::unordered_map**: C++11 unordered associative container
- **std::string**: C++11 string class
- **std::bad_any_cast**: Exception for type mismatch

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Settings**: Settings module documentation
- **Falcor Properties**: Properties module documentation

### Related Technologies

- **JSON**: JavaScript Object Notation (similar structure)
- **Property Trees**: Boost Property Tree
- **Configuration Files**: Various configuration formats
- **Type Erasure**: Type erasure patterns in C++
