#include <stdexcept>
#include <vector>

#include "gtest/gtest.h"

#include "nanopb_serializer.hpp"
#include "pb_encode.h"
#include "test.pb.h"

using namespace nanoipc;

struct TestMessage {
    int int_value;
    std::string string_value;
};

using TestSerializer = NanoPbSerializer<TestMessage, test_api_TestMessage>;

static bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static test_api_TestMessage get_test_message_descriptor(const TestMessage& msg);

TEST(ut_nanopb_serializer, invalid_args) {
	ASSERT_THROW(TestSerializer(nullptr), std::invalid_argument);
}

TEST(ut_nanopb_serializer, sanity) {
	// GIVEN
	const TestMessage message = {
		.int_value = 42,
		.string_value = "test"
	};
	TestSerializer serializer(get_test_message_descriptor);

	// WHEN
	const auto result = serializer(message);

	// THEN: protobuf encoding of {value: 42} → field 1 varint (0x08), value 42 (0x2A)
	const std::vector<std::uint8_t> expected = {0x08, 0x2A, 0x12, 0x04, 't', 'e', 's', 't'};
	ASSERT_EQ(result, expected);
}

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
	if (!arg) {
		throw std::runtime_error("encode_string called with null arg");
	}
	const auto str = static_cast<const std::string *>(*arg);
	if (!pb_encode_tag_for_field(stream, field)) {
		return false;
	}
	return pb_encode_string(stream, (const pb_byte_t *)(str->c_str()), str->size());
}

test_api_TestMessage get_test_message_descriptor(const TestMessage& msg) {
	test_api_TestMessage pb_message = test_api_TestMessage_init_default;
	pb_message.int_value = msg.int_value;
	pb_message.string_value.funcs.encode = encode_string;
	pb_message.string_value.arg = const_cast<std::string *>(&msg.string_value);
	return pb_message;
}