add_plugin(D3D12ExecuteIndirect)

target_sources(D3D12ExecuteIndirect PRIVATE
    D3D12ExecuteIndirect.cpp
    D3D12ExecuteIndirect.h
    Shaders.slang
    ShadersCulling.slang
    Compute.slang
    BuildArgs.slang
)

target_copy_shaders(D3D12ExecuteIndirect Samples/Desktop/D3D12ExecuteIndirect)

target_source_group(D3D12ExecuteIndirect "Samples")
