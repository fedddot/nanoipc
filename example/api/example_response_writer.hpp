#ifndef EXAMPLE_RESPONSE_WRITER_HPP
#define EXAMPLE_RESPONSE_WRITER_HPP

#include "example.pb.h"
#include "example_api.hpp"
#include "nanopb_serializer.hpp"
#include "nanoipc_writer.hpp"

namespace api {

    /// @brief Writer specialised for ExampleResponse messages.
    class ExampleResponseWriter : public nanoipc::NanoIpcWriter<ExampleResponse> {
    public:
        using RawDataWriter = typename nanoipc::NanoIpcWriter<ExampleResponse>::RawDataWriter;

        /// Construct a writer that emits encoded frames via `raw_writer`.
        explicit ExampleResponseWriter(const RawDataWriter& raw_writer)
        : nanoipc::NanoIpcWriter<ExampleResponse>(
            nanoipc::NanoPbSerializer<ExampleResponse, example_api_ExampleResponse>(response_to_pb, example_api_ExampleResponse_fields),
            raw_writer
        ) {}
    private:
        static example_api_ExampleResponse response_to_pb(const ExampleResponse& response) {
            example_api_ExampleResponse pb{};
            pb.result = response.result;
            return pb;
        }
    };

} // namespace api

#endif // EXAMPLE_RESPONSE_WRITER_HPP
