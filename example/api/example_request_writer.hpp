#ifndef EXAMPLE_REQUEST_WRITER_HPP
#define EXAMPLE_REQUEST_WRITER_HPP

#include <stdexcept>

#include "example.pb.h"
#include "example_api.hpp"
#include "nanopb_serializer.hpp"
#include "nanoipc_writer.hpp"

namespace api {

    /// @brief Writer specialised for ExampleRequest messages.
    class ExampleRequestWriter : public nanoipc::NanoIpcWriter<ExampleRequest> {
    public:
        using RawDataWriter = typename nanoipc::NanoIpcWriter<ExampleRequest>::RawDataWriter;
        /// Construct a writer that emits encoded frames via `raw_writer`.
        explicit ExampleRequestWriter(const RawDataWriter& raw_writer)
        : nanoipc::NanoIpcWriter<ExampleRequest>(
            nanoipc::NanoPbSerializer<ExampleRequest, example_api_ExampleRequest>(request_to_pb, example_api_ExampleRequest_fields),
            raw_writer
        ) {}
    private:
        static example_api_Action action_to_pb(const Action action) {
            switch (action) {
                case Action::ADD: return example_api_Action_ADD;
                case Action::SUBTRACT: return example_api_Action_SUBTRACT;
                default:
                    throw std::invalid_argument("invalid action value: " + std::to_string(static_cast<int32_t>(action)));
            }
        }
        static example_api_ExampleRequest request_to_pb(const ExampleRequest& request) {
            
            example_api_ExampleRequest pb{};
            pb.value = request.value;
            pb.action = action_to_pb(request.action);
            return pb;
        }
    };
} // namespace api

#endif // EXAMPLE_REQUEST_WRITER_HPP
