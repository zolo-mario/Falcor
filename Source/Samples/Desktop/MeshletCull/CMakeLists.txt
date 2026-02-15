add_falcor_executable(MeshletCull)

target_sources(MeshletCull PRIVATE
    MeshletCull.cpp
    MeshletCull.h
    MeshletCull.slang
)

target_copy_shaders(MeshletCull Samples/Desktop/MeshletCull)

target_source_group(MeshletCull "Samples")
