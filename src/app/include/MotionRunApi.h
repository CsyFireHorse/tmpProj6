#if 0  // Disabled legacy service bypass. Model signature: GPT-6.
#ifndef _ZMC_MOTION_RUN_API_H_
#define _ZMC_MOTION_RUN_API_H_

// Model signature: GPT-6. Runtime owners initialize srv before entering a protocol server.
namespace zmc {
// Run HTTP motion routes using the existing singleton binding.
int runHttpServer(int port);
// Run HTTP EtherCAT and motion routes using the existing singleton binding.
int runEcatHttpServer(int port);
}  // namespace zmc
#endif

#endif
