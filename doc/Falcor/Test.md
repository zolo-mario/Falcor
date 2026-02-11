---
modified: 2026-01-20T10:41:16+08:00
created: 2026-01-13T08:39:58+08:00
tags:
  - 2026/01/13
---
# Testing - Unit Testing Framework

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **UnitTest** - Unit testing framework
- [x] **UnitTestContext** - Unit test context base class
- [x] **CPUUnitTestContext** - CPU unit test context
- [x] **GPUUnitTestContext** - GPU unit test context

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, ComputeState)
- **Core/Program** - Shader program management (Program, ProgramVars, ProgramReflection, DefineList, ShaderModel, SlangCompilerFlags)
- **Core/Error** - Error handling (Exception)
- **Core/State** - Graphics state (ComputeState)
- **Utils/Math** - Mathematical utilities (Vector)
- **Utils/StringUtils** - String utilities (splitString, joinStrings)
- **Utils/Scripting** - Scripting system
- **Utils/Threading** - Threading utilities
- **Utils/Logger** - Logging system
- **Utils/TermColor** - Terminal color utilities

## Module Overview

The Testing module provides a comprehensive unit testing framework for Falcor. It supports both CPU and GPU unit tests, with features for test filtering, parallel execution, XML report generation, and flexible assertion macros. The framework is designed to be easy to use while providing powerful features for complex testing scenarios.

## Component Specifications

### UnitTest.h

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:1)

**Purpose**: User-facing API for unit testing framework.

**Key Features**:
- CPU and GPU test support
- Test registration and enumeration
- Test filtering by suite, case, tags, and device type
- Assertion macros for common test patterns
- Stream-based output capture
- Parallel and serial test execution
- XML report generation (JUnit format)
- Test skipping and tagging

**Constants**:
- **kMaxTestFailures** (25) - Maximum test failures before aborting

**Exception Types**:
- **TooManyFailedTestsException** - Thrown when too many tests fail
- **ErrorRunningTestException** - Test execution error
- **AssertingTestException** - Test assertion failure (no stack trace)
- **SkippingTestException** - Test skipped (no stack trace)

**RunOptions** (from [`RunOptions`](Source/Falcor/Testing/UnitTest.h:87)):
- **deviceDesc** - Device description for GPU tests
- **testSuiteFilter** - Filter tests by suite name
- **testCaseFilter** - Filter tests by case name
- **tagFilter** - Filter tests by tags
- **xmlReportPath** - Path for XML report output
- **parallel** - Number of parallel threads (default: 1)
- **repeat** - Number of times to repeat each test (default: 1)

**Test Types**:
- **CPUTestFunc** - CPU test function signature: `void(CPUUnitTestContext& ctx)`
- **GPUTestFunc** - GPU test function signature: `void(GPUUnitTestContext& ctx)`

**Test Structure** (from [`Test`](Source/Falcor/Testing/UnitTest.h:106)):
- **suiteName** - Test suite name (file path)
- **name** - Test case name
- **tags** - Set of tags for filtering
- **skipMessage** - Skip message (if not empty, test is skipped)
- **deviceType** - Device type for GPU tests
- **cpuFunc** - CPU test function
- **gpuFunc** - GPU test function

**Core Functions**:
- [`runTests()`](Source/Falcor/Testing/UnitTest.h:98) - Run tests with options
- [`enumerateTests()`](Source/Falcor/Testing/UnitTest.h:119) - Enumerate all registered tests
- [`filterTests()`](Source/Falcor/Testing/UnitTest.h:122) - Filter tests by criteria

### UnitTestContext

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:130)

**Purpose**: Base class for unit test contexts.

**Methods**:
- [`skip()`](Source/Falcor/Testing/UnitTest.h:136) - Skip current test with message
- [`reportFailure()`](Source/Falcor/Testing/UnitTest.h:143) - Report test failure
- [`getFailureMessages()`](Source/Falcor/Testing/UnitTest.h:145) - Get list of failure messages


### CPUUnitTestContext

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:153)

**Purpose**: Context for CPU unit tests.

**Inheritance**: Inherits from UnitTestContext

**Features**:
- No GPU resources
- Pure CPU testing
- Simple assertion-based testing

