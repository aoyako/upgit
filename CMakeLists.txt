cmake_minimum_required(VERSION 3.14)
project(upgit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")

add_executable(upgit upgit.cpp)

find_program(CLANG_TIDY NAMES "clang-tidy")
if(CLANG_TIDY)
    set_target_properties(upgit PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY}")
endif()

install(TARGETS upgit DESTINATION bin)
