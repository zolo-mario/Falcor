# BC4Encode - BC4 Texture Compression Encoder

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- None (standalone header-only implementation)

## Module Overview

BC4Encode is a BC4 texture compression encoder that compresses 4x4 blocks of alpha values into 64-bit BC4 format. The implementation is derived from libsquish's alpha compression algorithm and supports both 5-alpha and 7-alpha interpolation modes to achieve optimal compression quality.

## Component Specifications

### CompressAlphaDxt5 Function

**Signature**: `static void CompressAlphaDxt5(uint8_t* tile, void* block)`

**Purpose**: Encodes a 4x4 set of uint8 alpha values into a single 64-bit BC4 encoded block

**Parameters**:
- `tile`: Pointer to 16 uint8 values (4x4 alpha tile)
- `block`: Pointer to 8-byte buffer for BC4 compressed output

**Algorithm**:
1. Compute min/max ranges for both 5-alpha and 7-alpha modes
2. For 5-alpha mode: exclude 0 and 255 from range calculation
3. For 7-alpha mode: include all values in range calculation
4. Fix ranges to ensure minimum interpolation steps (5 for 5-alpha, 7 for 7-alpha)
5. Generate codebooks for both modes:
   - 5-alpha: 2 endpoints + 3 interpolated values + 2 fixed values (0, 255)
   - 7-alpha: 2 endpoints + 5 interpolated values
6. Fit alpha values to both codebooks using least-squares error
7. Select the mode with minimum total error
8. Write the compressed block using the selected mode

**Output Format**:
- Bytes 0-1: Alpha endpoint values (alpha0, alpha1)
- Bytes 2-7: 16 3-bit indices (packed into 6 bytes)

### FixRange Function

**Signature**: `static void FixRange(int& min, int& max, int steps)`

**Purpose**: Adjusts min/max range to ensure minimum interpolation steps

**Parameters**:
- `min`: Reference to minimum value (in/out)
- `max`: Reference to maximum value (in/out)
- `steps`: Minimum number of interpolation steps required

**Algorithm**:
1. If range is less than required steps, increase max to min + steps (capped at 255)
2. If still insufficient, decrease min to max - steps (capped at 0)

**Use Case**: Ensures valid interpolation for both 5-alpha and 7-alpha modes

### FitCodes Function

**Signature**: `static int FitCodes(uint8_t const* tile, uint8_t const* codes, uint8_t* indices)`

**Purpose**: Fits alpha values to a codebook and computes total error

**Parameters**:
- `tile`: Pointer to 16 alpha values
- `codes`: Pointer to 8 codebook values
- `indices`: Output array for 16 3-bit indices

**Returns**: Total squared error

**Algorithm**:
1. For each of 16 alpha values:
   - Find the codebook value with minimum squared error
   - Store the corresponding index
   - Accumulate the error
2. Return total error

**Complexity**: O(16 * 8) = O(128) operations per call

### WriteAlphaBlock Function

**Signature**: `static void WriteAlphaBlock(int alpha0, int alpha1, uint8_t const* indices, void* block)`

**Purpose**: Writes alpha endpoints and packed indices to BC4 block

**Parameters**:
- `alpha0`: First endpoint value
- `alpha1`: Second endpoint value
- `indices`: Array of 16 3-bit indices
- `block`: Output buffer for BC4 block (8 bytes)

**Algorithm**:
1. Write alpha0 and alpha1 to first 2 bytes
2. Pack 16 3-bit indices into 6 bytes:
   - Pack 8 indices into 24 bits (3 bytes)
   - Repeat for remaining 8 indices
   - Store as little-endian 3-byte groups

**Bit Packing**:
- Each index: 3 bits (0-7)
- 8 indices: 24 bits (3 bytes)
- Total: 48 bits (6 bytes) for 16 indices

### WriteAlphaBlock5 Function

**Signature**: `static void WriteAlphaBlock5(int alpha0, int alpha1, uint8_t const* indices, void* block)`

**Purpose**: Writes 5-alpha interpolation mode block with index swapping

**Parameters**: Same as WriteAlphaBlock

