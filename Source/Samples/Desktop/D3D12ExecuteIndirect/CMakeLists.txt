add_falcor_executable(D3D12ExecuteIndirect)

target_sources(D3D12ExecuteIndirect PRIVATE
    D3D12ExecuteIndirect.cpp
    D3D12ExecuteIndirect.h
    Shaders.slang
    Compute.slang
)

target_copy_shaders(D3D12ExecuteIndirect Samples/Desktop/D3D12ExecuteIndirect)

target_source_group(D3D12ExecuteIndirect "Samples")
