add_plugin(MeshletCull)

target_sources(MeshletCull PRIVATE
    MeshletCull.cpp
    MeshletCull.h
    MeshletCullBunny.slang
)

target_copy_shaders(MeshletCull Samples/Desktop/D3D12MeshShaders/MeshletCull)

target_source_group(MeshletCull "Samples")
