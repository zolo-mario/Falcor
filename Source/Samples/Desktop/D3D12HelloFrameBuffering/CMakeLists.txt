add_plugin(D3D12HelloFrameBuffering)

target_sources(D3D12HelloFrameBuffering PRIVATE
    D3D12HelloFrameBuffering.cpp
    D3D12HelloFrameBuffering.h
    HelloFrameBuffering.slang
)

target_copy_shaders(D3D12HelloFrameBuffering Samples/Desktop/D3D12HelloFrameBuffering)

target_source_group(D3D12HelloFrameBuffering "Samples")
