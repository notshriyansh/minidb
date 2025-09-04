cmake_minimum_required(VERSION 3.10)
project(minidb)

set(CMAKE_CXX_STANDARD 17)

# Add include directories
include_directories(src)

# Add executable
add_executable(minidb
    src/server.cpp
    src/engine/engine_core.cpp
    src/engine/engine_api.cpp
)

# External libraries (if you drop httplib.h + json.hpp in src/libs)
target_include_directories(minidb PRIVATE src/libs)
