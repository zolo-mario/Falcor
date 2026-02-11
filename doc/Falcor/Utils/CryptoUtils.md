# CryptoUtils - Cryptographic Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros (FALCOR_API)
- STL (array, string, cstdint, cstdlib, sstream, iomanip)

### Dependent Modules

- Falcor/Utils (internal use)
- Falcor/Scene (asset verification)
- Falcor/Rendering (shader cache validation)

## Module Overview

CryptoUtils provides cryptographic hash functionality, specifically implementing the SHA-1 (Secure Hash Algorithm 1) algorithm. The module offers a streaming interface for incremental hash computation as well as a convenience method for one-shot hashing of data blocks. SHA-1 produces a 160-bit (20-byte) message digest, commonly represented as a 40-character hexadecimal string.

## Component Specifications

### SHA1 Class

**File**: `Source/Falcor/Utils/CryptoUtils.h` (103 lines)
**File**: `Source/Falcor/Utils/CryptoUtils.cpp` (260 lines)

The SHA1 class implements the Secure Hash Algorithm 1, providing both streaming and one-shot hash computation interfaces.

#### Type Definitions

```cpp
using MD = std::array<uint8_t, 20>;  ///< Message digest (160-bit hash output)
```

**MD**: Array of 20 bytes representing the SHA-1 message digest. Each byte contains 8 bits of the 160-bit hash value.

#### Public Interface

**Constructor**:
```cpp
SHA1();
```
- Initializes the hash state with standard SHA-1 constants
- Sets mIndex to 0 (buffer position)
- Sets mBits to 0 (total message bit count)
- Initializes mState array with standard initial values:
  - mState[0] = 0x67452301
  - mState[1] = 0xefcdab89
  - mState[2] = 0x98badcfe
  - mState[3] = 0x10325476
  - mState[4] = 0xc3d2e1f0

**update(uint8_t)**:
```cpp
void update(uint8_t value);
```
- Adds a single byte to the hash computation
- Calls addByte() to buffer the byte
- Increments mBits by 8 (one byte = 8 bits)
- Parameters:
  - value: The byte to add to the hash

**update(const void*, size_t)**:
```cpp
void update(const void* data, size_t len);
```
- Adds a block of data to the hash computation
- Handles three phases:
  1. Fills remaining buffer space if mIndex > 0
  2. Processes complete 64-byte blocks
  3. Adds remaining bytes to buffer
- Updates mBits for each processed byte
- Parameters:
  - data: Pointer to data to hash
  - len: Length of data in bytes
- Returns early if data pointer is null

**update<T>(const T&)**:
```cpp
template<typename T, std::enable_if_t<std::is_fundamental<T>::value, bool> = true>
void update(const T& value)
```
- Template method for fundamental types (int, float, double, etc.)
- Uses SFINAE to restrict to fundamental types only
- Calls update(&value, sizeof(value)) to hash the raw bytes
- Parameters:
  - value: The value to hash

**update(string_view)**:
```cpp
void update(const std::string_view str)
```
- Convenience method for hashing strings
- Calls update(str.data(), str.size())
- Parameters:
  - str: String view to hash

**finalize()**:
```cpp
MD finalize();
```
- Completes the hash computation and returns the message digest
- Performs final padding:
  1. Adds 0x80 byte (bit 1 followed by 7 zeros)
  2. Pads with zeros until buffer position is 56 (mod 64)
  3. Appends 64-bit message length in big-endian format
- Processes final block if necessary
- Converts 5 32-bit state words to 20-byte array
- Returns the message digest as MD type

**compute()**:
```cpp
static MD compute(const void* data, size_t len);
```
- Static convenience method for one-shot hash computation
- Creates temporary SHA1 instance
- Calls update(data, len) and finalize()
- Parameters:
  - data: Pointer to data to hash
  - len: Length of data in bytes
- Returns the message digest

**toString()**:
```cpp
static std::string toString(const MD& sha1);
```
- Converts SHA-1 digest to hexadecimal string representation
- Each byte becomes 2 hexadecimal characters
- Uses std::stringstream with hex formatting
- Parameters:
  - sha1: The message digest to convert
