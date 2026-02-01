add_plugin(SceneDebugger)

target_sources(SceneDebugger PRIVATE
    SceneDebugger.cpp
    SceneDebugger.cs.slang
    SceneDebugger.h
    SharedTypes.slang
)

target_link_libraries(SceneDebugger PRIVATE meshoptimizer)

target_copy_shaders(SceneDebugger RenderPasses/SceneDebugger)

target_source_group(SceneDebugger "RenderPasses")
