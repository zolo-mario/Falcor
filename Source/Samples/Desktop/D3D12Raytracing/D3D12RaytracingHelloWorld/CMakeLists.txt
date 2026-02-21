add_plugin(D3D12RaytracingHelloWorld)

target_sources(D3D12RaytracingHelloWorld PRIVATE
    D3D12RaytracingHelloWorld.cpp
    D3D12RaytracingHelloWorld.h
    D3D12RaytracingHelloWorld.rt.slang
)

target_copy_shaders(D3D12RaytracingHelloWorld Samples/Desktop/D3D12Raytracing/D3D12RaytracingHelloWorld)

target_source_group(D3D12RaytracingHelloWorld "Samples")