- Returns 40-character hexadecimal string

#### Private Interface

**addByte()**:
```cpp
void addByte(uint8_t byte);
```
- Adds a single byte to the internal buffer
- Increments mIndex
- If buffer is full (mIndex >= 64), processes the block and resets index
- Parameters:
  - byte: The byte to add to buffer

**processBlock()**:
```cpp
void processBlock(const uint8_t* ptr);
```
- Processes a 64-byte block through the SHA-1 compression function
- Performs 80 rounds of mixing operations
- Uses 5 different round functions across 4 stages:
  - Rounds 0-19: f = (b & (c ^ d)) ^ d
  - Rounds 20-39: f = b ^ c ^ d
  - Rounds 40-59: f = ((b | c) & d) | (b & c)
  - Rounds 60-79: f = b ^ c ^ d
- Updates the 5 state variables (a, b, c, d, e)
- Parameters:
  - ptr: Pointer to 64-byte block to process

#### Member Variables

```cpp
uint32_t mIndex;      ///< Current position in buffer (0-63)
uint64_t mBits;       ///< Total number of bits processed
uint32_t mState[5];   ///< Hash state (5 32-bit words)
uint8_t mBuf[64];     ///< Input buffer (64 bytes)
```

**mIndex**: Tracks current position in the 64-byte buffer. When it reaches 64, the buffer is processed and reset.

**mBits**: Counts total number of bits processed in the message. Used for final padding to append message length.

**mState**: Contains the 5 32-bit hash state variables. Initialized with standard SHA-1 constants and updated during block processing.

**mBuf**: 64-byte buffer for accumulating input data. SHA-1 processes data in 64-byte (512-bit) blocks.

## Technical Details

### SHA-1 Algorithm Overview

SHA-1 operates on 512-bit (64-byte) blocks of data, producing a 160-bit (20-byte) hash value. The algorithm consists of:

1. **Padding**: Message is padded to a multiple of 512 bits
2. **Processing**: Each block is processed through 80 rounds
3. **Output**: Final state produces the 160-bit digest

### Block Processing Algorithm

The [`processBlock()`](Source/Falcor/Utils/CryptoUtils.cpp:132) function implements the SHA-1 compression function:

```
1. Load 16 32-bit words from 64-byte block
2. Expand to 80 words using message schedule
3. Initialize working variables a, b, c, d, e from state
4. Execute 80 rounds with different round functions
5. Add working variables back to state
```

### Round Functions

Four different round functions are used across the 80 rounds:

**Round 0-19** (SHA1_ROUND_0):
```
f = (b & (c ^ d)) ^ d
k = 0x5a827999
```

**Round 20-39** (SHA1_ROUND_1):
```
f = b ^ c ^ d
k = 0x6ed9eba1
```

**Round 40-59** (SHA1_ROUND_2):
```
f = ((b | c) & d) | (b & c)
k = 0x8f1bbcdc
```

**Round 60-79** (SHA1_ROUND_3):
```
f = b ^ c ^ d
k = 0xca62c1d6
```

Each round computes:
```
temp = rotate_left(a, 5) + f + e + k + w[i]
e = d
d = c
c = rotate_left(b, 30)
b = a
a = temp
```

### Message Schedule Expansion

The message schedule expands 16 words to 80 words:
```cpp
w[i & 15] = rotate_left(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ 
                       w[(i + 2) & 15] ^ w[i & 15], 1);
```

This uses a circular buffer of 16 words, overwriting old values as new ones are computed.

### Final Padding Algorithm

The [`finalize()`](Source/Falcor/Utils/CryptoUtils.cpp:80) method implements standard SHA-1 padding:

```
1. Append 0x80 byte (binary: 10000000)
2. Append zeros until buffer position is 56 (mod 64)
3. Append 64-bit message length in big-endian format
4. Process final block if necessary
5. Extract 160-bit digest from state
```

The padding ensures:
- Message length is a multiple of 512 bits
- Original message length is encoded in the last 64 bits
- The 0x80 byte separates message from padding

