cmake_minimum_required(VERSION 3.16)

project(nitro_enclave_sum5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf REQUIRED)

add_library(sum5_proto proto/sum5.proto)
target_link_libraries(sum5_proto PUBLIC protobuf::libprotobuf)
target_include_directories(sum5_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate(
    TARGET sum5_proto
    LANGUAGE cpp
    PROTOS proto/sum5.proto
)

add_executable(sum5_enclave
    src/main_stub.cpp
)

target_link_libraries(sum5_enclave PRIVATE sum5_proto)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(sum5_enclave PRIVATE
        src/main.cpp
    )
else()
    target_compile_definitions(sum5_enclave PRIVATE
        SUM5_ENCLAVE_UNSUPPORTED_PLATFORM=1
    )
endif()

target_compile_options(sum5_enclave PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
