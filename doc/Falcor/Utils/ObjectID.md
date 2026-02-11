# ObjectID - Strongly Typed Object Identification

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Error (FALCOR_ASSERT_GE, FALCOR_ASSERT_LE, FALCOR_ASSERT_LT)
- fmt/format.h (fmt::format_to)
- std::limits
- std::functional
- std::type_traits

### Dependent Modules

- Falcor/Scene (scene object identification)
- Falcor/Rendering (rendering object identification)
- Falcor/Utils (internal utilities)

## Module Overview

ObjectID provides a universal class for strongly typed IDs that allows usage for IDs in unrelated subsystems without polluting a single enum with unrelated kinds. It's a template class that takes a kind enum, kind value, and optional integer type to create type-safe object identifiers. The class provides invalid ID tracking, validation, conversion between kinds, and integration with standard library containers (hash, fmt formatting).

## Component Specifications

### ObjectID Class

**File**: `Source/Falcor/Utils/ObjectID.h` (lines 49-171)

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType = uint32_t>
class ObjectID
{
    // ... implementation ...
};
```

**Purpose**: Universal class for strongly typed IDs

**Template Parameters**:
- **TKindEnum**: Enum class from which kinds are drawn. Different enum classes are not directly convertible.
- **TKind**: Kind of the ID
- **TIntType**: Underlying numeric type (default: uint32_t). It is advised that it should be same for all TKinds in the same enum.

**Type Definitions**:
```cpp
using IntType = TIntType;
using KindEnum = TKindEnum;
static constexpr TKindEnum kKind = TKind;
static constexpr IntType kInvalidID = std::numeric_limits<IntType>::max();
```

**IntType**: Underlying integer type

**KindEnum**: Kind enum type

**kKind**: Kind value (static constexpr)

**kInvalidID**: Invalid ID value (max value of IntType)

#### Default Constructor

```cpp
ObjectID() : mID(kInvalidID) {}
```

**Purpose**: Default construction creates an invalid ID

**Behavior**:
- Initializes mID to kInvalidID
- Creates invalid ID by default
- TODO: Consider creating uninitialized ID instead, if vectors of IDs become a performance issue

#### Numeric Constructor

```cpp
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
explicit ObjectID(const T& id) : mID(IntType(id))
{
    FALCOR_ASSERT_GE(id, T(0));
    FALCOR_ASSERT_LE(std::make_unsigned_t<T>(id), std::numeric_limits<IntType>::max());
}
```

**Purpose**: Constructs ObjectID from any numeric type

**Template Parameters**:
- **T**: Integral type (int, uint, etc.)

**Parameters**:
- id: Integer ID to initialize from

**Behavior**:
- Validates that id >= 0
- Validates that id <= max value of IntType
- Initializes mID to IntType(id)
- Checks for validity of the ID with respect to the allowed range

**Validation**:
1. First we make sure it is positive
2. When we know for a fact it is not negative, we can cast it to the unsigned version of that integer for comparison (otherwise compiler complains about signed/unsigned mismatch when entering literals)

#### Kind Conversion Constructor

```cpp
template<TKindEnum TOtherKind>
explicit ObjectID(const ObjectID<TKindEnum, TOtherKind, TIntType>& other)
{
    mID = other.get();
}
```

**Purpose**: Allows converting between different Kinds of the same EnumKind

**Template Parameters**:
- **TOtherKind**: Other kind to convert from

**Parameters**:
- other: The ObjectID to be converted from

**Behavior**:
- Copies mID from other ObjectID
- Converts between different kinds of same enum
- Slightly safer than going straight through numeric ids via get()
- Mostly used when converting from an "union" ID, that can identify different objects based on other flags, e.g., kCurveOrMesh that is either Curve or Mesh, based on the tessellation flags
- NB: Ideally this would be removed, use as sparingly as possible

#### fromSlang Static Method

```cpp
template<typename T>
static ObjectID fromSlang(const T& id, std::enable_if_t<std::is_integral_v<T>, bool> = true)
{
    return ObjectID{id};
}
```

**Purpose**: A helper method when converting from an numeric ID in Slang, to the strongly typed CPU ID

**Template Parameters**:
- **T**: Integral type

**Parameters**:
- id: Integer ID to initialize from

**Returns**: The ObjectID created from a numeric ID

**Behavior**:
- Creates ObjectID from numeric ID
- Separate from a basic constructor only for purpose of clearly identifying conversion in code, as per Joel's "Making Wrong Code Look Wrong" principle
- TODO: Remove once slang side also has strongly typed IDs

#### Invalid Static Method

```cpp
static ObjectID Invalid() { return ObjectID(); }
```

**Purpose**: Provides an invalid ID for comparison purposes

**Returns**: An invalid ObjectID

**Behavior**:
- Returns default-constructed ObjectID (invalid)
- In future, most uses would be replaced by either isValid (for comparison), or by ObjectID(ObjectID::kInvalidID) (for obtaining an invalid ID)

#### isValid Method

```cpp
bool isValid() const { return mID != kInvalidID; }
```

**Purpose**: Returns true when ID is valid

**Returns**: True when valid (mID != kInvalidID)

**Behavior**:
- Checks if mID != kInvalidID
- Returns true if valid, false otherwise

#### get Method

```cpp
IntType get() const { return mID; }
```

**Purpose**: Return numeric value of ID

**Returns**: Numeric value of ID, can be kInvalidID

**Behavior**:
- Returns mID
- Should be used rather sparingly, e.g., consider allowing objects to be indexed by strongly typed ID, instead of just a number
- NB: Consider using getters with strongly typed IDs, rather than directly accessing even vectors/buffers

#### getSlang Method

```cpp
IntType getSlang() const { return get(); }
```

**Purpose**: A helper method to convert to numeric ID in Slang

**Returns**: Numeric value of ID, can be kInvalidID

**Behavior**:
- Functionally identical to get()
- In future it should be removed, and Slang should have a compatible and checked strongly typed ID as well
- Separated from get() to clearly show all such locations

#### Comparison Operators

```cpp
bool operator==(const ObjectID& rhs) const { return mID == rhs.mID; }
bool operator!=(const ObjectID& rhs) const { return mID != rhs.mID; }
bool operator<=(const ObjectID& rhs) const { return mID <= rhs.mID; }
bool operator>=(const ObjectID& rhs) const { return mID >= rhs.mID; }
bool operator<(const ObjectID& rhs) const { return mID < rhs.mID; }
bool operator>(const ObjectID& rhs) const { return mID > rhs.mID; }
```

**Purpose**: Comparison operators

**Behavior**:
- Compare mID with rhs.mID
- Full set of comparison operators
- Allow sorting and ordering

#### Increment Operators

```cpp
ObjectID& operator++()
{
    FALCOR_ASSERT_LT(mID, std::numeric_limits<IntType>::max());
    ++mID;
    return *this;
}

