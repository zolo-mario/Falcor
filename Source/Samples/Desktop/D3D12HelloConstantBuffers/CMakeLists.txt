add_falcor_executable(D3D12HelloConstantBuffers)

target_sources(D3D12HelloConstantBuffers PRIVATE
    D3D12HelloConstantBuffers.cpp
    D3D12HelloConstantBuffers.h
)

target_copy_shaders(D3D12HelloConstantBuffers Samples/Desktop/D3D12HelloConstantBuffers)

target_source_group(D3D12HelloConstantBuffers "Samples")
