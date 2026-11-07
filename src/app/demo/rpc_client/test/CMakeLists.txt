# Date: 2026-10-22; Author: GPT-6 Codex.
# Compile the shared public contract against the RPC implementation.
add_zml_client_test(test_zmc_motapp_rpc_client rpc 28969
  "${CMAKE_CURRENT_SOURCE_DIR}/../source/zml_motion_api.cpp")