### GPUUnitTestContext

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:156)

**Purpose**: Context for GPU unit tests.

**Inheritance**: Inherits from UnitTestContext

**Core Methods**:
- [`createProgram()`](Source/Falcor/Testing/UnitTest.h:167) - Create compute program from source file
- [`createProgram()`](Source/Falcor/Testing/UnitTest.h:179) - Create compute program from program desc
- [`createVars()`](Source/Falcor/Testing/UnitTest.h:186) - Create shader variables
- [`vars()`](Source/Falcor/Testing/UnitTest.h:192) - Get program variables
- [`operator[]()`](Source/Falcor/Testing/UnitTest.h:202) - Get shader variable by name
- [`allocateStructuredBuffer()`](Source/Falcor/Testing/UnitTest.h:219) - Allocate structured buffer
- [`readBuffer<T>()`](Source/Falcor/Testing/UnitTest.h:225) - Read buffer contents into vector
- [`runProgram()`](Source/Falcor/Testing/UnitTest.h:242) - Run compute program with dimensions
- [`runProgram()`](Source/Falcor/Testing/UnitTest.h:249) - Run compute program (1D, 2D, 3D)
- [`getDevice()`](Source/Falcor/Testing/UnitTest.h:254) - Get render device
- [`getRenderContext()`](Source/Falcor/Testing/UnitTest.h:259) - Get render context
- [`getProgram()`](Source/Falcor/Testing/UnitTest.h:264) - Get program
- [`getVars()`](Source/Falcor/Testing/UnitTest.h:269) - Get program vars

**Members**:
- **mpDevice** - Render device reference
- **mpState** - Compute state
- **mpProgram** - Program reference
- **mpVars** - Program vars reference
- **mThreadGroupSize** - Thread group size from reflection
- **mStructuredBuffers** - Map of allocated structured buffers

### Test Registration

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:352)

**Functions**:
- [`registerCPUTest()`](Source/Falcor/Testing/UnitTest.h:352) - Register CPU test
- [`registerGPUTest()`](Source/Falcor/Testing/UnitTest.h:353) - Register GPU test

**Registration Parameters**:
- **path** - File path of test
- **name** - Test name
- **options** - Test options (tags, skip message, device types)
- **func** - Test function

### StreamSink

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:363)

**Purpose**: Utility class for capturing test output and failures.

**Key Features**:
- Captures C++ operator<< output
- Discards or accumulates based on test status
- Passes failure messages to context
- Automatic new-line handling

**Methods**:
- [`StreamSink()`](Source/Falcor/Testing/UnitTest.h:371) - Move constructor
- [`StreamSink(UnitTestContext*)`](Source/Falcor/Testing/UnitTest.h:379) - Context constructor
- [`~StreamSink()`](Source/Falcor/Testing/UnitTest.h:381) - Destructor reports failures
- [`setInsertNewLine()`](Source/Falcor/Testing/UnitTest.h:387) - Set new-line insertion flag
- [`operator<<()`](Source/Falcor/Testing/UnitTest.h:390) - Stream insertion operator

**Members**:
- **mSs** - String stream for accumulation
- **mpCtx** - Test context (nullptr for discard, valid for capture)
- **mInsertNewLine** - New-line insertion flag

### Assertion Macros

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:436)

**Boolean Assertions**:
- **EXPECT_TRUE_MSG(expression, msg)** - Expect expression to be true
- **EXPECT_FALSE_MSG(expression, msg)** - Expect expression to be false
- **ASSERT_TRUE_MSG(expression, msg)** - Assert expression to be true (throws on failure)
- **ASSERT_FALSE_MSG(expression, msg)** - Assert expression to be false (throws on failure)
- **EXPECT_TRUE(expression)** - Expect expression to be true (no message)
- **EXPECT_FALSE(expression)** - Expect expression to be false (no message)
- **ASSERT_TRUE(expression)** - Assert expression to be true (no message, throws on failure)
- **ASSERT_FALSE(expression)** - Assert expression to be false (no message, throws on failure)
- **EXPECT(expression)** - Alias for EXPECT_TRUE
- **ASSERT(expression)** - Alias for ASSERT_TRUE
- **EXPECT_MSG(expression, msg)** - Alias for EXPECT_TRUE_MSG
- **ASSERT_MSG(expression, msg)** - Alias for ASSERT_TRUE_MSG

