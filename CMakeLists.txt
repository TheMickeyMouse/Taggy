cmake_minimum_required(VERSION 4.0)
project(Taggy)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(Quasi)
add_executable(Taggy main.cpp
        TagReader.cpp
        TagReader.h
        nlohmann-json.h)
target_link_libraries(Taggy Quasi)