ObjectID operator++(int) { return ObjectID(mID++); }
```

**Purpose**: Increment operators

**Pre-Increment**:
- Asserts that mID < max value
- Increments mID
- Returns reference to *this

**Post-Increment**:
- Returns ObjectID with current value
- Increments mID
- Returns old value

#### Private Member

```cpp
IntType mID;
```

**mID**: Numeric ID value

### to_string Function

**File**: `Source/Falcor/Utils/ObjectID.h` (lines 173-177)

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
inline std::string to_string(const ObjectID<TKindEnum, TKind, TIntType>& v)
{
    return std::to_string(v.get());
}
```

**Purpose**: Convert ObjectID to string

**Returns**: String representation of ObjectID

**Behavior**:
- Calls v.get() to get numeric value
- Converts to string using std::to_string
- Returns string representation

### std::hash Specialization

**File**: `Source/Falcor/Utils/ObjectID.h` (lines 181-186)

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct std::hash<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    using ObjectID = Falcor::ObjectID<TKindEnum, TKind, TIntType>;
    std::size_t operator()(const ObjectID& id) const noexcept { return std::hash<typename ObjectID::IntType>{}(id.get()); }
};
```

**Purpose**: std::hash specialization for ObjectID

**Behavior**:
- Hashes ObjectID by hashing its IntType value
- Allows ObjectID to be used in unordered containers
- noexcept guaranteed

### fmt::formatter Specialization

**File**: `Source/Falcor/Utils/ObjectID.h` (lines 188-204)

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct fmt::formatter<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    using ObjectID = Falcor::ObjectID<TKindEnum, TKind, TIntType>;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const ObjectID& id, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{0}", id.get());
    }
};
```