**Comparison Assertions**:
- **EXPECT_EQ_MSG(lhs, rhs, msg)** - Expect lhs == rhs
- **EXPECT_NE_MSG(lhs, rhs, msg)** - Expect lhs != rhs
- **EXPECT_LE_MSG(lhs, rhs, msg)** - Expect lhs <= rhs
- **EXPECT_GE_MSG(lhs, rhs, msg)** - Expect lhs >= rhs
- **EXPECT_LT_MSG(lhs, rhs, msg)** - Expect lhs < rhs
- **EXPECT_GT_MSG(lhs, rhs, msg)** - Expect lhs > rhs
- **ASSERT_EQ_MSG(lhs, rhs, msg)** - Assert lhs == rhs (throws on failure)
- **ASSERT_NE_MSG(lhs, rhs, msg)** - Assert lhs != rhs (throws on failure)
- **ASSERT_LE_MSG(lhs, rhs, msg)** - Assert lhs <= rhs (throws on failure)
- **ASSERT_GE_MSG(lhs, rhs, msg)** - Assert lhs >= rhs (throws on failure)
- **ASSERT_LT_MSG(lhs, rhs, msg)** - Assert lhs < rhs (throws on failure)
- **ASSERT_GT_MSG(lhs, rhs, msg)** - Assert lhs > rhs (throws on failure)
- **EXPECT_EQ(lhs, rhs)** - Expect lhs == rhs (no message)
- **EXPECT_NE(lhs, rhs)** - Expect lhs != rhs (no message)
- **EXPECT_LE(lhs, rhs)** - Expect lhs <= rhs (no message)
- **EXPECT_GE(lhs, rhs)** - Expect lhs >= rhs (no message)
- **EXPECT_LT(lhs, rhs)** - Expect lhs < rhs (no message)
- **EXPECT_GT(lhs, rhs)** - Expect lhs > rhs (no message)
- **ASSERT_EQ(lhs, rhs)** - Assert lhs == rhs (no message, throws on failure)
- **ASSERT_NE(lhs, rhs)** - Assert lhs != rhs (no message, throws on failure)
- **ASSERT_LE(lhs, rhs)** - Assert lhs <= rhs (no message, throws on failure)
- **ASSERT_GE(lhs, rhs)** - Assert lhs >= rhs (no message, throws on failure)
- **ASSERT_LT(lhs, rhs)** - Assert lhs < rhs (no message, throws on failure)
- **ASSERT_GT(lhs, rhs)** - Assert lhs > rhs (no message, throws on failure)

**Exception Assertions**:
- **EXPECT_THROW(expression)** - Expect expression to throw any exception
- **EXPECT_THROW_AS(expression, type)** - Expect expression to throw specific type

### Test Definition Macros

**File**: [`UnitTest.h`](Source/Falcor/Testing/UnitTest.h:556)

**CPU_TEST Macro**:
```cpp
CPU_TEST(name, ...)
```

**Optional Arguments**:
- **SKIP(msg)** - Skip test with message (expands to unittest::Skip)
- **TAGS(...)** - List of tags to associate with test (expands to unittest::Tags)
- String literal - Skip message (for backwards compatibility)

**Examples**:
- `CPU_TEST(Test1) {}` - Test is always run
- `CPU_TEST(Test2, SKIP("Not implemented")) {}` - Test is skipped
- `CPU_TEST(Test3, TAGS("tag1", "tag2")) {}` - Test is tagged

**Note**: All CPU tests are implicitly tagged with "cpu".

**GPU_TEST Macro**:
```cpp
GPU_TEST(name, ...)
```

**Optional Arguments**:
- **SKIP(msg)** - Skip test with message (expands to unittest::Skip)
- **TAGS(...)** - List of tags to associate with test (expands to unittest::Tags)
- **DEVICE_TYPES(...)** - List of device types to run test on (expands to unittest::DeviceTypes)
- Device::Type values - Specify device type to run test on
- String literal - Skip message (for backwards compatibility)

