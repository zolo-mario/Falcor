add_plugin(D3D12MeshletInstancing)

target_sources(D3D12MeshletInstancing PRIVATE
    D3D12MeshletInstancing.cpp
    D3D12MeshletInstancing.h
    MeshletInstancingBunny.ms.slang
)

target_copy_shaders(D3D12MeshletInstancing Samples/Desktop/D3D12MeshletInstancing)

target_source_group(D3D12MeshletInstancing "Samples")
