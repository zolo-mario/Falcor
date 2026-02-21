add_plugin(MeshletInstancing)

target_sources(MeshletInstancing PRIVATE
    MeshletInstancing.cpp
    MeshletInstancing.h
    MeshletInstancing.ms.slang
)

target_copy_shaders(MeshletInstancing Samples/Desktop/D3D12MeshShaders/MeshletInstancing)

target_source_group(MeshletInstancing "Samples")