**Purpose**: fmt::formatter specialization for ObjectID

**Behavior**:
- Parses format string (returns ctx.begin())
- Formats ObjectID by formatting its IntType value
- Allows ObjectID to be used with fmt library

## Technical Details

### Strong Typing

ObjectID provides strong typing through template parameters:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType = uint32_t>
class ObjectID
{
    // ... implementation ...
};
```

**Characteristics**:
- Different enum classes are not directly convertible
- Type-safe object identification
- Compile-time type checking
- Prevents accidental mixing of ID types

### Kind Separation

Uses enum classes to distinguish ID kinds:

```cpp
enum class ObjectKind { Mesh, Curve, Light, Camera, ... };
using MeshID = ObjectID<ObjectKind, ObjectKind::Mesh>;
using CurveID = ObjectID<ObjectKind, ObjectKind::Curve>;
```

**Characteristics**:
- Different kinds are not directly convertible
- Clear separation of ID types
- Type-safe operations
- Prevents accidental mixing

### Invalid ID Tracking

Supports invalid ID detection:

```cpp
static constexpr IntType kInvalidID = std::numeric_limits<IntType>::max();

ObjectID() : mID(kInvalidID) {}
bool isValid() const { return mID != kInvalidID; }
```

**Characteristics**:
- Invalid ID is max value of IntType
- Default constructor creates invalid ID
- isValid() method checks validity
- Invalid() static method returns invalid ID

### Validation

Validates ID values on construction:

```cpp
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
explicit ObjectID(const T& id) : mID(IntType(id))
{
    FALCOR_ASSERT_GE(id, T(0));
    FALCOR_ASSERT_LE(std::make_unsigned_t<T>(id), std::numeric_limits<IntType>::max());
}
```

**Characteristics**:
- Validates that id >= 0
- Validates that id <= max value of IntType
- Uses FALCOR_ASSERT macros for validation
- Prevents invalid ID values

### Kind Conversion

Allows conversion between different kinds of same enum:

```cpp
template<TKindEnum TOtherKind>
explicit ObjectID(const ObjectID<TKindEnum, TOtherKind, TIntType>& other)
{
    mID = other.get();
}
```

**Characteristics**:
- Explicit conversion (not implicit)
- Only works for same enum kind
- Copies numeric value
- Slightly safer than direct numeric conversion

### Slang Integration

Provides conversion to/from Slang numeric IDs:

```cpp
template<typename T>
static ObjectID fromSlang(const T& id, std::enable_if_t<std::is_integral_v<T>, bool> = true)
{
    return ObjectID{id};
}

IntType getSlang() const { return get(); }
```

**Characteristics**:
- fromSlang() creates ObjectID from numeric ID
- getSlang() returns numeric ID for Slang
- Separate methods for clarity
- TODO: Remove once Slang has strongly typed IDs

### Hash Support

Integrates with std::hash for unordered containers:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct std::hash<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    using ObjectID = Falcor::ObjectID<TKindEnum, TKind, TIntType>;
    std::size_t operator()(const ObjectID& id) const noexcept { return std::hash<typename ObjectID::IntType>{}(id.get()); }
};
```

**Characteristics**:
- Hashes ObjectID by hashing its IntType value
- Allows ObjectID to be used in unordered containers
- noexcept guaranteed
- Standard library integration

### Format Support

