cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(nspattern)

add_library(nspattern INTERFACE)
target_include_directories(nspattern INTERFACE "include")

option(NSPATTERN_BUILD_TEST "Build nspattern test executable" OFF)

if (NSPATTERN_BUILD_TEST)
    add_executable(nspattern_test "test.cpp")
    set_property(TARGET nspattern_test PROPERTY CXX_STANDARD 20)
    target_link_libraries(nspattern_test nspattern)
endif()