**Algorithm**:
1. If alpha0 > alpha1:
   - Swap indices according to 5-alpha mode rules:
     - Index 0 → 1
     - Index 1 → 0
     - Index 2-5 → 7 - index
     - Index 6-7 → unchanged
   - Write swapped block with swapped endpoints
2. Otherwise, write block as-is

**Index Mapping** (when alpha0 > alpha1):
- 0 ↔ 1 (swap)
- 2 ↔ 5
- 3 ↔ 4
- 6, 7 (unchanged)

### WriteAlphaBlock7 Function

**Signature**: `static void WriteAlphaBlock7(int alpha0, int alpha1, uint8_t const* indices, void* block)`

**Purpose**: Writes 7-alpha interpolation mode block with index swapping

**Parameters**: Same as WriteAlphaBlock

**Algorithm**:
1. If alpha0 < alpha1:
   - Swap indices according to 7-alpha mode rules:
     - Index 0 → 1
     - Index 1 → 0
     - Index 2-8 → 9 - index
   - Write swapped block with swapped endpoints
2. Otherwise, write block as-is

**Index Mapping** (when alpha0 < alpha1):
- 0 ↔ 1 (swap)
- 2 ↔ 7
- 3 ↔ 6
- 4 ↔ 5
- 8 (unchanged)

## Technical Details

### BC4 Block Format

```
Byte 0:    Alpha0 endpoint
Byte 1:    Alpha1 endpoint
Bytes 2-4: Indices 0-7 (3 bits each, 24 bits)
Bytes 5-7: Indices 8-15 (3 bits each, 24 bits)
```

### 5-Alpha Interpolation

**Codebook Generation**:
```
codes[0] = min
codes[1] = max
codes[2] = (4*min + 1*max) / 5
codes[3] = (3*min + 2*max) / 5
codes[4] = (2*min + 3*max) / 5
codes[5] = (1*min + 4*max) / 5
codes[6] = 0
codes[7] = 255
```

**Characteristics**:
- Uses 5 interpolated values between min and max
- Includes fixed 0 and 255 values for better coverage
- Excludes 0 and 255 from range calculation
- Better for data with extreme values

### 7-Alpha Interpolation

**Codebook Generation**:
```
codes[0] = min
codes[1] = max
codes[2] = (6*min + 1*max) / 7
codes[3] = (5*min + 2*max) / 7
codes[4] = (4*min + 3*max) / 7
codes[5] = (3*min + 4*max) / 7
codes[6] = (2*min + 5*max) / 7
codes[7] = (1*min + 6*max) / 7
```

**Characteristics**:
- Uses 7 interpolated values between min and max
- No fixed values
- Includes all values in range calculation
- Better for data with smooth gradients

### Error Calculation

**Squared Error Formula**:
```
error = Σ (original_value - codebook_value)²
```

**Total Error**:
```
total_error = Σ error for all 16 pixels
```

**Mode Selection**:
```
if (error5 <= error7)
    use 5-alpha mode
else
    use 7-alpha mode
```

## Integration Points

### Usage in Volume Module

The BC4Encode component is used by the Volume module to compress volume data:

1. **GridVolume**: Uses BC4 compression for volumetric data storage
2. **BrickedGrid**: Applies BC4 compression to brick data
3. **GridConverter**: Converts between different volume formats using BC4

### Integration Pattern

```cpp
// Example usage pattern
uint8_t alphaTile[16];  // 4x4 alpha values
uint8_t bc4Block[8];    // Compressed output

CompressAlphaDxt5(alphaTile, bc4Block);
```

### Data Flow

```
Raw Volume Data → 4x4 Tile Extraction → BC4Encode → Compressed Bricks → GPU Texture
```

## Architecture Patterns

### Strategy Pattern

The encoder implements a strategy pattern by selecting between 5-alpha and 7-alpha interpolation modes based on error metrics.

### Template Method Pattern

The compression algorithm follows a template method:
1. Compute ranges
2. Generate codebooks
3. Fit data to codebooks
4. Select best mode
5. Write compressed block

### Bit Packing Pattern

Efficient bit packing for storing 16 3-bit indices in 6 bytes, maximizing storage density.

## Code Patterns

### Range Calculation Pattern

