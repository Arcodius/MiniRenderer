cmake_minimum_required(VERSION 3.10)

project(MiniRenderer)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CUDA_STANDARD 17)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)
# 引入GLM头文件路径
# GLM的根目录在 third_party/glm
# 但实际的头文件在 third_party/glm/glm 里
# 所以我们把 third_party/glm 作为包含路径
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm)
include_directories(${GLM_DIR}) # 使用 include_directories 或 target_include_directories 都可以

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3" EXCLUDE_FROM_ALL)
link_libraries(SDL3::SDL3)

add_executable(MiniRenderer ${SOURCES}
    # src/cuda_transform_kernel.cu
)

target_include_directories(MiniRenderer PRIVATE ${GLM_DIR})

add_custom_command(TARGET MiniRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:MiniRenderer>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Resources
    $<TARGET_FILE_DIR:MiniRenderer>/Resources
)