**Examples**:
- `GPU_TEST(Test1) {}` - Test is always run
- `GPU_TEST(Test2, SKIP("Not implemented")) {}` - Test is skipped
- `GPU_TEST(Test3, TAGS("tag1", "tag2")) {}` - Test is tagged
- `GPU_TEST(Test4, DEVICE_TYPES(Device::Type::D3D12)) {}` - Test only runs on D3D12
- `GPU_TEST(Test5, Device::Type::D3D12) {}` - Test only runs on D3D12 (same as above)

**Note**: All GPU tests are implicitly tagged with "gpu".

**Helper Macros**:
- **TAGS(...)** - Tag a test with a set of strings
- **SKIP(msg)** - Mark a test to be skipped
- **DEVICE_TYPES(...)** - Mark a test to only run for certain devices

### UnitTest.cpp

**File**: [`UnitTest.cpp`](Source/Falcor/Testing/UnitTest.cpp:1)

**Purpose**: Implementation of unit testing framework.

**Key Components**:

**TestDesc** (from [`TestDesc`](Source/Falcor/Testing/UnitTest.cpp:57)):
- **path** - File path
- **name** - Test name
- **options** - Test options
- **cpuFunc** - CPU test function
- **gpuFunc** - GPU test function

**TestResult** (from [`TestResult`](Source/Falcor/Testing/UnitTest.cpp:66)):
- **Status** - Passed, Failed, or Skipped
- **messages** - List of failure messages
- **extraMessage** - Additional message
- **elapsedMS** - Elapsed time in milliseconds

**DevicePool** (from [`DevicePool`](Source/Falcor/Testing/UnitTest.cpp:87)):
- **Purpose**: Pool for device reuse
- **acquireDevice(deviceType)** - Acquire device from pool
- **releaseDevice(device)** - Release device back to pool

**Test Execution**:
- [`runTest()`](Source/Falcor/Testing/UnitTest.cpp:214) - Run a single test
- [`runTestsParallel()`](Source/Falcor/Testing/UnitTest.cpp:285) - Run tests in parallel
- [`runTestsSerial()`](Source/Falcor/Testing/UnitTest.cpp:377) - Run tests serially
- [`runTests()`](Source/Falcor/Testing/UnitTest.cpp:499) - Main entry point

**Test Enumeration**:
- [`enumerateTests()`](Source/Falcor/Testing/UnitTest.cpp:519) - Enumerate all registered tests
- [`filterTests()`](Source/Falcor/Testing/UnitTest.cpp:575) - Filter tests by criteria

**Report Generation**:
- [`writeXmlReport()`](Source/Falcor/Testing/UnitTest.cpp:170) - Write JUnit XML report
- **reportLine()** (from [`reportLine`](Source/Falcor/Testing/UnitTest.cpp:148)) - Print report line to console and log

**Output Format**:
- Google Test style output
- Test suite grouping
- Per-test timing
- Failure tracking
- XML report generation (JUnit format)

### UnitTest.cs.slang

**File**: [`UnitTest.cs.slang`](Source/Falcor/Testing/UnitTest.cs.slang:1)

**Purpose**: Simple GPU test shader for testing framework.

**Shader Components**:
- **result** - `RWStructuredBuffer<float>` for output
- **TestCB** - Constant buffer with:
  - **nValues** - Number of values
  - **scale** - Scale factor

**Main Function**:
- Iterates through nValues
- Computes result[i] = scale * i
- Used to verify GPU test framework works correctly

## Technical Details

### Test Registration System

**Automatic Registration**:
- Static registration via CPU_TEST/GPU_TEST macros
- Automatic test discovery
- File-based test suite organization
- Automatic tagging (cpu/gpu tags)

**Test Storage**:
- Static registry of test descriptors
- Per-test metadata (name, tags, skip message, device types)
- CPU and GPU function pointers

### Test Filtering

**Filter Criteria**:
- **Test Suite Filter** - Regex-based suite name filtering
- **Test Case Filter** - Regex-based case name filtering
- **Tag Filter** - Comma-separated tag filtering with +/- operators
- **Device Type Filter** - Filter by device type (D3D12, Vulkan)