```cpp
int min = 255;
int max = 0;
for (int i = 0; i < 16; ++i) {
    int value = tile[i];
    if (value < min) min = value;
    if (value > max) max = value;
}
```

### Codebook Generation Pattern

```cpp
uint8_t codes[8];
codes[0] = min;
codes[1] = max;
for (int i = 1; i < steps; ++i) {
    codes[1 + i] = ((steps - i) * min + i * max) / steps;
}
```

### Index Swapping Pattern

```cpp
if (alpha0 > alpha1) {
    for (int i = 0; i < 16; ++i) {
        uint8_t index = indices[i];
        if (index == 0) swapped[i] = 1;
        else if (index == 1) swapped[i] = 0;
        else swapped[i] = 7 - index;
    }
}
```

## Use Cases

### Volume Data Compression

Primary use case is compressing volumetric data for GPU rendering:
- Medical imaging data
- Scientific visualization data
- Atmospheric effects
- Volumetric lighting

### Memory Optimization

Reducing memory footprint of volume data:
- 50% compression ratio (16 bytes → 8 bytes)
- Hardware-accelerated decompression on GPU
- Random access to compressed data

### Performance Optimization

Improving rendering performance:
- Reduced memory bandwidth
- Better cache utilization
- Hardware texture compression support

## Performance Considerations

### Compression Performance

- **Complexity**: O(N) where N = number of 4x4 tiles
- **Operations per tile**: ~200-300 integer operations
- **Memory Access**: Sequential reads from tile, sequential writes to block
- **Branching**: Minimal (only mode selection and endpoint comparison)

### Memory Footprint

- **Input**: 16 bytes per 4x4 tile
- **Output**: 8 bytes per BC4 block
- **Compression Ratio**: 2:1 (50% reduction)
- **Temporary Storage**: ~32 bytes per tile (codebooks, indices)

### Optimization Opportunities

1. **SIMD**: Vectorize error calculation and codebook generation
2. **Parallel Processing**: Process multiple tiles in parallel
3. **Lookup Tables**: Precompute interpolation weights
4. **Early Exit**: Skip compression for uniform tiles

## Limitations

### Format Limitations

- **Fixed Block Size**: Only supports 4x4 blocks
- **Single Channel**: Only compresses alpha/luminance data
- **Lossy Compression**: Introduces quantization error
- **Limited Precision**: 8-bit input/output

### Algorithm Limitations

- **Local Optimization**: Only considers 4x4 blocks independently
- **No Temporal Coherence**: Doesn't exploit frame-to-frame coherence
- **No Adaptive Quantization**: Fixed interpolation modes
- **No Rate Control**: Cannot target specific quality levels

### Quality Limitations

- **Quantization Error**: Limited to 8 codebook values
- **Interpolation Artifacts**: May introduce banding in smooth gradients
- **Edge Artifacts**: May blur sharp transitions

## Best Practices

### When to Use BC4Encode

1. **Volume Data**: Ideal for volumetric data with single-channel values
2. **Memory-Constrained**: Use when memory bandwidth is a bottleneck
3. **GPU Rendering**: Leverage hardware texture compression support
4. **Static Data**: Best for data that doesn't change frequently

### When to Avoid BC4Encode

1. **Multi-Channel Data**: Use BC5/BC7 for multi-channel data
2. **High Precision**: Use uncompressed formats for high-precision requirements
3. **Dynamic Data**: Consider alternative compression for frequently updated data
4. **Critical Quality**: Use lossless compression for quality-critical applications

### Usage Guidelines

1. **Tile Alignment**: Ensure data is aligned to 4x4 tile boundaries
2. **Range Awareness**: Be aware of 5-alpha mode's exclusion of 0 and 255
3. **Error Tolerance**: Accept quantization error as trade-off for compression
4. **Hardware Support**: Verify GPU support for BC4 format

## Notes

- Derived from libsquish alpha.cpp (Copyright 2006 Simon Brown)
- Header-only implementation for easy integration
- No external dependencies
- Static functions for namespace isolation
- Integer-only arithmetic for determinism
- Little-endian byte order assumed
- BC4 format is also known as DXT5A or RGTC1
