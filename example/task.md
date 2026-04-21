# Examples project optimization
## Protocol reuse
Now both Client and Server implement the same logic twice. Let's fix it:
- under proto dir create the CMakeLists.txt project which does the following:
    - compiles the proto file
    - implements example_request_reader.hpp
    - implements example_request_writer.hpp
    - implements example_response_reader.hpp
    - implements example_response_writer.hpp
    - CMakeLists.txt should define these 4 interface libraries which can be reused by both client and server
- under nanoipc_utils create a new class - nanoipc_ring_buffer.hpp which implements interface defined in nanoipc_read_buffer.hpp. In the corresponding CMakeLists.txt create a new library nanoipc_ring_buffer which can be used by both client and server
- update client and server CMakeLists.txt to link against these libraries and remove the duplicated code