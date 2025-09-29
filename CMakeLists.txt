cmake_minimum_required(VERSION 3.20)
project(syncbone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SYNCBONE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/syncbone/sync.hpp
)
set(SYNCBONE_SOURCES
    src/sync.cpp
)

add_library(syncbone_lib ${SYNCBONE_HEADERS} ${SYNCBONE_SOURCES})
target_include_directories(syncbone_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(syncbone
    src/main.cpp
)
target_link_libraries(syncbone PRIVATE syncbone_lib)

# --- Tests ---
include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
