add_plugin(MeshletRender)

target_sources(MeshletRender PRIVATE
    MeshletRender.cpp
    MeshletRender.h
    MeshletRenderBunny.ms.slang
)

target_copy_shaders(MeshletRender Samples/Desktop/MeshletRender)

target_source_group(MeshletRender "Samples")
