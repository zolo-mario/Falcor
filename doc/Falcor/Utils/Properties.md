# Properties - Property Management System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros (FALCOR_API)
- Falcor/Core/Error (FALCOR_THROW)
- Falcor/Core/Enum (enumToString, stringToEnum, has_enum_info_v)
- Falcor/Utils/Math/VectorTypes (int2, int3, int4, uint2, uint3, uint4, float2, float3, float4)
- nlohmann/json_fwd.hpp (nlohmann::ordered_json)
- pybind11/pytypes.h (pybind11::dict, pybind11::handle)
- fmt/core.h (fmt::formatter)
- std::memory
- std::optional
- std::string_view
- std::string
- std::type_traits
- std::filesystem
- std::cstdint

### Dependent Modules

- Falcor/Utils/Settings (Settings module)
- Falcor/Scene (Scene properties)
- Falcor/Rendering (Rendering properties)
- Falcor/Mogwai (Application properties)

## Module Overview

Properties provides a comprehensive property management system for storing, accessing, and serializing properties. Properties are stored as JSON objects using nlohmann::ordered_json to preserve insertion order. The system supports automatic serialization/deserialization of objects, Python dictionary integration, enum support, vector type support, and custom serialization through the serialize() member function template.

## Component Specifications

### detail Namespace

**File**: `Source/Falcor/Utils/Properties.h` (lines 52-75)

#### DummyArchive Class

```cpp
class DummyArchive
{};
```

**Purpose**: Dummy archive type for detecting serialize() member function

**Behavior**: Empty class used for compile-time detection

#### serialize_t Type Alias

```cpp
template<typename T>
using serialize_t = decltype(&T::template serialize<DummyArchive>);
```

**Purpose**: Type alias for detecting serialize() member function

**Behavior**: Detects if type T has a serialize() member function template

#### has_serialize_v Variable Template

```cpp
template<typename T>
inline constexpr bool has_serialize_v = detect<T, serialize_t>::value;
```

**Purpose**: Compile-time check if type has serialize() member function

**Behavior**: Returns true if type T has serialize() member function, false otherwise

### Properties Class

**File**: `Source/Falcor/Utils/Properties.h` (lines 92-362)
**File**: `Source/Falcor/Utils/Properties.cpp` (lines 287-494)

```cpp
class FALCOR_API Properties
{
public:
    using json = nlohmann::ordered_json;

    Properties();
    Properties(const json& j);
    Properties(json&& j);
    Properties(const pybind11::dict& d);
    Properties(const Properties& other);
    Properties(Properties&& other);

    ~Properties();

    Properties& operator=(const Properties& other);
    Properties& operator=(Properties&& other);

    json toJson() const;
    pybind11::dict toPython() const;
    std::string dump(int indent = -1) const;
    bool empty() const;
    bool has(std::string_view name) const;

    template<typename T>
    void set(std::string_view name, const T& value);

    template<typename T>
    T get(std::string_view name) const;

    template<typename T>
    T get(std::string_view name, const T& def) const;

    template<typename T>
    bool getTo(std::string_view name, T& value) const;

    template<typename T>
    std::optional<T> getOpt(std::string_view name) const;

    void set(std::string_view name, const char* value);

    bool operator==(const Properties& rhs) const;
    bool operator!=(const Properties& rhs) const;

    Value operator[](std::string_view name);
    const ConstValue operator[](std::string_view name) const;

    Iterator begin();
    Iterator end();
    ConstIterator begin() const;
    ConstIterator end() const;

private:
    template<typename T>
    void setInternal(std::string_view name, const T& value);

    template<typename T>
    bool getInternal(std::string_view name, T& value) const;

    std::unique_ptr<json> mJson;
};
```

**Purpose**: Main class for storing properties

**Type Definitions**:
```cpp
using json = nlohmann::ordered_json;
```

**json**: Ordered JSON type for property storage

#### Constructors

**Default Constructor**:
```cpp
Properties();
```

**Purpose**: Construct empty properties

**Implementation**:
```cpp
Properties::Properties()
{
    mJson = std::make_unique<json>(json::object());
}
```

**Copy Constructor**:
```cpp
Properties(const Properties& other);
```

**Purpose**: Copy properties from another instance

**Implementation**:
```cpp
Properties::Properties(const Properties& other)
{
    mJson = std::make_unique<json>(*other.mJson);
}
```

**Move Constructor**:
```cpp
Properties(Properties&& other);
```

**Purpose**: Move properties from another instance

**Implementation**:
```cpp
Properties::Properties(Properties&& other)
{
    mJson = std::move(other.mJson);
}
```

**JSON Constructor (Copy)**:
```cpp
Properties(const json& j);
```

**Purpose**: Construct properties from JSON object

**Implementation**:
```cpp
Properties::Properties(const json& j)
{
    mJson = std::make_unique<json>(j);
}
```

**JSON Constructor (Move)**:
```cpp
Properties(json&& j);
```

**Purpose**: Construct properties from JSON object (move)

**Implementation**:
```cpp
Properties::Properties(json&& j)
{
    mJson = std::make_unique<json>(std::move(j));
}
```

**Python Dictionary Constructor**:
```cpp
Properties(const pybind11::dict& d);
```

**Purpose**: Construct properties from Python dictionary

**Implementation**:
```cpp
Properties::Properties(const pybind11::dict& d)
{
    mJson = std::make_unique<json>(pythonToJson(d));
}
```

**Destructor**:
```cpp
~Properties();
```

