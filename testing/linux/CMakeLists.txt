add_library(virtual_uart INTERFACE)
target_include_directories(virtual_uart INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(virtual_uart INTERFACE gtest gtest_main)

# openpty is in libutil on glibc (Ubuntu/Debian); it is part of musl libc on Alpine.
find_library(UTIL_LIB util)
if(UTIL_LIB)
    target_link_libraries(virtual_uart INTERFACE ${UTIL_LIB})
endif()
