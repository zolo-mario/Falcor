add_plugin(TinyRaytracer)

target_sources(TinyRaytracer PRIVATE
    TinyRaytracer.h
    TinyRaytracer.cpp
    src/CpuColorBuffer.h
    src/CpuColorBuffer.cpp
    src/RayTracer.h
    src/RayTracer.cpp
)

target_copy_shaders(TinyRaytracer Samples/GraphicsGems/TinyRaytracer)

target_source_group(TinyRaytracer "Samples/GraphicsGems")
