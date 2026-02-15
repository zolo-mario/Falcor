add_falcor_executable(MeshletRender)

target_sources(MeshletRender PRIVATE
    MeshletRender.cpp
    MeshletRender.h
    Model.cpp
    Model.h
    MeshletRender.slang
)

target_copy_shaders(MeshletRender Samples/Desktop/MeshletRender)

# Copy Dragon_LOD0.bin from DirectX-Graphics-Samples if available (run WavefrontConverter to generate)
set(DX_SAMPLES_PATH "${CMAKE_SOURCE_DIR}/../DirectX-Graphics-Samples" CACHE PATH "Path to DirectX-Graphics-Samples")
if(EXISTS "${DX_SAMPLES_PATH}/Samples/Desktop/D3D12MeshShaders/Assets/Dragon_LOD0.bin")
    add_custom_command(TARGET MeshletRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DX_SAMPLES_PATH}/Samples/Desktop/D3D12MeshShaders/Assets/Dragon_LOD0.bin"
            "$<TARGET_FILE_DIR:MeshletRender>/Dragon_LOD0.bin"
        COMMENT "Copying Dragon_LOD0.bin from DirectX-Graphics-Samples"
    )
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data/Dragon_LOD0.bin")
    add_custom_command(TARGET MeshletRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/data/Dragon_LOD0.bin"
            "$<TARGET_FILE_DIR:MeshletRender>/Dragon_LOD0.bin"
        COMMENT "Copying Dragon_LOD0.bin from sample data"
    )
endif()

target_source_group(MeshletRender "Samples")
