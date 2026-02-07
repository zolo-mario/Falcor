add_plugin(VBufferMeshletRaster)

target_sources(VBufferMeshletRaster PRIVATE
    VBufferMeshletRaster.cpp
    VBufferMeshletRaster.h
    MeshletRaster.cs.slang
    MeshletResolve.cs.slang
)

# Link against GBuffer to get GBufferBase symbols
target_link_libraries(VBufferMeshletRaster PRIVATE meshoptimizer GBuffer)

target_copy_shaders(VBufferMeshletRaster RenderPasses/VBufferMeshletRaster)

target_source_group(VBufferMeshletRaster "RenderPasses")
