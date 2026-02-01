add_plugin(MeshletCulling)

target_sources(MeshletCulling PRIVATE
    MeshletCulling.h
    MeshletCulling.cpp
    MeshletTypes.slang
    MeshletCulling.cs.slang
    MeshletRaster.slang
    MeshletVBufferVisualize.cs.slang
)

target_link_libraries(MeshletCulling PRIVATE meshoptimizer)

target_copy_shaders(MeshletCulling RenderPasses/MeshletCulling)

target_source_group(MeshletCulling "RenderPasses")
