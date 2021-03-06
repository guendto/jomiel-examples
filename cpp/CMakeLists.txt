# -*- coding: utf-8 -*-
#
# jomiel-client-demos
#
# Copyright
#  2021 Toni Gündoğdu
#
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.13.4)
project(demo)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Settings.
#
# Set CMAKE_CXX_STANDARD until "How to manage C++ standard
# [EXPERIMENTAL]" solidifies.
#   <https://docs.conan.io/en/latest/howtos/manage_cpp_standard.html>
#
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    # -Wno-error=deprecated-declarations
    -Wall -Werror -march=native -O2 -ggdb3 -pipe
    -Wformat=2 -Wformat-security
    --param=ssp-buffer-size=4
    -fstack-protector-strong
    -Werror=format-security
    -D_FORTIFY_SOURCE=2
    -Wpedantic
    -Wmissing-include-dirs
    -Wwrite-strings
    -Wshadow
    -Wextra
    -Wlogical-op
)

# Helpers.
#   - Namely the useful cmake_print_variables() used everywhere.
#
include(CMakePrintHelpers)

# CMAKE_MODULE_PATH.
#   - Modify so that tools/cmake/*.cmake modules can be used.
#
list(APPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/../tools/cmake)

# Protobuf.
#   - A "cmake-only" alternative to bootstrap script of jomiel-proto.
#
include(InvokeProtoc)

protoc_gen_bindings(cpp
    ${PROJECT_SOURCE_DIR}/../proto
    ${PROJECT_BINARY_DIR}/build/gen)
# cmake_print_variables(PROTOC_GEN_SRCFILES)

# Source files.
#
set(DEMO_SRCFILES
    src/demo/jomiel.cpp
    src/demo/jomiel.h
    src/demo/usage.cpp
    src/demo/main.cpp
)

# Include dirs.
#
include_directories(${PROJECT_BINARY_DIR}/build/gen)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(src)

# Target.
#
add_executable(demo ${DEMO_SRCFILES} ${PROTOC_GEN_SRCFILES})
target_link_libraries(demo ${CONAN_LIBS})
