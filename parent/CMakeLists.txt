cmake_minimum_required(VERSION 3.16)

project(sum5_parent_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf REQUIRED)

set(SUM5_SHARED_PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../enclave/proto/sum5.proto)
set(SUM5_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
set(SUM5_PROTO_FILE ${SUM5_PROTO_DIR}/sum5.proto)
set(SUM5_PARENT_PROTO_OUT ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(SUM5_PARENT_PROTO_SRC ${SUM5_PARENT_PROTO_OUT}/sum5.pb.cc)
set(SUM5_PARENT_PROTO_HDR ${SUM5_PARENT_PROTO_OUT}/sum5.pb.h)

file(MAKE_DIRECTORY ${SUM5_PROTO_DIR})
file(MAKE_DIRECTORY ${SUM5_PARENT_PROTO_OUT})
configure_file(${SUM5_SHARED_PROTO_FILE} ${SUM5_PROTO_FILE} COPYONLY)

add_custom_command(
    OUTPUT ${SUM5_PARENT_PROTO_SRC} ${SUM5_PARENT_PROTO_HDR}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        --proto_path=${SUM5_PROTO_DIR}
        --cpp_out=${SUM5_PARENT_PROTO_OUT}
        ${SUM5_PROTO_FILE}
    DEPENDS ${SUM5_PROTO_FILE}
    VERBATIM
)

add_library(sum5_parent_proto ${SUM5_PARENT_PROTO_SRC} ${SUM5_PARENT_PROTO_HDR})
target_link_libraries(sum5_parent_proto PUBLIC protobuf::libprotobuf)
target_include_directories(sum5_parent_proto PUBLIC
    ${SUM5_PARENT_PROTO_OUT}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(sum5_parent
        src/main.cpp
    )
else()
    add_executable(sum5_parent
        src/main_stub.cpp
    )
endif()

target_link_libraries(sum5_parent PRIVATE sum5_parent_proto)

target_compile_options(sum5_parent PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