**Purpose**: Destructor

**Implementation**:
```cpp
Properties::~Properties() {}
```

#### Assignment Operators

**Copy Assignment**:
```cpp
Properties& operator=(const Properties& other);
```

**Purpose**: Copy assignment

**Implementation**:
```cpp
Properties& Properties::operator=(const Properties& other)
{
    mJson = std::make_unique<json>(*other.mJson);
    return *this;
}
```

**Move Assignment**:
```cpp
Properties& operator=(Properties&& other);
```

**Purpose**: Move assignment

**Implementation**:
```cpp
Properties& Properties::operator=(Properties&& other)
{
    mJson = std::move(other.mJson);
    return *this;
}
```

#### Conversion Methods

**toJson()**:
```cpp
json toJson() const;
```

**Purpose**: Convert properties to JSON object

**Returns**: JSON object

**Implementation**:
```cpp
json Properties::toJson() const
{
    return *mJson;
}
```

**toPython()**:
```cpp
pybind11::dict toPython() const;
```

**Purpose**: Convert properties to Python dictionary

**Returns**: Python dictionary

**Implementation**:
```cpp
pybind11::dict Properties::toPython() const
{
    return jsonToPython(*mJson);
}
```

**dump()**:
```cpp
std::string dump(int indent = -1) const;
```

**Purpose**: Dump properties to string

**Parameters**:
- indent: Indentation level (default: -1 for no indentation)

**Returns**: String representation of properties

**Implementation**:
```cpp
std::string Properties::dump(int indent) const
{
    return mJson->dump(indent);
}
```

#### Query Methods

**empty()**:
```cpp
bool empty() const;
```

**Purpose**: Check if properties are empty

**Returns**: true if empty, false otherwise

**Implementation**:
```cpp
bool Properties::empty() const
{
    return mJson->empty();
}
```

**has()**:
```cpp
bool has(std::string_view name) const;
```

**Purpose**: Check if a property exists

**Parameters**:
- name: Property name

**Returns**: true if property exists, false otherwise

**Implementation**:
```cpp
bool Properties::has(std::string_view name) const
{
    return mJson->find(name) != mJson->end();
}
```

#### Property Access Methods

**set()**:
```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (has_enum_info_v<T>)
    {
        setInternal(name, enumToString(value));
    }
    else if constexpr (detail::has_serialize_v<T>)
    {
        setInternal(name, serializeToProperties(value));
    }
    else
    {
        setInternal<T>(name, value);
    }
}
```

**Purpose**: Set a property

**Parameters**:
- name: Property name
- value: Property value

**Behavior**:
- If T is enum, convert to string and set
- If T has serialize() member, serialize to Properties and set
- Otherwise, set value directly

**get()**:
```cpp
template<typename T>
T get(std::string_view name) const
{
    if constexpr (has_enum_info_v<T>)
    {
        std::string value;
        if (!getInternal(name, value))
            FALCOR_THROW("Property '{}' does not exist.", name);
        return stringToEnum<T>(value);
    }
    else if constexpr (detail::has_serialize_v<T>)
    {
        Properties props;
        if (!getInternal(name, props))
            FALCOR_THROW("Property '{}' does not exist.", name);
        return deserializeFromProperties<T>(props);
    }
    else
    {
        T value;
        if (!getInternal(name, value))
            FALCOR_THROW("Property '{}' does not exist.", name);
        return value;
    }
}
```

**Purpose**: Get a property (throws if not found)

**Parameters**:
- name: Property name

**Returns**: Property value

**Behavior**:
- If T is enum, get string and convert to enum
- If T has serialize() member, get Properties and deserialize
- Otherwise, get value directly
- Throws if property does not exist

**get() with Default**:
```cpp
template<typename T>
T get(std::string_view name, const T& def) const
{
    if constexpr (has_enum_info_v<T>)
    {
        std::string value;
        if (!getInternal(name, value))
            return def;
        return stringToEnum<T>(value);
    }
    else if constexpr (detail::has_serialize_v<T>)
    {
        Properties props;
        if (!getInternal(name, props))
            return def;
        return deserializeFromProperties<T>(props);
    }
    else
    {
        T value;
        if (!getInternal(name, value))
            return def;
        return value;
    }
}
```

**Purpose**: Get a property (returns default if not found)

**Parameters**:
- name: Property name
- def: Default value

**Returns**: Property value or default

**Behavior**:
- If T is enum, get string and convert to enum (or return default)
- If T has serialize() member, get Properties and deserialize (or return default)
- Otherwise, get value directly (or return default)

**getTo()**:
```cpp
template<typename T>
bool getTo(std::string_view name, T& value) const
{
    if constexpr (has_enum_info_v<T>)
    {
        std::string enumString;
        bool result = getInternal(name, enumString);
        if (result)
            value = stringToEnum<T>(enumString);
        return result;
    }
    else if constexpr (detail::has_serialize_v<T>)
    {
        Properties props;
        bool result = getInternal(name, props);
        if (result)
            value = deserializeFromProperties<T>(props);
        return result;
    }
    else
    {
        return getInternal<T>(name, value);
    }
}
```

**Purpose**: Get a property (stores to reference, returns true if found)

**Parameters**:
- name: Property name
- value: Reference to store value

**Returns**: true if property exists, false otherwise

**Behavior**:
- If T is enum, get string and convert to enum
- If T has serialize() member, get Properties and deserialize
- Otherwise, get value directly
- Returns true if property exists, false otherwise

