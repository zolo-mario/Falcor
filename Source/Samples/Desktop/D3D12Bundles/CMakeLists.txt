add_falcor_executable(D3D12Bundles)

target_sources(D3D12Bundles PRIVATE
    D3D12Bundles.cpp
    D3D12Bundles.h
    MeshSimple.slang
    MeshAlt.slang
)

target_copy_shaders(D3D12Bundles Samples/Desktop/D3D12Bundles)

# Copy occcity.bin from DirectX-Graphics-Samples if available (optional, when repos are siblings)
set(DX_SAMPLES_PATH "${CMAKE_SOURCE_DIR}/../DirectX-Graphics-Samples" CACHE PATH "Path to DirectX-Graphics-Samples")
if(EXISTS "${DX_SAMPLES_PATH}/Samples/Desktop/D3D12Bundles/src/occcity.bin")
    add_custom_command(TARGET D3D12Bundles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DX_SAMPLES_PATH}/Samples/Desktop/D3D12Bundles/src/occcity.bin"
            "$<TARGET_FILE_DIR:D3D12Bundles>/occcity.bin"
        COMMENT "Copying occcity.bin from DirectX-Graphics-Samples"
    )
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data/occcity.bin")
    add_custom_command(TARGET D3D12Bundles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/data/occcity.bin"
            "$<TARGET_FILE_DIR:D3D12Bundles>/occcity.bin"
        COMMENT "Copying occcity.bin from sample data"
    )
endif()

target_source_group(D3D12Bundles "Samples")
