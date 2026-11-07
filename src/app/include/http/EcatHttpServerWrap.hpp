#ifndef _ZMC_ECAT_HTTP_SERVER_WRAP_HPP_
#define _ZMC_ECAT_HTTP_SERVER_WRAP_HPP_
#include "HttpScriptHelper.hpp"
namespace zmc {
// Register EtherCAT routes after the shared motion routes. Model signature: GPT-6.
class MotHttpEcat : public MotHttpQcmd {
public:
    // Register routes against the application's service.
    int bind(srv::MotSrv& service) override;
private:
    MotHttpEcat() = default;
    // Stop and join workers before derived route handlers are destroyed. GPT-6, 2026-10-29.
    ~MotHttpEcat() override;
    void bind_ecat_routes();
    friend class MotionApp;
};
using MotHttp = MotHttpEcat;
}  // namespace zmc
#endif
