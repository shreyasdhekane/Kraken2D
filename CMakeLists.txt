cmake_minimum_required(VERSION 3.16)
project(Kraken2D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL3_ROOT "C:/SDL3-3.4.4/x86_64-w64-mingw32")
include_directories(${SDL3_ROOT}/include)
link_directories(${SDL3_ROOT}/lib)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(Kraken2D ${SOURCES})
target_link_libraries(Kraken2D SDL3)
