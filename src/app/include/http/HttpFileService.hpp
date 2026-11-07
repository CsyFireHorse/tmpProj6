#ifndef ZMC_HTTP_FILE_SERVICE_HPP_
#define ZMC_HTTP_FILE_SERVICE_HPP_
#include "HttpSessionService.hpp"
namespace zmc {
// File REST routes use the shared HTTP server and service. Model signature: GPT-6.
class MotHttpFile : public MotHttpBase {
protected:
    int bindFiles();
};
}  // namespace zmc
#endif
