add_plugin(MeshletRender)

target_sources(MeshletRender PRIVATE
    MeshletRender.cpp
    MeshletRender.h
    MeshletRender.ms.slang
)

target_copy_shaders(MeshletRender Samples/Desktop/D3D12MeshShaders/MeshletRender)

target_source_group(MeshletRender "Samples")
