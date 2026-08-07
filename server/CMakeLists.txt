cmake_minimum_required(VERSION 3.15)
project(first-circle-server CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Compiler Configuration (MSYS2) ---
if (WIN32)
    set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# === Server ===
file(GLOB SERVER_SOURCES "main.cpp" "src/*.cpp")
add_executable(server ${SERVER_SOURCES})

target_include_directories(server PRIVATE include)
target_link_libraries(server PRIVATE ws2_32)

# Enable detailed build output
set(CMAKE_VERBOSE_MAKEFILE ON)