### Byte Order Conversion

The [`finalize()`](Source/Falcor/Utils/CryptoUtils.cpp:80) method converts 5 32-bit words to 20 bytes:
```cpp
for (int i = 0; i < 5; i++) {
    for (int j = 3; j >= 0; j--) {
        md[i * 4 + j] = (mState[i] >> ((3 - j) * 8)) & 0xff;
    }
}
```

This extracts bytes in big-endian order (most significant byte first).

### Hexadecimal String Conversion

The [`toString()`](Source/Falcor/Utils/CryptoUtils.cpp:112) method converts digest to hex string:
```cpp
std::stringstream ss;
ss << std::hex << std::setfill('0') << std::setw(2);
for (auto c : sha1)
    ss << (int)c;
return ss.str();
```

Each byte becomes 2 hex characters, zero-padded to ensure consistent 40-character output.

## Integration Points

### Falcor Core Integration

- **FALCOR_API**: Export macro for DLL/shared library support
- **Macros.h**: Core macro definitions

### STL Integration

- **std::array**: Fixed-size container for message digest
- **std::string**: String type for toString() output
- **std::string_view**: Efficient string parameter passing
- **std::stringstream**: String formatting for hex conversion
- **std::enable_if_t**: SFINAE for template constraints

### Internal Falcor Usage

- **Asset Verification**: Hashing scene files and assets for cache validation
- **Shader Cache**: Hashing shader source for cache key generation
- **Data Integrity**: Verifying data integrity across network/file transfers

## Architecture Patterns

### Streaming Pattern

The SHA1 class implements the streaming pattern, allowing incremental data processing:

```cpp
SHA1 sha1;
sha1.update(part1, len1);
sha1.update(part2, len2);
auto digest = sha1.finalize();
```

This pattern enables:
- Processing data larger than available memory
- Hashing data from streams without buffering entire message
- Pausing and resuming hash computation

### Template Pattern

Template methods provide type-safe interfaces for fundamental types:

```cpp
int value = 42;
sha1.update(value);  // Hashes raw bytes of int
```

Uses SFINAE to restrict to fundamental types, preventing accidental hashing of complex objects.

### Static Factory Pattern

The [`compute()`](Source/Falcor/Utils/CryptoUtils.cpp:105) static method provides a factory-like interface for one-shot hashing:

```cpp
auto digest = SHA1::compute(data, len);
```

This encapsulates the common use case of hashing a complete data block.

### State Machine Pattern

The SHA1 class maintains internal state across updates:

- **mIndex**: Current buffer position
- **mBits**: Total bits processed
- **mState**: Hash state (5 words)
- **mBuf**: Input buffer (64 bytes)

State transitions occur on each [`update()`](Source/Falcor/Utils/CryptoUtils.cpp:43) call, with final state produced by [`finalize()`](Source/Falcor/Utils/CryptoUtils.cpp:80).

## Code Patterns

### Macro-Based Optimization

The implementation uses macros for the 80 rounds of processing:

```cpp
#define SHA1_ROUND_0(v,u,x,y,z,i) \
    z += ((u & (x ^ y)) ^ y) + w[i & 15] + c0 + rol32(v, 5); \
    u = rol32(u, 30);
```

This pattern:
- Reduces code duplication
- Improves compiler optimization opportunities
- Makes the round structure explicit

### Lambda Functions

Local lambda functions encapsulate helper operations:

```cpp
auto rol32 = [](uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
};

auto makeWord = [](const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
};
```

This pattern:
- Keeps helper logic local and self-contained
- Improves code readability
- Avoids polluting class namespace

### Bitwise Operations

Extensive use of bitwise operations for efficient bit manipulation:

```cpp
// Rotate left 32-bit value
(x << n) | (x >> (32 - n))

// Extract byte from word
(mState[i] >> ((3 - j) * 8)) & 0xff

// XOR for message schedule
w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15]
```

### Modular Arithmetic

Circular buffer operations use modulo arithmetic:

```cpp
w[i & 15]  // Equivalent to i % 15 for positive i
```

