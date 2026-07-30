cmake_minimum_required(VERSION 3.15)
project(first-circle-simple-client CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Compiler Configuration (MSYS2) ---
if (WIN32)
    set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# === Simple Client ===
file(GLOB CLIENT_SOURCES "main.cpp")
add_executable(simple-client ${CLIENT_SOURCES})
target_link_libraries(simple-client ws2_32)
#include_directories(include)

# Enable detailed build output
set(CMAKE_VERBOSE_MAKEFILE ON)