add_falcor_executable(D3D12HelloWorld)

target_sources(D3D12HelloWorld PRIVATE
    D3D12HelloWorld.cpp
    D3D12HelloWorld.h
)

target_copy_shaders(D3D12HelloWorld Samples/D3D12HelloWorld)

target_source_group(D3D12HelloWorld "Samples")