**getOpt()**:
```cpp
template<typename T>
std::optional<T> getOpt(std::string_view name) const
{
    if constexpr (has_enum_info_v<T>)
    {
        std::string enumString;
        bool result = getInternal(name, enumString);
        return result ? std::make_optional<T>(stringToEnum<T>(enumString)) : std::nullopt;
    }
    else if constexpr (detail::has_serialize_v<T>)
    {
        Properties props;
        bool result = getInternal(name, props);
        return result ? std::make_optional<T>(deserializeFromProperties<T>(props)) : std::nullopt;
    }
    else
    {
        T value;
        bool result = getInternal<T>(name, value);
        return result ? std::make_optional<T>(value) : std::nullopt;
    }
}
```

**Purpose**: Get a property (returns optional)

**Parameters**:
- name: Property name

**Returns**: std::optional<T> (value if found, nullopt otherwise)

**Behavior**:
- If T is enum, get string and convert to enum (or return nullopt)
- If T has serialize() member, get Properties and deserialize (or return nullopt)
- Otherwise, get value directly (or return nullopt)

**set() for C String**:
```cpp
void set(std::string_view name, const char* value) { set(name, std::string(value)); }
```

**Purpose**: Convenience overload for C strings

**Parameters**:
- name: Property name
- value: C string value

**Behavior**: Converts C string to std::string and sets property

#### Comparison Operators

**operator==()**:
```cpp
bool operator==(const Properties& rhs) const;
```

**Purpose**: Equality comparison

**Returns**: true if equal, false otherwise

**Implementation**:
```cpp
bool Properties::operator==(const Properties& rhs) const
{
    return *mJson == *rhs.mJson;
}
```

**operator!=()**:
```cpp
bool operator!=(const Properties& rhs) const;
```

**Purpose**: Inequality comparison

**Returns**: true if not equal, false otherwise

**Implementation**:
```cpp
bool Properties::operator!=(const Properties& rhs) const
{
    return !(*this == rhs);
}
```

#### Subscript Operators

**operator[]()**:
```cpp
Value operator[](std::string_view name) { return Value(*this, name); }
const ConstValue operator[](std::string_view name) const { return ConstValue(*this, name); }
```

**Purpose**: Subscript operator for property access

**Parameters**:
- name: Property name

**Returns**: Value accessor (non-const) or ConstValue accessor (const)

#### Iterator Methods

**begin()**:
```cpp
Iterator begin();
ConstIterator begin() const;
```

**Purpose**: Get iterator to beginning

**Returns**: Iterator (non-const) or ConstIterator (const)

**end()**:
```cpp
Iterator end();
ConstIterator end() const;
```

**Purpose**: Get iterator to end

**Returns**: Iterator (non-const) or ConstIterator (const)

#### Private Methods

**setInternal()**:
```cpp
template<typename T>
void setInternal(std::string_view name, const T& value);
```

**Purpose**: Internal method for setting property

**Parameters**:
- name: Property name
- value: Property value

**Behavior**: Converts value to JSON and sets property

**getInternal()**:
```cpp
template<typename T>
bool getInternal(std::string_view name, T& value) const;
```

**Purpose**: Internal method for getting property

**Parameters**:
- name: Property name
- value: Reference to store value

**Returns**: true if property exists, false otherwise

**Behavior**: Gets property from JSON, converts to T, stores to value

#### Private Member

```cpp
std::unique_ptr<json> mJson;
```

**mJson**: JSON object storing properties

### Properties::Value Class

**File**: `Source/Falcor/Utils/Properties.h` (lines 269-291)

```cpp
class Value
{
public:
    template<typename T>
        void operator=(const T& value) const
        {
            mProperties.set(mName, value);
        }

    template<typename T>
        operator T() const
        {
            return mProperties.get<T>(mName);
        }

private:
    Value(Properties& properties, std::string_view name) : mProperties(properties), mName(name) {}
    Properties& mProperties;
    std::string mName;

    friend class Properties;
};
```

**Purpose**: Value accessor class for convenient property access

**operator=()**: Set property value

**operator T()**: Get property value

**Private Members**:
- **mProperties**: Reference to Properties object
- **mName**: Property name

### Properties::ConstValue Class

**File**: `Source/Falcor/Utils/Properties.h` (lines 293-309)

```cpp
class ConstValue
{
public:
    template<typename T>
        operator T() const
        {
            return mProperties.get<T>(mName);
        }

private:
    ConstValue(const Properties& properties, std::string_view name) : mProperties(properties), mName(name) {}
    const Properties& mProperties;
    std::string mName;

    friend class Properties;
};
```

**Purpose**: Constant value accessor class for convenient property access

**operator T()**: Get property value

**Private Members**:
- **mProperties**: Reference to Properties object
- **mName**: Property name

### Properties::Iterator Class

**File**: `Source/Falcor/Utils/Properties.h` (lines 311-326)
**File**: `Source/Falcor/Utils/Properties.cpp` (lines 388-416)

```cpp
class FALCOR_API Iterator
{
public:
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;
    Iterator& operator++();
    std::pair<std::string, Value> operator*();
    ~Iterator();

private:
    struct Impl;
    Iterator(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> mImpl;
    friend class Properties;
};
```

**Purpose**: Forward iterator for Properties

**operator==()**: Equality comparison

**operator!=()**: Inequality comparison

**operator++()**: Pre-increment

**operator*()**: Dereference (returns pair of name and Value)

**Private Members**:
- **mImpl**: Pointer to implementation

