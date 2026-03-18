cmake_minimum_required(VERSION 3.16)

project(sum5_enclave_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf REQUIRED)

set(SUM5_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(SUM5_PROTO_FILE ${SUM5_PROTO_DIR}/sum5.proto)
set(SUM5_ENCLAVE_PROTO_OUT ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${SUM5_ENCLAVE_PROTO_OUT})

add_library(sum5_enclave_proto ${SUM5_PROTO_FILE})
target_link_libraries(sum5_enclave_proto PUBLIC protobuf::libprotobuf)
target_include_directories(sum5_enclave_proto PUBLIC ${SUM5_ENCLAVE_PROTO_OUT})

protobuf_generate(
    TARGET sum5_enclave_proto
    LANGUAGE cpp
    PROTOS ${SUM5_PROTO_FILE}
    IMPORT_DIRS ${SUM5_PROTO_DIR}
    PROTOC_OUT_DIR ${SUM5_ENCLAVE_PROTO_OUT}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(sum5_enclave
        src/main.cpp
    )
else()
    add_executable(sum5_enclave
        src/main_stub.cpp
    )
endif()

target_link_libraries(sum5_enclave PRIVATE sum5_enclave_proto)

target_compile_options(sum5_enclave PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
