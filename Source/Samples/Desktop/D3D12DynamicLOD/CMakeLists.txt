add_falcor_executable(D3D12DynamicLOD)

target_sources(D3D12DynamicLOD PRIVATE
    D3D12DynamicLOD.cpp
    D3D12DynamicLOD.h
)

target_copy_shaders(D3D12DynamicLOD Samples/D3D12DynamicLOD)

target_source_group(D3D12DynamicLOD "Samples")
