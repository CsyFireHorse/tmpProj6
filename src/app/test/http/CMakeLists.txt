# GPT-6 Codex, 2026-10-22. Service protocol cases link no C client implementation.
file(GLOB test_motapp_http_protocol_source
  ../test_main.cpp
  *.cpp)
add_executable(test_motapp_http_protocol ${test_motapp_http_protocol_source})

target_link_libraries(test_motapp_http_protocol PRIVATE
  ${THIRD_LIBS} motapp_test_runtime motapp ${ZECM_APP_LIBS} Threads::Threads)
set_target_properties(test_motapp_http_protocol PROPERTIES
  CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
target_include_directories(test_motapp_http_protocol PRIVATE
  ${CMAKE_SOURCE_DIR}/src/base/include ${CMAKE_SOURCE_DIR}/src/base/test
  ${THIRD_INCLUDE} ${CMAKE_SOURCE_DIR}/src/zecm/source)

if(MSVC)
  set(_motapp_http_test_cfg "Debug")
  if(BUILD_OPT STREQUAL "release")
    set(_motapp_http_test_cfg "Release")
  elseif(BUILD_OPT STREQUAL "debug")
    set(_motapp_http_test_cfg "Debug")
  elseif(CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
    set(_motapp_http_test_cfg "${CMAKE_BUILD_TYPE}")
  endif()
  set(_motapp_http_test_exe
    "${CMAKE_CURRENT_BINARY_DIR}/${_motapp_http_test_cfg}/test_motapp_http_protocol.exe")
else()
  set(_motapp_http_test_exe "${CMAKE_CURRENT_BINARY_DIR}/test_motapp_http_protocol")
endif()

add_test(dev_test_test_motapp_http_protocol "${CMAKE_COMMAND}"
  "-DTEST_EXE=${_motapp_http_test_exe}"
  "-DTEST_CASE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/test_cases"
  -DTEST_CASE=http_protocol "-DTEST_FILTER=*"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/../RunIsolatedTest.cmake")
set_tests_properties(dev_test_test_motapp_http_protocol PROPERTIES
  RESOURCE_LOCK motapp_http_28968 TIMEOUT 60 LABELS "http_protocol")

set(HTTP_SERVER_QUERY_EXIT_RUNNER ${MOTAPP_TEST_SH}
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_http_server_query_exit_ctest.sh)
set(ECAT_HTTP_LIFECYCLE_RUNNER ${MOTAPP_TEST_SH}
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_ecat_http_lifecycle_ctest.sh)
add_test(dev_test_http_server_query_exit
  ${HTTP_SERVER_QUERY_EXIT_RUNNER} ${MOTAPP_DEMO_EXE_FOR_CTEST}
  ${SERVO_FAK_FIXTURE_BIN} ${CMAKE_SOURCE_DIR})
set_tests_properties(dev_test_http_server_query_exit PROPERTIES
  RESOURCE_LOCK motapp_http_28768
  TIMEOUT 30)
add_test(dev_test_ecat_http_lifecycle
  ${ECAT_HTTP_LIFECYCLE_RUNNER} ${MOTAPP_DEMO_EXE_FOR_CTEST}
  ${SERVO_FAK_FIXTURE_BIN} ${CMAKE_SOURCE_DIR})
set_tests_properties(dev_test_ecat_http_lifecycle PROPERTIES
  RESOURCE_LOCK motapp_ecat_http_18768
  TIMEOUT 60)

if(WIN32)
  get_filename_component(MOTAPP_TEST_SH_DIR "${MOTAPP_TEST_SH}" DIRECTORY)
  string(REPLACE ";" "\\;" MOTAPP_TEST_PATH "${MOTAPP_TEST_SH_DIR};$ENV{PATH}")
  set_tests_properties(dev_test_http_server_query_exit dev_test_ecat_http_lifecycle
    PROPERTIES ENVIRONMENT "PATH=${MOTAPP_TEST_PATH}")
endif()