**Impl Structure**:
```cpp
struct Properties::Iterator::Impl
{
    Properties& properties;
    json::iterator it;
};
```

### Properties::ConstIterator Class

**File**: `Source/Falcor/Utils/Properties.h` (lines 328-343)
**File**: `Source/Falcor/Utils/Properties.cpp` (lines 418-446)

```cpp
class FALCOR_API ConstIterator
{
public:
    bool operator==(const ConstIterator& other) const;
    bool operator!=(const ConstIterator& other) const;
    ConstIterator& operator++();
    std::pair<std::string, ConstValue> operator*();
    ~ConstIterator();

private:
    struct Impl;
    ConstIterator(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> mImpl;
    friend class Properties;
};
```

**Purpose**: Constant forward iterator for Properties

**operator==()**: Equality comparison

**operator!=()**: Inequality comparison

**operator++()**: Pre-increment

**operator*()**: Dereference (returns pair of name and ConstValue)

**Private Members**:
- **mImpl**: Pointer to implementation

**Impl Structure**:
```cpp
struct Properties::ConstIterator::Impl
{
    const Properties& properties;
    json::const_iterator it;
};
```

### PropertiesWriter Class

**File**: `Source/Falcor/Utils/Properties.h` (lines 390-416)

```cpp
class PropertiesWriter
{
public:
    template<typename T>
        void operator()(std::string_view name, const T& value)
        {
            if constexpr (detail::has_serialize_v<T>)
                mProperties.set<Properties>(name, PropertiesWriter::write<T>(value));
            else
                mProperties.set<T>(name, value);
        }

    template<typename T>
    static Properties write(const T& value)
    {
        PropertiesWriter writer;
        const_cast<T&>(value).serialize(writer);
        return writer.mProperties;
    }

private:
    PropertiesWriter() = default;
    Properties mProperties;
};
```

**Purpose**: Helper class for serializing objects into Properties

**operator()()**: Set property with automatic serialization

**write()**: Static method for serializing object

**Private Members**:
- **mProperties**: Properties object for storing serialized data

### PropertiesReader Class

**File**: `Source/Falcor/Utils/Properties.h` (lines 418-449)

```cpp
class PropertiesReader
{
public:
    template<typename T>
        void operator()(std::string_view name, T& value)
        {
            if constexpr (detail::has_serialize_v<T>)
            {
                Properties props;
                if (mProperties.getTo<Properties>(name, props))
                    value = PropertiesReader::read<T>(props);
            }
            else
                mProperties.getTo<T>(name, value);
        }

    template<typename T>
    static T read(const Properties& props)
    {
        T value;
        PropertiesReader reader(props);
        value.serialize(reader);
        return value;
    }

private:
    PropertiesReader(const Properties& props) : mProperties(props) {}
    const Properties& mProperties;
};
```

**Purpose**: Helper class for deserializing objects from Properties

**operator()()**: Get property with automatic deserialization

**read()**: Static method for deserializing object

**Private Members**:
- **mProperties**: Reference to Properties object

### serializeToProperties Function

**File**: `Source/Falcor/Utils/Properties.h` (lines 451-455)

```cpp
template<typename T>
Properties serializeToProperties(const T& value)
{
    return PropertiesWriter::write(value);
}
```

**Purpose**: Serialize object to Properties

**Parameters**:
- value: Object to serialize

**Returns**: Properties containing serialized object

**Behavior**: Uses PropertiesWriter to serialize object

### deserializeFromProperties Function

**File**: `Source/Falcor/Utils/Properties.h` (lines 457-461)

```cpp
template<typename T>
T deserializeFromProperties(const Properties& props)
{
    return PropertiesReader::read<T>(props);
}
```

**Purpose**: Deserialize object from Properties

**Parameters**:
- props: Properties to deserialize from

**Returns**: Deserialized object

**Behavior**: Uses PropertiesReader to deserialize object

### fmt::formatter Specialization

**File**: `Source/Falcor/Utils/Properties.h` (lines 465-473)

```cpp
template<>
struct fmt::formatter<Falcor::Properties> : formatter<std::string>
{
    template<typename FormatContext>
    auto format(const Falcor::Properties& props, FormatContext& ctx) const
    {
        return formatter<std::string>::format(props.dump(), ctx);
    }
};
```

**Purpose**: fmt formatter specialization for Properties

**Behavior**: Formats Properties as string using dump()

## Technical Details

### JSON Storage

Uses nlohmann::ordered_json for property storage:

```cpp
using json = nlohmann::ordered_json;
std::unique_ptr<json> mJson;
```

**Characteristics**:
- Ordered storage (preserves insertion order)
- JSON format for serialization
- Unique pointer for efficient copying
- Support for multiple data types

### Type Detection

Detects serialize() member function at compile time:

```cpp
template<typename T>
using serialize_t = decltype(&T::template serialize<DummyArchive>);

template<typename T>
inline constexpr bool has_serialize_v = detect<T, serialize_t>::value;
```

**Characteristics**:
- Compile-time detection
- SFINAE-based
- No runtime overhead
- Type-safe

### Enum Support

