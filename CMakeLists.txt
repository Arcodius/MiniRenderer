cmake_minimum_required(VERSION 3.10)

project(MiniRenderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# Try to enable CUDA
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CUDA_ENABLED TRUE)
    message(STATUS "CUDA support enabled")
else()
    set(CUDA_ENABLED FALSE)
    message(STATUS "CUDA not found - building without CUDA support")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

# Add CUDA files if CUDA is available
if(CUDA_ENABLED)
    file(GLOB_RECURSE CUDA_SOURCES CONFIGURE_DEPENDS
        src/*.cu
        src/*.cuh
    )
    list(APPEND SOURCES ${CUDA_SOURCES})
endif()

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
)
target_sources(MiniRenderer PRIVATE ${IMGUI_SOURCES})

# Set CUDA properties
if(CUDA_ENABLED)
    set_property(TARGET MiniRenderer PROPERTY CUDA_RUNTIME_LIBRARY Shared)
    set_target_properties(MiniRenderer PROPERTIES CUDA_ARCHITECTURES "61;75;86")
    # Define CUDA_ENABLED macro when CUDA is available
    target_compile_definitions(MiniRenderer PRIVATE CUDA_ENABLED)
    # Suppress GLM warnings in CUDA code
    target_compile_options(MiniRenderer PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)
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
