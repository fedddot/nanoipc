cmake_minimum_required(VERSION 3.16)

project(example_proto C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/nanoipc_build EXCLUDE_FROM_ALL)

find_package(Python REQUIRED)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

add_custom_command(
    OUTPUT ${PROTO_GEN_DIR}/example.pb.c ${PROTO_GEN_DIR}/example.pb.h
    DEPENDS ${PROTO_SRC_DIR}/example.proto
    COMMAND ${Python_EXECUTABLE} ${nanopb_SOURCE_DIR}/generator/nanopb_generator.py -I ${PROTO_SRC_DIR} -D ${PROTO_GEN_DIR} ${PROTO_SRC_DIR}/example.proto
    COMMENT "Generating nanopb stubs from example.proto"
)

add_custom_target(
    generate_example_api_pb
    DEPENDS ${PROTO_GEN_DIR}/example.pb.c ${PROTO_GEN_DIR}/example.pb.h
)

add_library(example_api_pb INTERFACE)
add_dependencies(example_api_pb generate_example_api_pb)
target_sources(example_api_pb INTERFACE ${PROTO_GEN_DIR}/example.pb.c)
target_include_directories(example_api_pb INTERFACE ${PROTO_GEN_DIR})
target_link_libraries(example_api_pb INTERFACE nanopb_common)

add_library(example_api INTERFACE)
target_include_directories(example_api INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(example_request_writer INTERFACE)
target_include_directories(example_request_writer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(example_request_writer INTERFACE example_api_pb nanopb_serializer example_api)

add_library(example_request_reader INTERFACE)
target_include_directories(example_request_reader INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(example_request_reader INTERFACE example_api_pb nanopb_parser example_api)

add_library(example_response_writer INTERFACE)
target_include_directories(example_response_writer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(example_response_writer INTERFACE example_api_pb nanopb_serializer example_api)

add_library(example_response_reader INTERFACE)
target_include_directories(example_response_reader INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(example_response_reader INTERFACE example_api_pb nanopb_parser example_api)
