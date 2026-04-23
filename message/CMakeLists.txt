FetchContent_Declare(
    nanopb
    GIT_REPOSITORY https://github.com/nanopb/nanopb.git
    GIT_TAG        nanopb-0.4.9
)

FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG        1.9.5
)

FetchContent_MakeAvailable(nanopb jsoncpp)

# Create jsoncpp_lib alias for backward compatibility
if(NOT TARGET jsoncpp_lib)
    if(TARGET jsoncpp_static)
        add_library(jsoncpp_lib ALIAS jsoncpp_static)
    else()
        add_library(jsoncpp_lib ALIAS jsoncpp_object)
    endif()
endif()

add_library(nanopb_common INTERFACE)
target_sources(nanopb_common INTERFACE ${nanopb_SOURCE_DIR}/pb_common.c)
target_include_directories(nanopb_common INTERFACE ${nanopb_SOURCE_DIR})

add_library(nanopb_encode INTERFACE)
target_sources(nanopb_encode INTERFACE ${nanopb_SOURCE_DIR}/pb_encode.c)
target_link_libraries(nanopb_encode INTERFACE nanopb_common)

add_library(nanopb_decode INTERFACE)
target_sources(nanopb_decode INTERFACE ${nanopb_SOURCE_DIR}/pb_decode.c)
target_link_libraries(nanopb_decode INTERFACE nanopb_common)

add_library(pb_message_reader INTERFACE)
target_include_directories(pb_message_reader INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pb_message_reader INTERFACE nanopb_decode reader)

add_library(pb_message_writer INTERFACE)
target_include_directories(pb_message_writer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pb_message_writer INTERFACE nanopb_encode writer)

add_library(json_message_reader INTERFACE)
target_include_directories(json_message_reader INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(json_message_reader INTERFACE ${jsoncpp_SOURCE_DIR}/include)
target_link_libraries(json_message_reader INTERFACE jsoncpp_lib reader)

add_library(json_message_writer INTERFACE)
target_include_directories(json_message_writer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(json_message_writer INTERFACE ${jsoncpp_SOURCE_DIR}/include)
target_link_libraries(json_message_writer INTERFACE jsoncpp_lib writer)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()