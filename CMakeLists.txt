cmake_minimum_required(VERSION 3.16)

project(nitro_enclave_sum5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(parent)
add_subdirectory(enclave)