Integrates with fmt for string formatting:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct fmt::formatter<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    template<typename FormatContext>
    auto format(const ObjectID& id, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{0}", id.get());
    }
};
```

**Characteristics**:
- Formats ObjectID by formatting its IntType value
- Allows ObjectID to be used with fmt library
- Standard library integration
- Easy string conversion

### Comparison Operators

Full set of comparison operators:

```cpp
bool operator==(const ObjectID& rhs) const { return mID == rhs.mID; }
bool operator!=(const ObjectID& rhs) const { return mID != rhs.mID; }
bool operator<=(const ObjectID& rhs) const { return mID <= rhs.mID; }
bool operator>=(const ObjectID& rhs) const { return mID >= rhs.mID; }
bool operator<(const ObjectID& rhs) const { return mID < rhs.mID; }
bool operator>(const ObjectID& rhs) const { return mID > rhs.mID; }
```

**Characteristics**:
- Compare mID with rhs.mID
- Full set of comparison operators
- Allow sorting and ordering
- Consistent with IntType comparison

### Increment Operators

Pre- and post-increment operators:

```cpp
ObjectID& operator++()
{
    FALCOR_ASSERT_LT(mID, std::numeric_limits<IntType>::max());
    ++mID;
    return *this;
}

ObjectID operator++(int) { return ObjectID(mID++); }
```

**Characteristics**:
- Pre-increment increments and returns reference
- Post-increment returns old value and increments
- Asserts that mID < max value (pre-increment)
- Allows ID iteration

### Memory Layout

Memory layout of ObjectID:

```
ObjectID<TKindEnum, TKind, TIntType>:
    mID: TIntType (numeric ID value)
    Total size: sizeof(TIntType)
```

**Characteristics**:
- Simple POD type
- No dynamic allocation
- No virtual functions
- Minimal memory overhead

## Integration Points

### Falcor Core Integration

- **Core/Error**: FALCOR_ASSERT_GE, FALCOR_ASSERT_LE, FALCOR_ASSERT_LT

### STL Integration

- **std::limits**: For kInvalidID value
- **std::functional**: For std::hash specialization
- **std::type_traits**: For std::enable_if_t, std::is_integral_v, std::make_unsigned_t
- **fmt/format.h**: For fmt::formatter specialization

### Internal Falcor Usage

- **Scene**: Scene object identification
- **Rendering**: Rendering object identification
- **Utils**: Internal utilities

## Architecture Patterns

### Strong Typing Pattern

Use template parameters for strong typing:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType = uint32_t>
class ObjectID
{
    // ... implementation ...
};
```

Benefits:
- Type-safe object identification
- Compile-time type checking
- Prevents accidental mixing of ID types

### Kind Separation Pattern

Use enum classes to distinguish ID kinds:

```cpp
enum class ObjectKind { Mesh, Curve, Light, Camera, ... };
using MeshID = ObjectID<ObjectKind, ObjectKind::Mesh>;
using CurveID = ObjectID<ObjectKind, ObjectKind::Curve>;
```

Benefits:
- Different kinds are not directly convertible
- Clear separation of ID types
- Type-safe operations

### Invalid ID Pattern

Support invalid ID detection:

```cpp
static constexpr IntType kInvalidID = std::numeric_limits<IntType>::max();

ObjectID() : mID(kInvalidID) {}
bool isValid() const { return mID != kInvalidID; }
```

Benefits:
- Clear invalid ID representation
- Easy validity checking
- Default invalid ID

### Validation Pattern

Validate input parameters:

```cpp
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
explicit ObjectID(const T& id) : mID(IntType(id))
{
    FALCOR_ASSERT_GE(id, T(0));
    FALCOR_ASSERT_LE(std::make_unsigned_t<T>(id), std::numeric_limits<IntType>::max());
}
```

Benefits:
- Early error detection
- Clear error messages
- Prevents invalid ID values

### Standard Library Integration Pattern

