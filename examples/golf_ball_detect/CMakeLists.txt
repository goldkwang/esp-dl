# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

add_compile_options(-fdiagnostics-color=always)

# Optimize for size to save memory
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")


# Generate version info
set(VERSION_NUMBER 5)
add_definitions(-DVERSION_STRING="v${VERSION_NUMBER}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cat_detect)