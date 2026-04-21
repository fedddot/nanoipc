#ifndef EXAMPLE_RESPONSE_READER_HPP
#define EXAMPLE_RESPONSE_READER_HPP

#include "example.pb.h"
#include "example_api.hpp"
#include "nanoipc_reader.hpp"
#include "nanopb_parser.hpp"

namespace api {

    /// @brief Reader specialised for ExampleResponse messages.
    class ExampleResponseReader : public nanoipc::NanoIpcReader<ExampleResponse> {
    public:
        /// Construct a reader that reads encoded frames from `read_buffer`.
        explicit ExampleResponseReader(nanoipc::ReadBuffer* read_buffer)
        : nanoipc::NanoIpcReader<ExampleResponse>(
            nanoipc::NanoPbParser<ExampleResponse, example_api_ExampleResponse>(pb_to_response, example_api_ExampleResponse_fields),
            read_buffer
        ) {}
    private:
        static ExampleResponse pb_to_response(const example_api_ExampleResponse& pb) {
            return ExampleResponse{ .result = pb.result };
        }
    };

} // namespace api

#endif // EXAMPLE_RESPONSE_READER_HPP
