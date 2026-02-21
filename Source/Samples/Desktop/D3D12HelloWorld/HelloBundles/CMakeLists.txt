add_plugin(D3D12HelloBundles)

target_sources(D3D12HelloBundles PRIVATE
    D3D12HelloBundles.cpp
    D3D12HelloBundles.h
    HelloBundles.slang
)

target_copy_shaders(D3D12HelloBundles Samples/Desktop/D3D12HelloWorld/HelloBundles)

target_source_group(D3D12HelloBundles "Samples")
