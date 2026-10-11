cmake_minimum_required(VERSION 3.20)

project(TerraNex VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(TerraNex
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(TerraNex PRIVATE Qt6::Widgets)

target_compile_options(TerraNex PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic>
)
