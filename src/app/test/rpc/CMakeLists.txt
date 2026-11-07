# GPT-6 Codex, 2026-11-05. Service protocol cases link no C client implementation.
file(GLOB test_motapp_rpc_protocol_source
  ../test_main.cpp
  *.cpp)
add_executable(test_motapp_rpc_protocol ${test_motapp_rpc_protocol_source})

target_link_libraries(test_motapp_rpc_protocol PRIVATE
  ${THIRD_LIBS} motapp_test_runtime motapp ${ZECM_APP_LIBS} rpc)
set_target_properties(test_motapp_rpc_protocol PROPERTIES
  CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)

if(MSVC)
  set(_rpc_test_cfg "Debug")
  if(BUILD_OPT STREQUAL "release")
    set(_rpc_test_cfg "Release")
  elseif(BUILD_OPT STREQUAL "debug")
    set(_rpc_test_cfg "Debug")
  elseif(CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
    set(_rpc_test_cfg "${CMAKE_BUILD_TYPE}")
  endif()
  set(_rpc_test_exe "${CMAKE_CURRENT_BINARY_DIR}/${_rpc_test_cfg}/test_motapp_rpc_protocol.exe")
else()
  set(_rpc_test_exe "${CMAKE_CURRENT_BINARY_DIR}/test_motapp_rpc_protocol")
endif()

add_test(dev_test_test_motapp_rpc_protocol "${CMAKE_COMMAND}"
  "-DTEST_EXE=${_rpc_test_exe}"
  "-DTEST_CASE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/test_cases"
  -DTEST_CASE=rpc_protocol "-DTEST_FILTER=*"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/../RunIsolatedTest.cmake")
set_tests_properties(dev_test_test_motapp_rpc_protocol PROPERTIES
  RESOURCE_LOCK motapp_rpc_28969 TIMEOUT 60 LABELS "rpc_protocol")
