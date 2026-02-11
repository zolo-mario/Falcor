# DefineList

## Module Overview

The **DefineList** class provides a convenient interface for managing macro definitions (preprocessor defines) used in shader compilation. This class inherits from `std::map<std::string, std::string>` and provides fluent interface methods for adding and removing macro definitions.

## Files

- **Header**: `Source/Falcor/Core/Program/DefineList.h` (86 lines)
- **Implementation**: Header-only (no separate .cpp file)

## Class Definition

```cpp
class DefineList : public std::map<std::string, std::string>
{
public:
    DefineList& add(const std::string& name, const std::string& val = "");
    DefineList& remove(const std::string& name);
    DefineList& add(const DefineList& dl);
    DefineList& remove(const DefineList& dl);
    DefineList() = default;
    DefineList(std::initializer_list<std::pair<const std::string, std::string>> il);
};
```

## Dependencies

### Internal Dependencies

- **std::map**: Inherits from std::map for map functionality
- **std::string**: Uses std::string for keys and values
- **std::initializer_list**: Uses std::initializer_list for initialization

### External Dependencies

- **None**: No external dependencies (header-only file)

## Usage Patterns

### Adding Macro Definitions

```cpp
// Add single macro definition
DefineList defines;
defines.add("USE_LIGHTING", "1");
defines.add("MAX_LIGHTS", "16");

// Method chaining
defines.add("USE_SHADOWS", "1")
       .add("MAX_SHADOW_CASCADES", "4")
       .add("SHADOW_MAP_SIZE", "2048");
```

### Removing Macro Definitions

```cpp
// Remove single macro definition
defines.remove("USE_LIGHTING");

// Method chaining
defines.remove("USE_SHADOWS")
       .remove("MAX_SHADOW_CASCADES");
```

### Bulk Operations

```cpp
// Add multiple macro definitions
DefineList defines1;
defines1.add("USE_LIGHTING", "1")
        .add("MAX_LIGHTS", "16");

DefineList defines2;
defines2.add("USE_SHADOWS", "1")
        .add("MAX_SHADOW_CASCADES", "4");

// Add all definitions from defines2 to defines1
defines1.add(defines2);

// Remove all definitions from defines2 from defines1
defines1.remove(defines2);
```

### Initializer List

```cpp
// Initialize with initializer list
DefineList defines = {
    {"USE_LIGHTING", "1"},
    {"MAX_LIGHTS", "16"},
    {"USE_SHADOWS", "1"},
    {"MAX_SHADOW_CASCADES", "4"}
};
```

### Empty Macro Values

```cpp
// Add macro with empty value
defines.add("USE_LIGHTING"); // Value defaults to empty string

// Equivalent to
defines.add("USE_LIGHTING", "");
```

## Summary

**DefineList** is a header-only class that provides a convenient interface for managing macro definitions:

### Memory Layout
- Inherits from std::map<std::string, std::string>
- Empty map size: ~48-64 bytes
- Per entry size: ~48-64 bytes + string heap allocations
- 8-byte natural alignment
- Heap allocation for each node and string
- Poor cache locality (nodes are not contiguous)

### Threading Model
- Not thread-safe for concurrent writes
- Thread-safe for concurrent reads (if no concurrent writes)
- External synchronization required for concurrent writes
- No internal synchronization primitives
- Mutable state requires external locking

### Cache Locality
- Poor cache locality (red-black tree structure)
- Heap allocation for each node and string
- High cache misses (nodes are scattered in memory)
- No cache optimization for tree structure
- Poor prefetching (tree structure is not predictable)

### Algorithmic Complexity
- add(): O(log N) (red-black tree insertion)
- remove(): O(log N) (red-black tree erasure)
- add(const DefineList&): O(M log N) (M = number of entries)
- remove(const DefineList&): O(M log N) (M = number of entries)
- std::map operations: O(log N) for insertion, erasure, and search

### Module Characteristics
- Header-only (no implementation file)
- Fluent interface (methods return *this for method chaining)
- Inherits from std::map for map functionality
- Convenience methods for adding and removing macro definitions
- Bulk operations for adding and removing multiple definitions
- Initializer list support for initialization

The class provides a convenient fluent interface for managing macro definitions with logarithmic complexity for all operations, but suffers from poor cache locality and high memory overhead due to heap allocation for each node and string, and is not thread-safe for concurrent writes.
