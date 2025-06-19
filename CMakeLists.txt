cmake_minimum_required(VERSION 3.10)

project(MiniRenderer)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CUDA_STANDARD 17)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm)
include_directories(${GLM_DIR})

# Include ImGui source files
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui")
set(IMGUI_BACKENDS_DIR "${IMGUI_DIR}/backends")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_sdlrenderer3.cpp
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3" EXCLUDE_FROM_ALL)
link_libraries(SDL3::SDL3)

add_executable(MiniRenderer ${SOURCES}
    # src/cuda_transform_kernel.cu
)
target_sources(MiniRenderer PRIVATE ${IMGUI_SOURCES})

target_include_directories(MiniRenderer PRIVATE 
    ${GLM_DIR} 
    ${IMGUI_DIR} 
    ${IMGUI_BACKENDS_DIR}
)

add_custom_command(TARGET MiniRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:MiniRenderer>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Resources
    $<TARGET_FILE_DIR:MiniRenderer>/Resources
)
