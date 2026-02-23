add_plugin(Niagara)

target_sources(Niagara PRIVATE
    Niagara.cpp
    Niagara.h
    SceneFormat.h
    FalcorSceneAdapter.h
    FalcorSceneAdapter.cpp
)

target_link_libraries(Niagara PRIVATE meshoptimizer)

target_copy_shaders(Niagara Samples/Niagara)

target_source_group(Niagara "Samples")
