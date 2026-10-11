cmake_minimum_required(VERSION 3.20)

project(TerraNex VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

# Геологическая модель не зависит от Qt и может тестироваться отдельно.
add_library(terranex_domain
    src/domain/GeologicalModel.cpp
    src/domain/GeologicalModel.h
)

target_include_directories(terranex_domain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(terranex_domain PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic>
)

qt_add_executable(TerraNex
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(TerraNex PRIVATE
    Qt6::Widgets
    terranex_domain
)

target_compile_options(TerraNex PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic>
)

include(CTest)
if(BUILD_TESTING)
    add_executable(terranex_domain_tests
        tests/domain_model_tests.cpp
    )
    target_link_libraries(terranex_domain_tests PRIVATE terranex_domain)
    add_test(NAME terranex_domain_tests COMMAND terranex_domain_tests)
endif()
