cmake_minimum_required(VERSION 3.16)
project(minidb LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Engine library 
add_library(engine STATIC
  src/engine/engine_core.cpp
  src/engine/engine_api.cpp
)
target_include_directories(engine PUBLIC src/engine)

# Server executable
add_executable(server src/server.cpp)
target_include_directories(server PRIVATE third_party src/engine)
target_link_libraries(server PRIVATE engine)

# copy web-dist into executable dir after build 
add_custom_command(TARGET server POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/web-dist $<TARGET_FILE_DIR:server>/web-dist
)
