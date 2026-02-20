add_plugin(D3D12HDR)

target_sources(D3D12HDR PRIVATE
    D3D12HDR.cpp
    D3D12HDR.h
    Color.slang
    Gradient.slang
    Palette709.slang
    Palette2020.slang
    Present.slang
)

target_copy_shaders(D3D12HDR Samples/Desktop/D3D12HDR)

target_source_group(D3D12HDR "Samples")
