add_library(virtual_uart INTERFACE)
target_include_directories(virtual_uart INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(virtual_uart INTERFACE gtest util)