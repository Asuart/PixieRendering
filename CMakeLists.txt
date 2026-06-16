cmake_minimum_required(VERSION 3.22)

project(PixieRendering VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# settings for dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT OFF CACHE BOOL "Disable SPIR-V optimisation" FORCE)
add_subdirectory(Dependencies)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE _source_list "PixieRendering/*.c" "PixieRendering/*.cpp" "PixieRendering/*.h" "PixieRendering/*.hpp")

add_library(PixieRendering STATIC ${_source_list})

target_compile_features(PixieRendering PUBLIC cxx_std_23)

target_include_directories(PixieRendering PUBLIC "/")
target_include_directories(PixieRendering PUBLIC "${glm_SOURCE_DIR}/")

target_link_libraries(PixieRendering PRIVATE glad)
target_link_libraries(PixieRendering PRIVATE glfw)
target_link_libraries(PixieRendering PRIVATE glslang)
target_link_libraries(PixieRendering PRIVATE glslang-default-resource-limits)
target_link_libraries(PixieRendering PRIVATE Vulkan::Vulkan)

if (MSVC)
    target_compile_options(PixieRendering PRIVATE /W4 /WX)
else()
    target_compile_options(PixieRendering PRIVATE -Wall -Werror)
endif()

foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}/PixieRendering" "" _group_path "${_source_path}")
    string(REPLACE "../" "" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()
