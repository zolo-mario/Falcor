add_plugin(D3D12HelloTexture)

target_sources(D3D12HelloTexture PRIVATE
    D3D12HelloTexture.cpp
    D3D12HelloTexture.h
    HelloTexture.slang
)

target_copy_shaders(D3D12HelloTexture Samples/Desktop/D3D12HelloWorld/HelloTexture)

target_source_group(D3D12HelloTexture "Samples")
