cmake_minimum_required(VERSION 3.14)

project(MiniRenderer)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CUDA_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.2b
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.20
)
FetchContent_MakeAvailable(SDL3)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    message(FATAL_ERROR "Failed to download Dear ImGui")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

set(GLM_DIR ${glm_SOURCE_DIR})
include_directories(${GLM_DIR})

# Include ImGui source files
set(IMGUI_DIR "${imgui_SOURCE_DIR}")
set(IMGUI_BACKENDS_DIR "${IMGUI_DIR}/backends")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_sdlrenderer3.cpp
)

link_libraries(SDL3::SDL3)

add_executable(MiniRenderer ${SOURCES}
    # src/cuda_transform_kernel.cu
)
target_sources(MiniRenderer PRIVATE ${IMGUI_SOURCES})

find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
    target_link_libraries(MiniRenderer PRIVATE OpenMP::OpenMP_CXX)
endif()

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