Integrate with standard library:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct std::hash<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    std::size_t operator()(const ObjectID& id) const noexcept { return std::hash<typename ObjectID::IntType>{}(id.get()); }
};
```

Benefits:
- Works with standard library containers
- No custom container implementation needed
- Seamless integration

## Code Patterns

### Strong Typing Pattern

Use template parameters for strong typing:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType = uint32_t>
class ObjectID
{
    // ... implementation ...
};
```

Pattern:
- Template parameters define type
- Different enum classes are not directly convertible
- Compile-time type checking

### Kind Separation Pattern

Use enum classes to distinguish ID kinds:

```cpp
enum class ObjectKind { Mesh, Curve, Light, Camera, ... };
using MeshID = ObjectID<ObjectKind, ObjectKind::Mesh>;
using CurveID = ObjectID<ObjectKind, ObjectKind::Curve>;
```

Pattern:
- Define enum class for kinds
- Create type aliases for each kind
- Different kinds are not directly convertible

### Invalid ID Pattern

Support invalid ID detection:

```cpp
static constexpr IntType kInvalidID = std::numeric_limits<IntType>::max();

ObjectID() : mID(kInvalidID) {}
bool isValid() const { return mID != kInvalidID; }
```

Pattern:
- Define invalid ID value
- Default constructor creates invalid ID
- isValid() method checks validity

### Validation Pattern

Validate input parameters:

```cpp
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
explicit ObjectID(const T& id) : mID(IntType(id))
{
    FALCOR_ASSERT_GE(id, T(0));
    FALCOR_ASSERT_LE(std::make_unsigned_t<T>(id), std::numeric_limits<IntType>::max());
}
```

Pattern:
- Use SFINAE to restrict template instantiation
- Validate input parameters
- Use FALCOR_ASSERT macros

### Standard Library Integration Pattern

Integrate with standard library:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct std::hash<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    std::size_t operator()(const ObjectID& id) const noexcept { return std::hash<typename ObjectID::IntType>{}(id.get()); }
};
```

Pattern:
- Specialize std::hash
- Hash by hashing underlying type
- noexcept guarantee

## Use Cases

### Basic ID Usage

```cpp
enum class ObjectKind { Mesh, Curve, Light, Camera };

using MeshID = ObjectID<ObjectKind, ObjectKind::Mesh>;
using CurveID = ObjectID<ObjectKind, ObjectKind::Curve>;

MeshID meshID1(0);
CurveID curveID1(0);

// Different kinds are not directly convertible
// MeshID meshID2 = curveID1;  // Compile error
```

### Invalid ID

```cpp
MeshID invalidID;  // Default constructor creates invalid ID

if (invalidID.isValid()) {
    // This won't execute
}

MeshID validID(0);
if (validID.isValid()) {
    // This will execute
}
```

### ID Comparison

```cpp
MeshID id1(0);
MeshID id2(1);

if (id1 == id2) {
    // Won't execute
}

if (id1 < id2) {
    // Will execute
}
```

### ID Iteration

```cpp
MeshID id(0);
for (int i = 0; i < 10; ++i) {
    std::cout << id.get() << " ";
    ++id;
}
// Output: 0 1 2 3 4 5 6 7 8 9
```

### Unordered Map

```cpp
std::unordered_map<MeshID, Mesh> meshMap;

MeshID id(0);
Mesh mesh;
meshMap[id] = mesh;

Mesh& retrievedMesh = meshMap[id];
```

### String Formatting

```cpp
MeshID id(42);
std::string str = fmt::format("Mesh ID: {}", id);
// str = "Mesh ID: 42"
```

### Kind Conversion

```cpp
enum class ObjectKind { Mesh, Curve, Union };

using MeshID = ObjectID<ObjectKind, ObjectKind::Mesh>;
using CurveID = ObjectID<ObjectKind, ObjectKind::Curve>;
using UnionID = ObjectID<ObjectKind, ObjectKind::Union>;

MeshID meshID(0);
UnionID unionID(meshID);  // Convert from Mesh to Union
```

### Slang Integration

```cpp
MeshID id = MeshID::fromSlang(42);
uint32_t slangID = id.getSlang();
```

### Vector of IDs

```cpp
std::vector<MeshID> meshIDs;

