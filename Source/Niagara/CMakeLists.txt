add_falcor_executable(Niagara)

target_link_libraries(Niagara PRIVATE meshoptimizer)

target_sources(Niagara PRIVATE
    Niagara.cpp
    Niagara.h
    NiagaraConfig.h
    NiagaraScene.cpp
    NiagaraScene.h
    shaders/NiagaraMeshlet.ms.slang
)

target_copy_shaders(Niagara Samples/Niagara)

target_source_group(Niagara "Samples")