Bitwise AND is used instead of modulo for efficiency when the modulus is a power of 2.

## Use Cases

### Asset Caching

Hashing scene files and assets for cache validation:

```cpp
auto assetHash = SHA1::compute(assetData, assetSize);
// Use assetHash as cache key
```

### Shader Cache Key Generation

Generating unique keys for shader compilation cache:

```cpp
SHA1 sha1;
sha1.update(shaderSource);
sha1.update(defines);
sha1.update(options);
auto cacheKey = SHA1::toString(sha1.finalize());
```

### Data Integrity Verification

Verifying data integrity across transfers:

```cpp
auto receivedHash = SHA1::compute(receivedData, receivedSize);
if (receivedHash == expectedHash) {
    // Data is intact
}
```

### Stream Processing

Hashing large files without loading entirely into memory:

```cpp
SHA1 sha1;
while (!file.eof()) {
    auto chunk = file.readChunk(CHUNK_SIZE);
    sha1.update(chunk.data(), chunk.size());
}
auto digest = sha1.finalize();
```

### Type-Safe Hashing

Hashing fundamental types with type safety:

```cpp
int a = 42;
float b = 3.14f;
double c = 2.718;
sha1.update(a);
sha1.update(b);
sha1.update(c);
```

## Performance Considerations

### Memory Efficiency

- **Fixed Allocation**: Uses fixed-size buffers (64-byte mBuf, 20-byte MD)
- **No Dynamic Allocation**: Zero heap allocations during operation
- **Small Footprint**: Total state is ~100 bytes (64 + 20 + 16)

### Processing Efficiency

- **Block Processing**: Processes data in 64-byte chunks for efficiency
- **Macro Optimization**: Macros enable compiler optimizations
- **Bitwise Operations**: Bitwise operations are faster than arithmetic
- **Circular Buffer**: Message schedule uses efficient circular buffer

### Cache Performance

- **Local Data**: All state fits in cache lines
- **Sequential Access**: Processes data sequentially
- **Small Working Set**: Minimal data accessed per round

### Streaming Benefits

- **Incremental Processing**: Can process data as it arrives
- **No Full Buffer**: Doesn't require entire message in memory
- **Early Termination**: Can stop processing early if needed

### Optimization Opportunities

- **SIMD**: Rounds could be vectorized with SIMD instructions
- **Parallel**: Multiple hashes could be computed in parallel
- **Hardware Acceleration**: Could use CPU SHA extensions

## Limitations

### Security Considerations

- **SHA-1 Deprecated**: SHA-1 is cryptographically broken, not suitable for security-critical applications
- **Collision Attacks**: Practical collision attacks exist against SHA-1
- **Not for Security**: Use SHA-256 or SHA-3 for security purposes

### Functional Limitations

- **160-bit Output**: Limited to 160-bit output (20 bytes)
- **No HMAC**: Doesn't implement HMAC construction
- **Single Algorithm**: Only implements SHA-1, not other hash functions
- **No Salt**: Doesn't support salting for password hashing

### Platform Limitations

- **Big-Endian**: Assumes big-endian byte order for word construction
- **32-bit Words**: Uses 32-bit arithmetic (not 64-bit optimized)
- **No GPU**: No GPU implementation for parallel hashing

### API Limitations

- **No Reset**: Can't reset hash state after finalize (must create new instance)
- **No Clone**: Can't clone hash state for branching computations
- **No Partial Output**: Can't get partial hash before finalization

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Cache key generation (non-security)
- Data integrity verification (non-adversarial)
- Asset fingerprinting
- Deduplication identification

**Inappropriate Use Cases**:
- Password storage (use bcrypt/Argon2)
- Digital signatures (use SHA-256+)
- Cryptographic authentication (use HMAC with SHA-256+)
- Security-critical applications

### Usage Patterns

**Streaming for Large Data**:
```cpp
SHA1 sha1;
for (auto chunk : dataStream) {
    sha1.update(chunk.data(), chunk.size());
}
auto digest = sha1.finalize();
```

**One-Shot for Small Data**:
```cpp
auto digest = SHA1::compute(data, len);
```

