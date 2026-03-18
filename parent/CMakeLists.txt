find_package(Protobuf REQUIRED)

set(SUM5_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../enclave/proto)
set(SUM5_PROTO_FILE ${SUM5_PROTO_DIR}/sum5.proto)
set(SUM5_PARENT_PROTO_OUT ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${SUM5_PARENT_PROTO_OUT})

add_library(sum5_parent_proto ${SUM5_PROTO_FILE})
target_link_libraries(sum5_parent_proto PUBLIC protobuf::libprotobuf)
target_include_directories(sum5_parent_proto PUBLIC
    ${SUM5_PARENT_PROTO_OUT}
    ${SUM5_PARENT_PROTO_OUT}/proto
)

protobuf_generate(
    TARGET sum5_parent_proto
    LANGUAGE cpp
    PROTOS ${SUM5_PROTO_FILE}
    IMPORT_DIRS ${SUM5_PROTO_DIR}
    PROTOC_OUT_DIR ${SUM5_PARENT_PROTO_OUT}
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
