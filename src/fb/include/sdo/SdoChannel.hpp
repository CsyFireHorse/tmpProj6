/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#ifndef _ZMC_SDO_CHANNEL_HPP_
#define _ZMC_SDO_CHANNEL_HPP_

#include "SdoMessage.hpp"
#include "common/zmc/pcie_compat.h"

namespace zmc {

class SdoWorker;

// Own both one-way queues and enforce the frontend/backend capability split.
class SdoChannel {
public:
  // Restrict the RT side to request production and result consumption.
  class Frontend {
  public:
    // Push one request batch toward the worker.
    int push_request(const SdoRequest &request);
    // Pop one completed result payload from the worker.
    bool pop_result(SdoResult &result);

  private:
    friend class SdoChannel;

    // Bind this handle to one worker-owned channel.
    explicit Frontend(SdoChannel &channel);

    SdoChannel *channel_;
  };

  SdoChannel() = default;
  ~SdoChannel() = default;

  // Create the only capability exposed to SdoRtManager.
  Frontend frontend();

  SdoChannel(const SdoChannel &) = delete;
  SdoChannel &operator=(const SdoChannel &) = delete;

private:
  friend class SdoWorker;

  using RequestQueue = SPSCQueue<SdoRequest, 64>;
  using ResultQueue = SPSCQueue<SdoResult, 64>;

  // Pop one frontend request on the worker side.
  bool pop_request(SdoRequest &request);
  // Push one worker result toward the frontend.
  int push_result(const SdoResult &result);
  // Drain both queues only after worker-thread ownership has ended.
  void reset();

  RequestQueue request_queue_{};
  ResultQueue result_queue_{};
};

} // namespace zmc

#endif // _ZMC_SDO_CHANNEL_HPP_
