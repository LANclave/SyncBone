cmake_minimum_required(VERSION 3.20)
project(syncbone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(syncbone
    src/main.cpp
)

# --- Tests ---
include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
