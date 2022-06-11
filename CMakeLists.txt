cmake_minimum_required(VERSION 2.8)
project(haro)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

#include paths
include_directories(src/include)

#source file here
aux_source_directory (src/common LIB_SRC_COMMON)

add_library(haro SHARED ${LIB_SRC_COMMON})

add_executable(test tests/test.cc)
add_dependencies(test haro)
target_link_libraries(test haro)

#${PROJECT_SOURCE_DIR} is the dir contains CMakeLists.txt
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)