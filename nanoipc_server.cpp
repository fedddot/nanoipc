#include "nanoipc_server.hpp"

#include <string>

// Explicit template instantiation for NanoIpcServer with std::string request and int response
template class nanoipc::NanoIpcServer<std::string, int>;

