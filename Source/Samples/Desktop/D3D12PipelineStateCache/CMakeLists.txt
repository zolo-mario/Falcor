add_falcor_executable(D3D12PipelineStateCache)

target_sources(D3D12PipelineStateCache PRIVATE
    D3D12PipelineStateCache.cpp
    D3D12PipelineStateCache.h
    Cube.slang
    PostEffect.slang
)

target_copy_shaders(D3D12PipelineStateCache Samples/Desktop/D3D12PipelineStateCache)

target_source_group(D3D12PipelineStateCache "Samples")
