#include <stdexcept>
#include <vector>

#include "gtest/gtest.h"

#include "nanopb_serializer.hpp"
#include "test.pb.h"

using namespace nanoipc;

using TestSerializer = NanoPbSerializer<TestMessage>;

static pb_msgdesc_t* get_test_message_descriptor(const TestMessage&) {
	return const_cast<pb_msgdesc_t*>(&TestMessage_msg);
}

TEST(ut_nanopb_serializer, invalid_args) {
	ASSERT_THROW(TestSerializer(nullptr), std::invalid_argument);
}

TEST(ut_nanopb_serializer, sanity) {
	// GIVEN
	const TestMessage message = {42};
	TestSerializer serializer(get_test_message_descriptor);

	// WHEN
	const auto result = serializer(message);

	// THEN: protobuf encoding of {value: 42} → field 1 varint (0x08), value 42 (0x2A)
	const std::vector<std::uint8_t> expected = {0x08, 0x2A};
	ASSERT_EQ(result, expected);
}

TEST(ut_nanopb_serializer, zero_value) {
	// GIVEN
	const TestMessage message = {0};
	TestSerializer serializer(get_test_message_descriptor);

	// WHEN
	const auto result = serializer(message);

	// THEN: required field with value 0 is still encoded
	ASSERT_FALSE(result.empty());
}