Automatic enum string conversion:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (has_enum_info_v<T>)
    {
        setInternal(name, enumToString(value));
    }
    // ...
}
```

**Characteristics**:
- Automatic enum to string conversion
- Uses Falcor enum utilities
- Type-safe enum handling
- Compile-time detection

### Vector Support

Support for vector types (int2, int3, int4, uint2, uint3, uint4, float2, float3, float4):

```cpp
template<typename VecT>
json vecToJson(const VecT& vec)
{
    auto constexpr length = VecT::length();
    auto j = json::array();
    for (size_t i = 0; i < length; ++i)
        j.push_back(vec[i]);
    return j;
}
```

**Characteristics**:
- Automatic vector to JSON array conversion
- Preserves vector length
- Type-safe conversion
- Compile-time length detection

### Custom Serialization

Support for custom serialization through serialize() member function:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (detail::has_serialize_v<T>)
    {
        setInternal(name, serializeToProperties(value));
    }
    // ...
}
```

**Characteristics**:
- Compile-time detection
- Automatic serialization
- Recursive serialization
- Type-safe

### Python Integration

Seamless conversion to/from Python dictionaries:

```cpp
json pythonToJson(const pybind11::handle& obj);
pybind11::object jsonToPython(const json& j);
```

**Characteristics**:
- Automatic type conversion
- Support for Python types (bool, int, float, str, list, dict)
- Support for filesystem::path
- Support for vector types

### Iterator Implementation

Forward iterator for Properties:

```cpp
class Iterator
{
    struct Impl
    {
        Properties& properties;
        json::iterator it;
    };

    std::pair<std::string, Value> operator*()
    {
        return std::make_pair(mImpl->it.key(), Value(mImpl->properties, mImpl->it.key()));
    }
};
```

**Characteristics**:
- Forward iterator only
- Returns pair of name and Value
- Pimpl pattern for implementation
- Const and non-const variants

### Value Accessor Classes

Convenient property access through accessor classes:

```cpp
class Value
{
    template<typename T>
        void operator=(const T& value) const
        {
            mProperties.set(mName, value);
        }

    template<typename T>
        operator T() const
        {
            return mProperties.get<T>(mName);
        }
};
```

**Characteristics**:
- Convenient property access
- Type-safe access
- Const and non-const variants
- Subscript operator support

### JSON Conversion

Conversion between JSON and C++ types:

```cpp
template<typename T>
json valueToJson(const T& value);

template<typename T>
T valueFromJson(const json& json, std::string_view name);
```

**Characteristics**:
- Type-safe conversion
- Validation of JSON types
- Support for multiple types
- Error throwing on mismatch

## Integration Points

### Falcor Core Integration

- **Core/Macros**: FALCOR_API
- **Core/Error**: FALCOR_THROW
- **Core/Enum**: enumToString, stringToEnum, has_enum_info_v

### Falcor Utils Integration

- **Utils/Math/VectorTypes**: int2, int3, int4, uint2, uint3, uint4, float2, float3, float4

### External Dependencies

- **nlohmann/json_fwd.hpp**: nlohmann::ordered_json
- **pybind11/pytypes.h**: pybind11::dict, pybind11::handle
- **fmt/core.h**: fmt formatter

### STL Integration

- **std::memory**: std::unique_ptr
- **std::optional**: std::optional
- **std::string_view**: String view
- **std::string**: String
- **std::type_traits**: Type traits
- **std::filesystem**: Filesystem path
- **std::cstdint**: Integer types

### Internal Falcor Usage

- **Utils/Settings**: Settings module
- **Scene**: Scene properties
- **Rendering**: Rendering properties
- **Mogwai**: Application properties

## Architecture Patterns

### Type Safety Pattern

Template-based type-safe property access:

```cpp
template<typename T>
T get(std::string_view name) const
{
    // ... type-safe access ...
}
```

Benefits:
- Compile-time type checking
- No runtime type errors
- Clear API
- Type conversions handled automatically

### Custom Serialization Pattern

Support for custom serialization through serialize() member function:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (detail::has_serialize_v<T>)
    {
        setInternal(name, serializeToProperties(value));
    }
    // ...
}
```

Benefits:
- Extensible serialization
- Automatic detection
- Recursive serialization
- Type-safe

### Pimpl Pattern

Pimpl pattern for iterator implementation:

```cpp
class Iterator
{
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};
```

Benefits:
- Encapsulation
- Stable ABI
- Reduced compilation dependencies
- Clear separation of interface and implementation

### Value Accessor Pattern

Convenient property access through accessor classes:

```cpp
class Value
{
    template<typename T>
        void operator=(const T& value) const
        {
            mProperties.set(mName, value);
        }

    template<typename T>
        operator T() const
        {
            return mProperties.get<T>(mName);
        }
};
```

Benefits:
- Convenient access
- Type-safe access
- Clear semantics
- Subscript operator support

### Enum Conversion Pattern

Automatic enum string conversion:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (has_enum_info_v<T>)
    {
        setInternal(name, enumToString(value));
    }
    // ...
}
```

Benefits:
- Automatic conversion
- Type-safe enum handling
- Compile-time detection
- No manual conversion needed

### Python Integration Pattern

Seamless conversion to/from Python dictionaries:

```cpp
json pythonToJson(const pybind11::handle& obj);
pybind11::object jsonToPython(const json& j);
```

Benefits:
- Seamless Python integration
- Automatic type conversion
- Support for multiple types
- Easy Python API

## Code Patterns

### Type Detection Pattern

Detect serialize() member function at compile time:

```cpp
template<typename T>
using serialize_t = decltype(&T::template serialize<DummyArchive>);

template<typename T>
inline constexpr bool has_serialize_v = detect<T, serialize_t>::value;
```

Pattern:
- SFINAE-based detection
- Compile-time evaluation
- No runtime overhead
- Type-safe

### Enum Conversion Pattern

