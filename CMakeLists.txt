cmake_minimum_required(VERSION 3.16)
project(Kraken2D)

set(CMAKE_CXX_STANDARD 17)

# SDL2
set(SDL2_PATH "C:/SDL2")
include_directories(${SDL2_PATH}/include)
link_directories(${SDL2_PATH}/lib)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(Kraken2D ${SOURCES})

target_link_libraries(Kraken2D mingw32 SDL2main SDL2)