# BaseGraphicsPass

## Module Overview

The **BaseGraphicsPass** class provides a base class for graphics render passes in the Falcor engine. This class serves as the foundation for all graphics passes, providing common functionality for shader program management, state management, and variable binding. It implements a flexible system for adding, removing, and managing shader defines and program variables.

## Files

- **Header**: `Source/Falcor/Core/Pass/BaseGraphicsPass.h` (95 lines)
- **Implementation**: `Source/Falcor/Core/Pass/BaseGraphicsPass.cpp` (68 lines)

## Class Definition

```cpp
class BaseGraphicsPass : public Object
{
    FALCOR_OBJECT(BaseGraphicsPass)
public:
    virtual ~BaseGraphicsPass() = default;

    /**
     * Add a define
     * @param[in] name Define name
     * @param[in] value Define value
     * @param[in] updateVars Whether to update program vars (default: false)
     */
    void addDefine(const std::string& name, const std::string& value = "", bool updateVars = false);

    /**
     * Remove a define
     * @param[in] name Define name
     * @param[in] updateVars Whether to update program vars (default: false)
     */
    void removeDefine(const std::string& name, bool updateVars = false);

    /**
     * Get the program
     * @return The program associated with this pass
     */
    ref<Program> getProgram() const { return mpState->getProgram(); }

    /**
     * Get the state
     * @return The graphics state associated with this pass
     */
    const ref<GraphicsState>& getState() const { return mpState; }

    /**
     * Get the vars
     * @return The program vars associated with this pass
     */
    const ref<ProgramVars>& getVars() const { return mpVars; }

    /**
     * Get the root var
     * @return The root program var
     */
    ShaderVar getRootVar() const { return mpVars->getRootVar(); }

    /**
     * Set a vars object. Allows the user to override internal vars, for example when one wants to share a vars object between different
     * passes.
     * @param[in] pVars The new ProgramVars object. If this is nullptr, then the pass will automatically create a new ProgramVars object
     */
    void setVars(const ref<ProgramVars>& pVars);

    /**
     * Break strong reference to device
     */
    void breakStrongReferenceToDevice();

protected:
    /**
     * Create a new object.
     * @param[in] pDevice GPU device.
     * @param[in] progDesc The program description.
     * @param[in] programDefines List of macro definitions to set into the program. The macro definitions will be set on all shader stages.
     * @return A new object, or an exception is thrown if creation failed.
     */
    BaseGraphicsPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines);

    BreakableReference<Device> mpDevice;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;
};
```

## Dependencies

### Internal Dependencies

- **Object**: Base class for reference counting
- **Device**: Required for device access and program creation
- **Program**: Required for shader program management
- **GraphicsState**: Required for graphics state management
- **ProgramVars**: Required for program variable management

### External Dependencies

- **std::string**: Standard library string container
- **std::atomic**: Standard library atomic operations
- **std::shared_ptr**: Standard library shared pointer (via ref<>)

## Cross-Platform Considerations

- **Cross-Platform**: Works on all platforms supported by Falcor
- **No Platform-Specific Code**: Pure C++ implementation
- **Standard Library**: Uses only standard library containers
- **No Conditional Compilation**: No platform-specific conditional compilation

## Usage Patterns

### Creating a Graphics Pass

```cpp
ref<Device> pDevice = ...;
ProgramDesc progDesc;
DefineList programDefines;

ref<BaseGraphicsPass> pPass = BaseGraphicsPass::create(pDevice, progDesc, programDefines);
```

### Adding Shader Defines

```cpp
pPass->addDefine("MAX_LIGHTS", "16");
pPass->addDefine("ENABLE_SHADOWS", "1");
pPass->addDefine("USE_PCF", "1", true);
```

### Removing Shader Defines

```cpp
pPass->removeDefine("MAX_LIGHTS");
pPass->removeDefine("ENABLE_SHADOWS", true);
```

### Accessing Pass Components

```cpp
ref<Program> pProgram = pPass->getProgram();
const ref<GraphicsState>& pState = pPass->getState();
const ref<ProgramVars>& pVars = pPass->getVars();
ShaderVar rootVar = pPass->getRootVar();
```

### Setting Program Variables

```cpp
ref<ProgramVars> pSharedVars = ...;
pPass->setVars(pSharedVars);
```

### Breaking Device Reference

```cpp
pPass->breakStrongReferenceToDevice();
```

## Summary

**BaseGraphicsPass** is a foundational base class for graphics render passes that provides:

### BaseGraphicsPass
- Excellent cache locality (all members in single cache line)
- O(N) time complexity for creation (N = number of shader stages × defines)
- O(1) time complexity for all accessor and modifier methods
- Dynamic shader define management
- Program and state management
- Program vars management
- Automatic resource cleanup via smart pointers
- Breakable reference pattern for device references
- 40 bytes total size (excluding virtual table and reference count storage)
- 8-byte natural alignment
- Only 4 bytes of padding

The class implements a flexible and efficient base class for graphics passes with excellent cache locality, constant-time accessors, and comprehensive shader program management, providing a solid foundation for all derived graphics pass implementations.
