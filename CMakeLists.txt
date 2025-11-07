cmake_minimum_required(VERSION 3.13)

project(hashsets
        VERSION 1.0
        DESCRIPTION "Implementations of concurrent hash sets"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto")

find_package(Threads REQUIRED)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
    add_compile_options(-Werror -Wall -Wextra -pedantic -Weverything)
    add_compile_options(
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-padded
            -Wno-unsafe-buffer-usage
    )
    set(USE_SANITIZER "" CACHE STRING
            "Use the clang sanitizer [asan|tsan]")
    if("${USE_SANITIZER}" STREQUAL "asan")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope)
        add_link_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope)
    elseif("${USE_SANITIZER}" STREQUAL "tsan")
        add_compile_options(-fsanitize=thread)
        add_link_options(-fsanitize=thread)
    elseif(NOT "${USE_SANITIZER}" STREQUAL "")
        message(FATAL_ERROR "Unknown argument to USE_SANITIZER: ${USE_SANITIZER} - options are [asan|tsan]")
    endif()
else()
    message(FATAL_ERROR "The clang compiler is required")
endif()

add_library(checks STATIC
        src/tree_map_base.h
        src/benchmark.h
        src/tree_map_coarse_grained.h
        src/benchmark.cc
        src/demo_coarse_grained.cpp
        src/tree_map_coarse_grained.cpp
        src/tree_map_base.cpp
        src/demo_fine_grained.cpp)
target_include_directories(checks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Update the function to include the necessary source files.
function(add_tree_map_demo name)
    # Define a list of source files required for the demo executable
    set(DEMO_SOURCES
            src/tree_map_base.cpp
            src/benchmark.cc
            src/tree_map_${name}.cpp  # Source file for the map implementation (put/get)
            src/demo_${name}.cpp      # Source file containing the main() function
    )

    add_executable(demo_${name} ${DEMO_SOURCES})

    target_include_directories(demo_${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(demo_${name} PRIVATE Threads::Threads)
endfunction()

# This call now correctly defines the demo_coarse_grained executable
add_tree_map_demo(coarse_grained)
add_tree_map_demo(fine_grained)
#add_executable(playground
#        src/hash_set_base.h
#        src/hash_set_coarse_grained.h
#        src/hash_set_refinable.h
#        src/hash_set_sequential.h
#        src/hash_set_striped.h
#        src/playground.cc)
#target_include_directories(playground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(playground PRIVATE Threads::Threads)