Automatic enum string conversion:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (has_enum_info_v<T>)
    {
        setInternal(name, enumToString(value));
    }
    // ...
}
```

Pattern:
- Compile-time detection
- Automatic conversion
- Type-safe handling
- Conditional compilation

### Vector Conversion Pattern

Automatic vector to JSON array conversion:

```cpp
template<typename VecT>
json vecToJson(const VecT& vec)
{
    auto constexpr length = VecT::length();
    auto j = json::array();
    for (size_t i = 0; i < length; ++i)
        j.push_back(vec[i]);
    return j;
}
```

Pattern:
- Compile-time length detection
- Array conversion
- Type-safe conversion
- Loop over vector elements

### Custom Serialization Pattern

Support for custom serialization:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (detail::has_serialize_v<T>)
    {
        setInternal(name, serializeToProperties(value));
    }
    // ...
}
```

Pattern:
- Compile-time detection
- Automatic serialization
- Recursive serialization
- Type-safe

### Pimpl Pattern

Pimpl pattern for iterator implementation:

```cpp
class Iterator
{
    struct Impl
    {
        Properties& properties;
        json::iterator it;
    };

    std::unique_ptr<Impl> mImpl;
};
```

Pattern:
- Private implementation structure
- Unique pointer to implementation
- Encapsulation
- Stable ABI

### Value Accessor Pattern

Convenient property access through accessor classes:

```cpp
class Value
{
    template<typename T>
        void operator=(const T& value) const
        {
            mProperties.set(mName, value);
        }

    template<typename T>
        operator T() const
        {
            return mProperties.get<T>(mName);
        }

private:
    Value(Properties& properties, std::string_view name) : mProperties(properties), mName(name) {}
    Properties& mProperties;
    std::string mName;
};
```

Pattern:
- Private constructor
- Reference to Properties
- Property name stored
- Template operators

## Use Cases

### Basic Property Access

```cpp
Properties props;

// Set properties
props.set("name", "John");
props.set("age", 30);
props.set("height", 1.75f);

// Get properties
std::string name = props.get<std::string>("name");
int age = props.get<int>("age");
float height = props.get<float>("height");
```

### Enum Properties

```cpp
enum class Color { Red, Green, Blue };

Properties props;

// Set enum property
props.set("color", Color::Red);

// Get enum property
Color color = props.get<Color>("color");
```

### Vector Properties

```cpp
Properties props;

// Set vector property
props.set("position", float3(1.0f, 2.0f, 3.0f));
props.set("size", int2(640, 480));

// Get vector property
float3 position = props.get<float3>("position");
int2 size = props.get<int2>("size");
```

### Custom Serialization

```cpp
struct MyObject
{
    int value;
    std::string name;

    template<typename Archive>
    void serialize(Archive& ar)
    {
        ar("value", value);
        ar("name", name);
    }
};

Properties props;

// Set custom object
MyObject obj{42, "test"};
props.set("object", obj);

// Get custom object
MyObject result = props.get<MyObject>("object");
```

### Default Values

```cpp
Properties props;

// Get with default
int age = props.get<int>("age", 0);
std::string name = props.get<std::string>("name", "unknown");
```

### Optional Values

```cpp
Properties props;

// Get optional
std::optional<int> age = props.getOpt<int>("age");
if (age)
{
    // Property exists
    std::cout << "Age: " << *age << std::endl;
}
else
{
    // Property does not exist
    std::cout << "Age not set" << std::endl;
}
```

### Subscript Access

```cpp
Properties props;

// Set using subscript
props["name"] = "John";
props["age"] = 30;

// Get using subscript
std::string name = props["name"];
int age = props["age"];
```

### Iteration

```cpp
Properties props;

props.set("name", "John");
props.set("age", 30);
props.set("height", 1.75f);

// Iterate over properties
for (auto& [key, value] : props)
{
    std::cout << key << " = " << value.get<std::string>() << std::endl;
}
```

### JSON Conversion

```cpp
Properties props;

props.set("name", "John");
props.set("age", 30);

// Convert to JSON
auto json = props.toJson();
std::string jsonString = json.dump();

// Convert from JSON
Properties props2(json);
```

### Python Integration

```cpp
Properties props;

props.set("name", "John");
props.set("age", 30);

// Convert to Python dictionary
pybind11::dict dict = props.toPython();

// Convert from Python dictionary
Properties props2(dict);
```

### File Serialization

```cpp
Properties props;

props.set("name", "John");
props.set("age", 30);

// Dump to string
std::string jsonString = props.dump(4);

// Save to file
std::ofstream file("properties.json");
file << jsonString;
file.close();
```

### Nested Properties

```cpp
Properties props;

Properties nested;
nested.set("value", 42);
nested.set("name", "test");

props.set("nested", nested);

// Get nested properties
Properties result = props.get<Properties>("nested");
int value = result.get<int>("value");
std::string name = result.get<std::string>("name");
```

## Performance Considerations

### Memory Overhead

- **JSON Storage**: nlohmann::ordered_json overhead
- **Unique Pointer**: std::unique_ptr overhead
- **String Storage**: String storage for property names and values
- **Vector Storage**: Array storage for vector types

### Construction Performance

- **Default Constructor**: Simple JSON object creation
- **Copy Constructor**: Deep copy of JSON object
- **Move Constructor**: Move of unique pointer
- **JSON Constructor**: Copy of JSON object
- **Python Constructor**: Python to JSON conversion

### Access Performance