for (int i = 0; i < 10; ++i) {
    meshIDs.push_back(MeshID(i));
}

for (const auto& id : meshIDs) {
    std::cout << id.get() << " ";
}
```

### ID Validation

```cpp
MeshID id(0);

if (id.isValid()) {
    // ID is valid
}

MeshID invalidID;
if (!invalidID.isValid()) {
    // ID is invalid
}
```

## Performance Considerations

### Memory Overhead

- **ID Size**: sizeof(TIntType) (default: sizeof(uint32_t) = 4 bytes)
- **No Dynamic Allocation**: No heap allocation
- **No Virtual Functions**: No vtable overhead
- **Minimal Overhead**: Same as underlying integer type

### Construction Performance

- **Default Constructor**: Simple assignment (kInvalidID)
- **Numeric Constructor**: Validation + assignment
- **Kind Conversion Constructor**: Simple assignment
- **fromSlang()**: Delegates to numeric constructor

### Comparison Performance

- **Comparison Operators**: Simple integer comparison
- **Equality**: Simple integer equality check
- **Inequality**: Simple integer inequality check
- **Ordering**: Simple integer ordering

### Hash Performance

- **Hash Function**: Hash of underlying integer type
- **Lookup**: O(1) average for unordered containers
- **Insertion**: O(1) average for unordered containers

### Format Performance

- **String Conversion**: std::to_string of underlying integer
- **fmt Formatting**: fmt::format of underlying integer
- **Overhead**: Same as integer formatting

### Compiler Optimizations

- **Inline Expansion**: All methods can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Copy Elision**: Copy elision for return values

### Comparison with Alternatives

**ObjectID vs Raw Integer**:
- ObjectID: Type-safe, validation, invalid ID tracking
- Raw Integer: No type safety, no validation, no invalid ID tracking
- Performance: Similar (ObjectID has minimal overhead)

**ObjectID vs std::variant**:
- ObjectID: Compile-time type checking, minimal overhead
- std::variant: Runtime type checking, more overhead
- Performance: ObjectID is faster

### Optimization Tips

1. **Use Appropriate Type**: Use smallest TIntType for range
2. **Avoid Unnecessary Conversions**: Avoid unnecessary kind conversions
3. **Use Const References**: Use const references when possible
4. **Enable Optimizations**: Enable compiler optimizations
5. **Use isValid()**: Use isValid() instead of comparing with kInvalidID

## Limitations

### Feature Limitations

- **No Decrement Operators**: No decrement operators (only increment)
- **No Arithmetic Operators**: No arithmetic operators (+, -, *, /)
- **No Bitwise Operators**: No bitwise operators (&, |, ^, ~)
- **No Modulo Operator**: No modulo operator (%)
- **No Range Checking**: No range checking beyond construction
- **No ID Generation**: No ID generation mechanism
- **No ID Pooling**: No ID pooling mechanism
- **No ID Recycling**: No ID recycling mechanism

### API Limitations

- **No to_string() Member**: No to_string() member function (only free function)
- **No from_string()**: No from_string() function
- **No stream operators**: No stream operators (<<, >>)
- **No swap()**: No swap() function
- **No exchange()**: No exchange() function
- **No size()**: No size() method
- **No empty()**: No empty() method
- **No front()**: No front() method
- **No back()**: No back() method

### Type Limitations

- **Integral Types Only**: Only works with integral types
- **No Floating Point**: No floating point support
- **No Custom Types**: No custom type support
- **No Pointer Types**: No pointer type support
- **Same Enum Kind**: Kind conversion only works for same enum kind

### Performance Limitations

- **Validation Overhead**: Validation on construction
- **Type Erasure**: No type erasure (always strongly typed)
- **No Runtime Polymorphism**: No runtime polymorphism

### Platform Limitations

- **C++17 Only**: Requires C++17 or later
- **No C++20 Features**: No C++20 features (concepts, ranges, etc.)
- **No Standard Library Views**: No standard library views support

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Object identification
- Type-safe IDs
- Scene object IDs
- Rendering object IDs
- Resource IDs
- Entity IDs

**Inappropriate Use Cases**:
- Floating point values (use other methods)
- Custom types (use other methods)
- Pointer types (use other methods)
- Complex objects (use other methods)

### Usage Patterns

**Basic ID Usage**:
```cpp
enum class ObjectKind { Mesh, Curve, Light, Camera };
using MeshID = ObjectID<ObjectKind, ObjectKind::Mesh>;

