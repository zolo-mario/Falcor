add_plugin(D3D12MeshletRender)

target_sources(D3D12MeshletRender PRIVATE
    D3D12MeshletRender.cpp
    D3D12MeshletRender.h
    MeshletRender.ms.slang
)

target_copy_shaders(D3D12MeshletRender Samples/Desktop/D3D12MeshShaders/MeshletRender)

target_source_group(D3D12MeshletRender "Samples")
