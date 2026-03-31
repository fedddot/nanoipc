#include "gtest/gtest.h"
#include <string>

#include "nanoipc.hpp"

using namespace nanoipc;

using ApiRequest = std::string;
using ApiResponse = int;
using TestNanoIpc = NanoIpc<ApiRequest, ApiResponse, 10UL>;

TEST(ut_nanoipc, sanity) {
	// GIVEN
	// WHEN

	// THEN
}