**Type-Safe Hashing**:
```cpp
sha1.update(intValue);
sha1.update(floatValue);
```

### Error Handling

- **Null Pointer**: [`update()`](Source/Falcor/Utils/CryptoUtils.cpp:49) returns early if data is null
- **Empty Data**: Hashing empty data produces valid digest
- **Large Data**: No size limits (uses 64-bit bit counter)

### Performance Tips

- **Batch Updates**: Update with larger chunks when possible
- **Avoid Small Updates**: Minimize single-byte updates
- **Reuse Instances**: Reuse SHA1 instances for multiple hashes
- **Prefer compute()**: Use static [`compute()`](Source/Falcor/Utils/CryptoUtils.cpp:105) for one-shot hashing

### Integration Guidelines

- **Cache Keys**: Use [`toString()`](Source/Falcor/Utils/CryptoUtils.cpp:112) output for string-based cache keys
- **Binary Keys**: Use raw MD array for binary cache keys
- **File Verification**: Hash entire file for integrity checks
- **Shader Caching**: Hash source + defines + options for cache key

## Implementation Notes

### Standard Compliance

The implementation follows RFC 3174 (US Secure Hash Algorithm 1) specification:

- Standard initial values
- Correct padding scheme
- Proper round functions
- Accurate message schedule expansion

### Endianness

The implementation assumes big-endian byte order for word construction:

```cpp
auto makeWord = [](const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
};
```

This matches the SHA-1 specification's big-endian requirements.

### Bit Counting

The implementation counts bits (not bytes) for message length:

```cpp
mBits += 8;  // One byte = 8 bits
mBits += sizeof(mBuf) * 8;  // 64 bytes = 512 bits
```

This matches the SHA-1 specification's bit-length encoding.

### Clang Format

The implementation uses `// clang-format off` and `// clang-format on` around the macro-based round processing to preserve the specific formatting of the 80 round calls.

### Thread Safety

The SHA1 class is **not thread-safe** for concurrent updates to the same instance. Each thread should use its own SHA1 instance or synchronize access.

### Exception Safety

The implementation provides basic exception safety:
- No dynamic allocation (no bad_alloc exceptions)
- No external dependencies (no file/network exceptions)
- Minimal state changes (strong exception safety for update operations)

## Future Enhancements

### Potential Improvements

1. **SHA-256 Support**: Add SHA-256 implementation for better security
2. **HMAC Support**: Add HMAC construction for authentication
3. **SIMD Optimization**: Use CPU SHA extensions for acceleration
4. **GPU Implementation**: Add CUDA/OpenCL implementation for parallel hashing
5. **Reset Method**: Add reset() to reuse instances without reallocation
6. **Clone Method**: Add clone() for branching hash computations
7. **Partial Output**: Add method to get partial hash state

### API Extensions

1. **Streaming Interface**: Add iterator-based interface for STL containers
2. **File Interface**: Add direct file hashing interface
3. **Progress Callback**: Add callback for progress reporting
4. **Batch Hashing**: Add interface for hashing multiple items
5. **Hash Combining**: Add method to combine multiple hashes

### Performance Enhancements

1. **SIMD Intrinsics**: Use AVX/AVX2 SHA extensions
2. **Parallel Processing**: Multi-threaded hashing for large data
3. **Memory Mapping**: Use memory-mapped files for large file hashing
4. **Pipeline Optimization**: Optimize instruction pipeline

## References

### Standards

- **RFC 3174**: US Secure Hash Algorithm 1 (SHA-1)
- **FIPS 180-4**: Secure Hash Standard (SHS)

### Algorithm Details

- **NIST SHA-1 Specification**: Official algorithm description
- **Wikipedia SHA-1**: General overview and history
- **Cryptanalysis**: SHA-1 collision attacks and security analysis

### Related Algorithms

- **SHA-256**: Successor to SHA-1 with 256-bit output
- **SHA-3**: Latest member of SHA family
- **MD5**: Predecessor to SHA-1 (also broken)
- **HMAC**: Keyed-hashing for message authentication
