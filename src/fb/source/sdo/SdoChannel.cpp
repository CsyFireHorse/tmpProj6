/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#include "SdoChannel.hpp"

#include "common/zmc/fbt_errorcode_def.h"

namespace zmc {

SdoChannel::Frontend::Frontend(SdoChannel &channel) : channel_(&channel) {}

int SdoChannel::Frontend::push_request(const SdoRequest &request) {
  return channel_->request_queue_.try_push(request) ? 0 : ZMC_ERR_SDO_QUEUE_FULL;
}

bool SdoChannel::Frontend::pop_result(SdoResult &result) {
  return channel_->result_queue_.try_pop(result);
}

SdoChannel::Frontend SdoChannel::frontend() { return Frontend(*this); }

bool SdoChannel::pop_request(SdoRequest &request) {
  return request_queue_.try_pop(request);
}

int SdoChannel::push_result(const SdoResult &result) {
  return result_queue_.try_push(result) ? 0 : ZMC_ERR_SDO_QUEUE_FULL;
}

void SdoChannel::reset() {
  SdoRequest request{};
  while (request_queue_.try_pop(request)) {
  }

  SdoResult result{};
  while (result_queue_.try_pop(result)) {
  }
}

} // namespace zmc