MeshID meshID(0);
```

**Invalid ID**:
```cpp
MeshID invalidID;  // Default constructor creates invalid ID
if (!invalidID.isValid()) {
    // Handle invalid ID
}
```

**ID Comparison**:
```cpp
MeshID id1(0);
MeshID id2(1);

if (id1 == id2) {
    // IDs are equal
}
```

**ID Iteration**:
```cpp
MeshID id(0);
for (int i = 0; i < 10; ++i) {
    // Process id
    ++id;
}
```

**Unordered Map**:
```cpp
std::unordered_map<MeshID, Mesh> meshMap;
meshMap[MeshID(0)] = mesh;
```

### Type Safety

- **Use Strong Typing**: Use strong typing for all IDs
- **Avoid Raw Integers**: Avoid using raw integers for IDs
- **Use Appropriate Kind**: Use appropriate kind for each ID type
- **Avoid Kind Conversion**: Avoid unnecessary kind conversion

### Performance Tips

1. **Use Appropriate Type**: Use smallest TIntType for range
2. **Avoid Unnecessary Conversions**: Avoid unnecessary kind conversions
3. **Use Const References**: Use const references when possible
4. **Enable Optimizations**: Enable compiler optimizations
5. **Use isValid()**: Use isValid() instead of comparing with kInvalidID

### Error Handling

- **Validate IDs**: Validate IDs on construction
- **Check Validity**: Check ID validity before use
- **Handle Invalid IDs**: Handle invalid IDs appropriately
- **Use Assertions**: Use assertions for debugging

### Slang Integration

- **Use fromSlang()**: Use fromSlang() for Slang integration
- **Use getSlang()**: Use getSlang() for Slang integration
- **Separate Methods**: Use separate methods for clarity
- **Plan for Removal**: Plan for removal when Slang has strongly typed IDs

### Future Migration

- **Plan for C++20**: Plan for C++20 features
- **Use Concepts**: Use concepts when available
- **Use Ranges**: Use ranges when available
- **Update Code**: Update code to use new features

## Implementation Notes

### Strong Typing Implementation

Uses template parameters for strong typing:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType = uint32_t>
class ObjectID
{
    // ... implementation ...
};
```

**Characteristics**:
- Template parameters define type
- Different enum classes are not directly convertible
- Compile-time type checking
- Type-safe object identification

### Invalid ID Implementation

Supports invalid ID detection:

```cpp
static constexpr IntType kInvalidID = std::numeric_limits<IntType>::max();

ObjectID() : mID(kInvalidID) {}
bool isValid() const { return mID != kInvalidID; }
```

**Characteristics**:
- Invalid ID is max value of IntType
- Default constructor creates invalid ID
- isValid() method checks validity
- Invalid() static method returns invalid ID

### Validation Implementation

Validates ID values on construction:

```cpp
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
explicit ObjectID(const T& id) : mID(IntType(id))
{
    FALCOR_ASSERT_GE(id, T(0));
    FALCOR_ASSERT_LE(std::make_unsigned_t<T>(id), std::numeric_limits<IntType>::max());
}
```

**Characteristics**:
- Validates that id >= 0
- Validates that id <= max value of IntType
- Uses FALCOR_ASSERT macros for validation
- Prevents invalid ID values

### Kind Conversion Implementation

Allows conversion between different kinds of same enum:

```cpp
template<TKindEnum TOtherKind>
explicit ObjectID(const ObjectID<TKindEnum, TOtherKind, TIntType>& other)
{
    mID = other.get();
}
```

