#include <stdexcept>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "pb_message_reader.hpp"
#include "pb_message_writer.hpp"

using namespace nanoipc;

using TestParser = NanoPbParser<TestMessage, test_api_TestMessage>;

static bool decode_string(pb_istream_t* stream, const pb_field_t* /*field*/, void** arg) {
	auto* str = static_cast<std::string*>(*arg);
	str->resize(stream->bytes_left);
	return pb_read(stream, reinterpret_cast<pb_byte_t*>(str->data()), str->size());
}

TEST(ut_nanopb_parser, invalid_args) {
	ASSERT_THROW(TestParser(nullptr, test_api_TestMessage_fields), std::invalid_argument);
	ASSERT_THROW(TestParser([](const test_api_TestMessage&) { return TestMessage{}; }, nullptr), std::invalid_argument);
}

TEST(ut_nanopb_parser, sanity) {
	// GIVEN: protobuf encoding of {int_value: 42, string_value: "test"}
	// (mirror of ut_nanopb_serializer.sanity expected output)
	const std::vector<std::uint8_t> encoded = {0x08, 0x2A, 0x12, 0x04, 't', 'e', 's', 't'};
	std::string decoded_string;
	TestParser parser(
		[&decoded_string](const test_api_TestMessage& pb_msg) -> TestMessage {
			return {pb_msg.int_value, decoded_string};
		},
		test_api_TestMessage_fields,
		[&decoded_string](test_api_TestMessage& pb_msg) {
			pb_msg.string_value.funcs.decode = decode_string;
			pb_msg.string_value.arg = &decoded_string;
		}
	);

	// WHEN
	const auto result = parser(encoded);

	// THEN
	ASSERT_EQ(result.int_value, 42);
	ASSERT_EQ(result.string_value, "test");
}

TEST(ut_nanopb_parser, invalid_data) {
	// GIVEN: truncated varint — tag byte for field 1 with no value following
	const std::vector<std::uint8_t> corrupt_data = {0x08};
	TestParser parser(
		[](const test_api_TestMessage& pb_msg) -> TestMessage {
			return {pb_msg.int_value, ""};
		},
		test_api_TestMessage_fields
	);

	// WHEN / THEN
	ASSERT_THROW(parser(corrupt_data), std::runtime_error);
}
