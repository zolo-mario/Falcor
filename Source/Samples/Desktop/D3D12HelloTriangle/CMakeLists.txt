add_falcor_executable(D3D12HelloTriangle)

target_sources(D3D12HelloTriangle PRIVATE
    D3D12HelloTriangle.cpp
    D3D12HelloTriangle.h
    HelloTriangle.slang
)

target_copy_shaders(D3D12HelloTriangle Samples/Desktop/D3D12HelloTriangle)

target_source_group(D3D12HelloTriangle "Samples")