- **Property Access**: JSON object lookup (O(log n) average)
- **Type Conversion**: Type conversion overhead
- **Enum Conversion**: String to enum conversion overhead
- **Vector Conversion**: Array to vector conversion overhead

### Serialization Performance

- **Custom Serialization**: Recursive serialization overhead
- **Vector Serialization**: Array creation overhead
- **Type Detection**: Compile-time detection (no runtime overhead)

### Python Integration Performance

- **Python to JSON**: Python type checking and conversion
- **JSON to Python**: JSON type checking and conversion
- **Type Conversion**: Type conversion overhead

### Compiler Optimizations

- **Inline Expansion**: Template methods can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead

### Comparison with Alternatives

**Properties vs std::map**:
- Properties: JSON-based, ordered, serialization support
- std::map: Faster, no serialization, unordered
- Performance: std::map is faster for simple key-value storage

**Properties vs std::unordered_map**:
- Properties: JSON-based, ordered, serialization support
- std::unordered_map: Faster, no serialization, unordered
- Performance: std::unordered_map is faster for simple key-value storage

### Optimization Tips

1. **Reuse Properties**: Reuse Properties objects when possible
2. **Avoid Copies**: Avoid unnecessary copies of Properties
3. **Use Move Semantics**: Use move semantics for efficiency
4. **Cache Lookups**: Cache frequently accessed properties
5. **Enable Optimizations**: Enable compiler optimizations

## Limitations

### Feature Limitations

- **Forward Iterator Only**: No bidirectional or random access iterators
- **No Property Deletion**: No method to delete properties
- **No Property Count**: No method to get property count
- **No Property Keys**: No method to get all property keys
- **No Property Values**: No method to get all property values
- **No Property Merge**: No method to merge Properties
- **No Property Clone**: No method to clone Properties (use copy constructor)
- **No Property Clear**: No method to clear all properties
- **No Property Validation**: No validation of property values
- **No Property Constraints**: No constraints on property values

### API Limitations

- **No Erase Method**: No method to erase properties
- **No Clear Method**: No method to clear all properties
- **No Size Method**: No method to get property count
- **No Keys Method**: No method to get all property keys
- **No Values Method**: No method to get all property values
- **No Merge Method**: No method to merge Properties
- **No Validate Method**: No method to validate properties
- **No Reserve Method**: No method to reserve capacity

### Type Limitations

- **JSON Types Only**: Only supports JSON types (null, boolean, number, string, array, object)
- **No Custom Types**: No support for custom types without serialize() member
- **No Pointer Types**: No support for pointer types
- **No Reference Types**: No support for reference types
- **No Function Types**: No support for function types

### Performance Limitations

- **JSON Overhead**: JSON storage overhead
- **Type Conversion**: Type conversion overhead
- **String Conversion**: Enum string conversion overhead
- **Python Conversion**: Python type conversion overhead
- **No Caching**: No property caching

### Platform Limitations

- **C++17 Only**: Requires C++17 or later
- **nlohmann/json**: Requires nlohmann/json library
- **pybind11**: Requires pybind11 for Python integration
- **No C++20 Features**: No C++20 features (concepts, ranges, etc.)

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Configuration management
- Scene properties
- Rendering properties
- Application settings
- Serialization/deserialization
- Python integration
- Type-safe property storage

**Inappropriate Use Cases**:
- High-performance key-value storage (use std::unordered_map)
- Large data sets (use specialized data structures)
- Complex data structures (use specialized containers)
- Real-time systems (use optimized data structures)

### Usage Patterns

**Basic Property Access**:
```cpp
Properties props;

props.set("name", "John");
std::string name = props.get<std::string>("name");
```

**Enum Properties**:
```cpp
enum class Color { Red, Green, Blue };

Properties props;

props.set("color", Color::Red);
Color color = props.get<Color>("color");
```

**Vector Properties**:
```cpp
Properties props;

props.set("position", float3(1.0f, 2.0f, 3.0f));
float3 position = props.get<float3>("position");
```

**Custom Serialization**:
```cpp
struct MyObject
{
    int value;
    std::string name;

    template<typename Archive>
    void serialize(Archive& ar)
    {
        ar("value", value);
        ar("name", name);
    }
};

Properties props;

MyObject obj{42, "test"};
props.set("object", obj);
MyObject result = props.get<MyObject>("object");
```

### Type Safety

- **Use Template Methods**: Use template methods for type safety
- **Avoid Manual Casting**: Avoid manual type casting
- **Use Enum Support**: Use enum support for enum types
- **Use Custom Serialization**: Use custom serialization for complex types

### Performance Tips

1. **Reuse Properties**: Reuse Properties objects when possible
2. **Avoid Copies**: Avoid unnecessary copies of Properties
3. **Use Move Semantics**: Use move semantics for efficiency
4. **Cache Lookups**: Cache frequently accessed properties
5. **Enable Optimizations**: Enable compiler optimizations

### Python Integration

- **Use toPython()**: Use toPython() for Python integration
- **Use Python Constructor**: Use Python constructor for Python dictionaries
- **Handle Type Conversion**: Handle type conversion between C++ and Python
- **Use pybind11**: Use pybind11 for Python integration

### Serialization

- **Implement serialize()**: Implement serialize() for custom types
- **Use PropertiesWriter**: Use PropertiesWriter for serialization
- **Use PropertiesReader**: Use PropertiesReader for deserialization
- **Handle Nested Objects**: Handle nested objects correctly

### Error Handling

