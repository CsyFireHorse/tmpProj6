/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-18
@ArtifactStatusAuthority GPT-5 Codex
*/
#include <gtest/gtest.h>

#include "axfb_bridge_api.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "ecat/ecat_state_adapter.h"

namespace {

int32_t get_slave_count(void *, uint16_t *slave_count) {
  *slave_count = 0u;
  return 0;
}

int32_t get_slave_state(void *, uint16_t, uint8_t *slave_state) {
  *slave_state = 0u;
  return 0;
}

int32_t get_master_state(void *, uint8_t *master_state) {
  *master_state = 0x08u;
  return 0;
}

} // namespace

TEST(AxFbBridgeEcatApi, BindValidatesHandleAndForwardsAdapter) {
  EcatStateAdapter adapter{};
  adapter.get_slave_count = get_slave_count;
  adapter.get_slave_state = get_slave_state;
  adapter.get_master_state = get_master_state;

  EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_HANDLE,
            axfb_bridge_bind_ecat_state_adapter(nullptr, &adapter));

  axfb_bridge_handle_t handle = nullptr;
  ASSERT_EQ(0, axfb_bridge_create(&handle));
  EXPECT_EQ(0, axfb_bridge_bind_ecat_state_adapter(handle, &adapter));

  EcatStateAdapter incomplete{};
  EXPECT_EQ(ZMC_ERR_BRIDGE_ECAT_STATE_ADAPTER_INVALID,
            axfb_bridge_bind_ecat_state_adapter(handle, &incomplete));
  axfb_bridge_destroy(handle);
}