**Tag Filtering**:
- **+tag** - Include tests with this tag
- **-tag** or **!tag** or **~tag** - Exclude tests with this tag
- **tag** - Include tests with this tag (default)
- Multiple tags can be specified

### Test Execution

**Parallel Execution**:
- Thread pool-based parallel execution
- Configurable parallel thread count
- Per-test device pooling
- Ctrl-C handling for graceful abort

**Serial Execution**:
- Sequential test execution
- Test suite grouping
- Per-suite timing
- Repeat support for flaky tests

**Device Pooling**:
- Device reuse across tests
- Type-based device pooling
- Automatic device creation and cleanup
- Thread-safe device acquisition

### GPU Test Context

**Program Creation**:
- Source file-based program creation
- Program descriptor-based creation
- Optional shader variable creation
- Custom entry point support
- Preprocessor defines support
- Compiler flags support
- Shader model support

**Buffer Management**:
- Automatic structured buffer allocation
- Buffer name-based lookup
- Initial data support
- Size validation
- Automatic buffer binding

**Program Execution**:
- Thread group size from reflection
- Automatic thread group calculation
- Dispatch dimension validation
- Automatic buffer binding
- Render context integration

### Assertion System

**Boolean Assertions**:
- EXPECT_* macros continue on failure
- ASSERT_* macros throw on failure
- Message support for all assertions
- Single evaluation guarantee

**Comparison Assertions**:
- Type-safe comparisons
- Template-based comparison helpers
- Support for ` ==, !=, <=, >=, <, >`
- Custom error messages

**Exception Assertions**:
- EXPECT_THROW - Any exception
- EXPECT_THROW_AS - Specific exception type
- Try-catch pattern

**Stream Capture**:
- C++ operator<< interception
- Automatic new-line handling
- Context-based failure reporting
- Discard mode for passed tests

### Report Generation

**Console Output**:
- Google Test style formatting
- Test suite grouping
- Per-test status tags
- Timing information
- Failure summaries

**XML Report**:
- JUnit format
- Per-suite organization
- Test case attributes
- Failure elements
- Skipped elements
- Timing information

**Report Structure**:
- Test suite nodes
- Test case nodes
- Status attributes
- Failure message elements
- Timing attributes

## Integration Points

### Core Integration

**Device Management**:
- Device pool for GPU tests
- Device descriptor support
- Device type filtering
- Automatic device creation

**Program System Integration**:
- Program creation for compute shaders
- Program vars for shader binding
- Program reflection for thread group size
- Compute state for dispatch

**Render Context Integration**:
- Automatic render context access
- Compute dispatch support
- Device synchronization

### Utilities Integration

**String Utilities**:
- Test suite name extraction
- String joining for reports
- String splitting for tag parsing

**Logging Integration**:
- Test output logging
- Console output control
- Debug window output

**Threading Integration**:
- Thread pool for parallel execution
- Device pool thread safety
- Atomic abort flag

## Architecture Patterns

**Macro-Based Registration Pattern**:
- Static registration via macros
- Automatic test discovery
- Type-safe test function signatures
- Metadata attachment

**Context-Based Testing Pattern**:
- Context objects for test state
- Failure message accumulation
- Resource management (GPU context)
- Assertion tracking

**Stream Capture Pattern**:
- Operator<< interception
- Automatic failure reporting
- Discard mode for passed tests
- Context-based accumulation

**Filter Chain Pattern**:
- Multiple filter criteria
- Regex-based name filtering
- Tag-based filtering
- Device type filtering
- Composable filters

**Device Pool Pattern**:
- Device reuse
- Type-based pooling
- Thread-safe acquisition
- Automatic cleanup

## Progress Log

- **2026-01-07T20:30:00Z**: Testing module analysis completed. Analyzed UnitTest.h user-facing API, UnitTest.cpp implementation, and UnitTest.cs.slang shader. Documented CPU and GPU test support, test registration and enumeration, test filtering, assertion macros, stream-based output capture, parallel and serial test execution, XML report generation, device pooling, and GPU test context. Testing module analysis is complete.

## Next Steps

Testing module analysis is complete. All sub-modules have been analyzed and documented. Ready to proceed with other modules or finalize the overall analysis.
