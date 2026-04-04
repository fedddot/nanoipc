FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG		 v1.17.0
)
FetchContent_MakeAvailable(googletest)

add_executable(
	nanoipc_tests
	${CMAKE_CURRENT_LIST_DIR}/src/ut_nanoipc_reader.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ut_nanoipc_writer.cpp
)

target_link_libraries(
	nanoipc_tests
	PRIVATE
	nanoipc_reader
	nanoipc_writer
	nanoipc_utils
	gtest
	gtest_main
)

add_test(NAME nanoipc_tests COMMAND nanoipc_tests)