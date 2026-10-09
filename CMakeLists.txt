cmake_minimum_required(VERSION 4.0)
project(Taggy)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(Quasi)
target_compile_definitions(Quasi PUBLIC Q_NO_IMGUI)

add_executable(Taggy main.cpp
        TagReader.cpp
        TagReader.h
        nlohmann-json.h
        CLI.cpp
        App.cpp
        App.h
        WinUtils.cpp
        WinUtils.h
        UIRect.cpp
        UIRect.h
        Titlebar.cpp
        Titlebar.h
        Sidebar.cpp
        Sidebar.h
        Icon.cpp
        Icon.cpp
        Icon.h
        Smooth.h
        MusicDisplay.cpp
        MusicDisplay.h
        Playbar.cpp
        Playbar.h
        Track.cpp
        Track.h
        miniaudio.h
        miniaudio.cpp)
target_link_libraries(Taggy Quasi dwmapi.dll)