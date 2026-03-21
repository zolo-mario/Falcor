add_plugin(TinyRenderer)

target_sources(TinyRenderer PRIVATE
    TinyRenderer.h
    TinyRenderer.cpp
    src/TriangleRaster.h
    src/TriangleRaster.cpp
    src/CpuPipeline.h
    src/CpuPipeline.cpp
)

target_copy_shaders(TinyRenderer Samples/GraphicsGems/TinyRenderer)

target_source_group(TinyRenderer "Samples/GraphicsGems")
