add_falcor_executable(D3D12MeshletInstancing)

target_sources(D3D12MeshletInstancing PRIVATE
    D3D12MeshletInstancing.cpp
    D3D12MeshletInstancing.h
    MeshletModel.cpp
    MeshletModel.h
)

target_copy_shaders(D3D12MeshletInstancing Samples/Desktop/D3D12MeshletInstancing)

# Copy ToyRobot.bin from DirectX-Graphics-Samples if available
set(DX_SAMPLES_PATH "${CMAKE_SOURCE_DIR}/../DirectX-Graphics-Samples" CACHE PATH "Path to DirectX-Graphics-Samples")
if(EXISTS "${DX_SAMPLES_PATH}/Samples/Desktop/D3D12MeshShaders/Assets/ToyRobot.bin")
    add_custom_command(TARGET D3D12MeshletInstancing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DX_SAMPLES_PATH}/Samples/Desktop/D3D12MeshShaders/Assets/ToyRobot.bin"
            "$<TARGET_FILE_DIR:D3D12MeshletInstancing>/ToyRobot.bin"
        COMMENT "Copying ToyRobot.bin from DirectX-Graphics-Samples"
    )
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data/ToyRobot.bin")
    add_custom_command(TARGET D3D12MeshletInstancing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/data/ToyRobot.bin"
            "$<TARGET_FILE_DIR:D3D12MeshletInstancing>/ToyRobot.bin"
        COMMENT "Copying ToyRobot.bin from sample data"
    )
endif()

target_source_group(D3D12MeshletInstancing "Samples")
