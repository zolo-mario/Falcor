add_plugin(D3D12MeshletCull)

target_sources(D3D12MeshletCull PRIVATE
    D3D12MeshletCull.cpp
    D3D12MeshletCull.h
    MeshletCull.slang
)

target_copy_shaders(D3D12MeshletCull Samples/Desktop/D3D12MeshShaders/MeshletCull)

target_source_group(D3D12MeshletCull "Samples")
