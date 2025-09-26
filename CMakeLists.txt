cmake_minimum_required(VERSION 3.20)
project(syncbone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем include/
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(syncbone
    src/main.cpp
)