- **Handle Exceptions**: Handle FALCOR_THROW exceptions
- **Check Property Existence**: Check property existence before access
- **Use Default Values**: Use default values for optional properties
- **Use Optional Values**: Use optional values for optional properties

## Implementation Notes

### Type Detection Implementation

Detects serialize() member function at compile time:

```cpp
template<typename T>
using serialize_t = decltype(&T::template serialize<DummyArchive>);

template<typename T>
inline constexpr bool has_serialize_v = detect<T, serialize_t>::value;
```

**Characteristics**:
- SFINAE-based detection
- Compile-time evaluation
- No runtime overhead
- Type-safe

### JSON Storage Implementation

Uses nlohmann::ordered_json for property storage:

```cpp
using json = nlohmann::ordered_json;
std::unique_ptr<json> mJson;
```

**Characteristics**:
- Ordered storage (preserves insertion order)
- JSON format for serialization
- Unique pointer for efficient copying
- Support for multiple data types

### Enum Conversion Implementation

Automatic enum string conversion:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (has_enum_info_v<T>)
    {
        setInternal(name, enumToString(value));
    }
    // ...
}
```

**Characteristics**:
- Compile-time detection
- Automatic conversion
- Type-safe handling
- Conditional compilation

### Vector Conversion Implementation

Automatic vector to JSON array conversion:

```cpp
template<typename VecT>
json vecToJson(const VecT& vec)
{
    auto constexpr length = VecT::length();
    auto j = json::array();
    for (size_t i = 0; i < length; ++i)
        j.push_back(vec[i]);
    return j;
}
```

**Characteristics**:
- Compile-time length detection
- Array conversion
- Type-safe conversion
- Loop over vector elements

### Python Integration Implementation

Seamless conversion to/from Python dictionaries:

```cpp
json pythonToJson(const pybind11::handle& obj);
pybind11::object jsonToPython(const json& j);
```

**Characteristics**:
- Automatic type conversion
- Support for Python types (bool, int, float, str, list, dict)
- Support for filesystem::path
- Support for vector types

### Iterator Implementation

Forward iterator for Properties:

```cpp
class Iterator
{
    struct Impl
    {
        Properties& properties;
        json::iterator it;
    };

    std::pair<std::string, Value> operator*()
    {
        return std::make_pair(mImpl->it.key(), Value(mImpl->properties, mImpl->it.key()));
    }
};
```

**Characteristics**:
- Forward iterator only
- Returns pair of name and Value
- Pimpl pattern for implementation
- Const and non-const variants

### Value Accessor Implementation

Convenient property access through accessor classes:

```cpp
class Value
{
    template<typename T>
        void operator=(const T& value) const
        {
            mProperties.set(mName, value);
        }

    template<typename T>
        operator T() const
        {
            return mProperties.get<T>(mName);
        }

private:
    Value(Properties& properties, std::string_view name) : mProperties(properties), mName(name) {}
    Properties& mProperties;
    std::string mName;
};
```

**Characteristics**:
- Private constructor
- Reference to Properties
- Property name stored
- Template operators

### Custom Serialization Implementation

Support for custom serialization through serialize() member function:

```cpp
template<typename T>
void set(std::string_view name, const T& value)
{
    if constexpr (detail::has_serialize_v<T>)
    {
        setInternal(name, serializeToProperties(value));
    }
    // ...
}
```

**Characteristics**:
- Compile-time detection
- Automatic serialization
- Recursive serialization
- Type-safe

## Future Enhancements

### Potential Improvements

1. **Bidirectional Iterator**: Add bidirectional iterator support
2. **Random Access Iterator**: Add random access iterator support
3. **Property Deletion**: Add method to delete properties
4. **Property Clear**: Add method to clear all properties
5. **Property Count**: Add method to get property count
6. **Property Keys**: Add method to get all property keys
7. **Property Values**: Add method to get all property values
8. **Property Merge**: Add method to merge Properties
9. **Property Validation**: Add method to validate properties
10. **Property Constraints**: Add constraints on property values

### API Extensions

1. **erase()**: Add method to erase properties
2. **clear()**: Add method to clear all properties
3. **size()**: Add method to get property count
4. **keys()**: Add method to get all property keys
5. **values()**: Add method to get all property values
6. **merge()**: Add method to merge Properties
7. **validate()**: Add method to validate properties
8. **reserve()**: Add method to reserve capacity
9. **contains()**: Add method to check if property exists
10. **find()**: Add method to find property

### Performance Enhancements

1. **Property Caching**: Add property caching
2. **Lazy Evaluation**: Add lazy property evaluation
3. **Batch Operations**: Add batch property operations
4. **Parallel Operations**: Add parallel property operations
5. **Memory Pool**: Add memory pool for properties

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from std::map
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::unique_ptr**: C++11 smart pointer
- **std::optional**: C++17 optional type
- **std::string_view**: C++17 string view
- **std::type_traits**: C++11 type traits
- **std::filesystem**: C++17 filesystem library

### nlohmann/json

- **nlohmann::ordered_json**: Ordered JSON type
- **nlohmann::json**: JSON library

### pybind11

- **pybind11::dict**: Python dictionary type
- **pybind11::handle**: Python handle type
- **pybind11::isinstance**: Python type checking

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation
- **Falcor Core/Enum**: Enum utilities documentation

### Related Technologies

- **JSON**: JSON data format
- **Serialization**: Object serialization patterns
- **Type Safety**: Type safety in C++
- **Python Integration**: Python-C++ integration
