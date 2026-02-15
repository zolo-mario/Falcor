add_falcor_executable(Niagara)

target_sources(Niagara PRIVATE
    Niagara.cpp
    Niagara.h
    NiagaraScene.cpp
    NiagaraScene.h
)

target_copy_shaders(Niagara Samples/Niagara)

target_source_group(Niagara "Samples")