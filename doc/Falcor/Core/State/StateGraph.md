# StateGraph

## Module Overview

The **StateGraph** module provides state graph management for Falcor engine. This module includes [`StateGraph`](Source/Falcor/Core/State/StateGraph.h:36) template class, providing efficient state graph management with edge-based navigation and node matching.

## Files

- **Header**: `Source/Falcor/Core/State/StateGraph.h` (113 lines)
- **Implementation**: `Source/Falcor/Core/State/StateGraph.cpp` (not provided in file list)

## Module Structure

### Classes

1. **StateGraph** - State graph management

## StateGraph Class

### Definition

```cpp
template<typename NodeType, typename EdgeType, typename EdgeHashType = std::hash<EdgeType>>
class StateGraph
{
public:
    using CompareFunc = std::function<bool(const NodeType& data)>;

    StateGraph() : mGraph(1) {}

    bool isEdgeExists(const EdgeType& e) const;
    bool walk(const EdgeType& e);
    const NodeType& getCurrentNode() const;
    void setCurrentNodeData(const NodeType& data);
    bool scanForMatchingNode(CompareFunc cmpFunc);

private:
    using edge_map = std::unordered_map<EdgeType, uint32_t, EdgeHashType>;

    const auto getEdgeIt(const EdgeType& e) const;

    struct Node
    {
        NodeType data = {0};
        edge_map edges;
    };

    std::vector<Node> mGraph;
    uint32_t mCurrentNode = 0;
};
```

### Dependencies

### Internal Dependencies

- **None**: No internal dependencies

### External Dependencies

- **std::vector**: Uses std::vector for container storage
- **std::unordered_map**: Uses std::unordered_map for edge map
- **std::hash**: Uses std::hash for edge hashing
- **std::function**: Uses std::function for compare function
- **uint32_t**: Uses uint32_t for index types

## Usage Patterns

### Creating StateGraph

```cpp
StateGraph<ref<ComputeStateObject>, void*> graph;
```

### Checking Edge Existence

```cpp
bool exists = graph.isEdgeExists(edge);
```

### Walking Edge

```cpp
bool found = graph.walk(edge);
if (found) {
    // Edge existed, moved to existing node
} else {
    // Edge didn't exist, created new node
}
```

### Getting Current Node

```cpp
const NodeType& node = graph.getCurrentNode();
```

### Setting Current Node Data

```cpp
graph.setCurrentNodeData(data);
```

### Scanning for Matching Node

```cpp
bool found = graph.scanForMatchingNode([](const NodeType& data) {
    return data == someValue;
});
if (found) {
    // Found matching node, reconnected edges
}
```

## Summary

**StateGraph** is a flexible module that provides state graph management for Falcor engine:

### StateGraph Class
- **Memory Layout**: ~28-32 bytes + heap allocations
- **Threading Model**: Not thread-safe
- **Cache Locality**: Excellent (inline members fit in cache line)
- **Algorithmic Complexity**: O(1) average case for most operations, O(N × M) for scan for matching node
- **Graph-Based State Management**: Graph-based state management for efficient state object lookup
- **Edge-Based Navigation**: Edge-based navigation for state transitions
- **Node Matching**: Node matching for state object reuse
- **Template-Based**: Template-based design for flexibility
- **Hash-Based Edge Lookup**: Hash-based edge lookup for O(1) average case
- **Dynamic Graph Growth**: Dynamic graph growth on demand

### Node Struct
- **Memory Layout**: Size of NodeType + ~48-56 bytes + heap allocations
- **Threading Model**: Not thread-safe
- **Cache Locality**: Excellent (inline members fit in cache line if NodeType is small)
- **Algorithmic Complexity**: O(1) for construction, O(N) for destruction
- **Node Data**: Node data is stored inline (if NodeType is small)
- **Edge Map**: Edge map is heap-allocated (unordered_map)

### Module Characteristics
- Graph-based state management (graph-based state management for efficient state object lookup)
- Edge-based navigation (edge-based navigation for state transitions)
- Node matching (node matching for state object reuse)
- Template-based (template-based design for flexibility)
- Hash-based edge lookup (hash-based edge lookup for O(1) average case)
- Dynamic graph growth (dynamic graph growth on demand)
- Excellent cache locality (inline members fit in cache line)
- Not thread-safe (requires external synchronization)
- Moderate complexity (graph structure)

The module provides a comprehensive state graph management system with graph-based state management, edge-based navigation, node matching, template-based design, and hash-based edge lookup for O(1) average case, optimized for performance with dynamic graph growth on demand.