**Characteristics**:
- Explicit conversion (not implicit)
- Only works for same enum kind
- Copies numeric value
- Slightly safer than direct numeric conversion

### Slang Integration Implementation

Provides conversion to/from Slang numeric IDs:

```cpp
template<typename T>
static ObjectID fromSlang(const T& id, std::enable_if_t<std::is_integral_v<T>, bool> = true)
{
    return ObjectID{id};
}

IntType getSlang() const { return get(); }
```

**Characteristics**:
- fromSlang() creates ObjectID from numeric ID
- getSlang() returns numeric ID for Slang
- Separate methods for clarity
- TODO: Remove once Slang has strongly typed IDs

### Hash Implementation

Integrates with std::hash for unordered containers:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct std::hash<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    using ObjectID = Falcor::ObjectID<TKindEnum, TKind, TIntType>;
    std::size_t operator()(const ObjectID& id) const noexcept { return std::hash<typename ObjectID::IntType>{}(id.get()); }
};
```

**Characteristics**:
- Hashes ObjectID by hashing its IntType value
- Allows ObjectID to be used in unordered containers
- noexcept guaranteed
- Standard library integration

### Format Implementation

Integrates with fmt for string formatting:

```cpp
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct fmt::formatter<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    template<typename FormatContext>
    auto format(const ObjectID& id, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{0}", id.get());
    }
};
```

**Characteristics**:
- Formats ObjectID by formatting its IntType value
- Allows ObjectID to be used with fmt library
- Standard library integration
- Easy string conversion

### Memory Layout

Memory layout of ObjectID:

```
ObjectID<TKindEnum, TKind, TIntType>:
    mID: TIntType (numeric ID value)
    Total size: sizeof(TIntType)
```

**Characteristics**:
- Simple POD type
- No dynamic allocation
- No virtual functions
- Minimal memory overhead

## Future Enhancements

### Potential Improvements

1. **Decrement Operators**: Add decrement operators
2. **Arithmetic Operators**: Add arithmetic operators (+, -, *, /)
3. **Bitwise Operators**: Add bitwise operators (&, |, ^, ~)
4. **Modulo Operator**: Add modulo operator (%)
5. **Range Checking**: Add range checking methods
6. **ID Generation**: Add ID generation mechanism
7. **ID Pooling**: Add ID pooling mechanism
8. **ID Recycling**: Add ID recycling mechanism
9. **C++20 Concepts**: Use C++20 concepts for type constraints
10. **C++20 Ranges**: Use C++20 ranges for iteration

### API Extensions

1. **to_string() Member**: Add to_string() member function
2. **from_string()**: Add from_string() function
3. **Stream Operators**: Add stream operators (<<, >>)
4. **swap()**: Add swap() function
5. **exchange()**: Add exchange() function
6. **size()**: Add size() method (for ranges)
7. **empty()**: Add empty() method (for ranges)
8. **front()**: Add front() method (for ranges)
9. **back()**: Add back() method (for ranges)
10. **distance()**: Add distance() method

### Performance Enhancements

1. **Remove Validation**: Add option to disable validation
2. **Uninitialized ID**: Add option for uninitialized ID
3. **Constexpr**: Add constexpr support
4. **Inline Everything**: Ensure all methods are inline
5. **Compiler Hints**: Add optimization hints for compiler

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from raw integers
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::enable_if_t**: C++14 type trait for SFINAE
- **std::is_integral_v**: C++17 type trait for integral types
- **std::make_unsigned_t**: C++14 type trait for unsigned types
- **std::numeric_limits**: C++11 class for numeric limits
- **std::hash**: C++11 class for hashing

### C++17 Features

- **std::enable_if_t**: C++14 type trait for SFINAE
- **std::is_integral_v**: C++17 type trait for integral types
- **std::make_unsigned_t**: C++14 type trait for unsigned types

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **Strong Typing**: Strong typing in C++
- **Type Safety**: Type safety in C++
- **SFINAE**: Substitution Failure Is Not An Error
- **Template Metaprogramming**: Template metaprogramming in C++
