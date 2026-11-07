# Date: 2026-11-05; Author: GPT-6 Codex.
# One source list, dataset and registration list serve both client implementations.
set(ZML_SHARED_TEST_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(ZML_TEST_DATA "${CMAKE_BINARY_DIR}/src/app/test/zml_fixtures")
file(MAKE_DIRECTORY "${ZML_TEST_DATA}")
file(COPY "${CMAKE_SOURCE_DIR}/src/mhelper/data/pse_servo1.xml"
          "${CMAKE_SOURCE_DIR}/src/mhelper/data/pse_servo2.xml"
          "${CMAKE_SOURCE_DIR}/src/mhelper/data/zmc_motapp_http_io_metadata_test.ini"
     DESTINATION "${ZML_TEST_DATA}")
# The production script parser accepts commands only; omit the template's byline.
file(READ "${ZML_SHARED_TEST_DIR}/zml_startup.zcode" ZML_STARTUP)
string(REGEX REPLACE "^//[^\n]*\n" "" ZML_STARTUP "${ZML_STARTUP}")
file(WRITE "${ZML_TEST_DATA}/zml_startup.zcode" "${ZML_STARTUP}")
set(ZML_AXIS_NUM 2)
set(ZML_UNITS 10)
set(ZML_ENI "${ZML_TEST_DATA}/pse_servo2.xml")
configure_file("${ZML_SHARED_TEST_DIR}/zml_motion.ini.in" "${ZML_TEST_DATA}/zml_motion.ini" @ONLY)
set(ZML_AXIS_NUM 1)
set(ZML_UNITS 25)
set(ZML_ENI "${ZML_TEST_DATA}/pse_servo1.xml")
configure_file("${ZML_SHARED_TEST_DIR}/zml_motion.ini.in" "${ZML_TEST_DATA}/zml_next.ini" @ONLY)
configure_file("${ZML_SHARED_TEST_DIR}/zml_io.ini.in" "${ZML_TEST_DATA}/zml_io.ini" @ONLY)

function(add_zml_client_test target mode port client_source)
  file(GLOB ZML_SHARED_TEST_SOURCES "${ZML_SHARED_TEST_DIR}/*.cpp")
  add_executable(${target} "${client_source}" ${ZML_SHARED_TEST_SOURCES})
  target_include_directories(${target} PRIVATE
    "${CMAKE_SOURCE_DIR}/src/app/demo/${mode}_client/include"
    "${CMAKE_SOURCE_DIR}/src/app/demo/rpc_client/include"
    "${CMAKE_SOURCE_DIR}/src/base/include")
  if(mode STREQUAL "http")
    set(ZML_HTTP_CLIENT_DIR "${CMAKE_SOURCE_DIR}/src/app/demo/http_client")
    target_sources(${target} PRIVATE "${ZML_HTTP_CLIENT_DIR}/source/zml_http_json.cpp"
      "${ZML_HTTP_CLIENT_DIR}/source/zml_json.cpp"
      "${ZML_HTTP_CLIENT_DIR}/test/test_zml_http_json.cpp")
    target_include_directories(${target} PRIVATE "${ZML_SHARED_TEST_DIR}")
  endif()
  target_compile_definitions(${target} PRIVATE ZML_EXPORTS
    ZML_TEST_MODE="${mode}" ZML_TEST_PORT=${port} ZML_TEST_FIXTURE_DIR="${ZML_TEST_DATA}")
  target_link_libraries(${target} PRIVATE gtest_main motapp_test_runtime motapp
    ${ZECM_APP_LIBS} Threads::Threads)
  if(mode STREQUAL "rpc")
    target_link_libraries(${target} PRIVATE rpc)
  endif()
  set_target_properties(${target} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)

  # Match the repository's CMake 3.9 / Visual Studio multi-config registration.
  set(test_exe "${CMAKE_CURRENT_BINARY_DIR}/${target}")
  if(MSVC)
    set(test_cfg Debug)
    if(BUILD_OPT STREQUAL "release")
      set(test_cfg Release)
    elseif(CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
      set(test_cfg "${CMAKE_BUILD_TYPE}")
    endif()
    set(test_exe "${CMAKE_CURRENT_BINARY_DIR}/${test_cfg}/${target}.exe")
  endif()
  # Run all enabled tests once per executable in its own working directory.
  set(test_name "dev_test_${mode}_client")
  add_test(${test_name} "${CMAKE_COMMAND}" "-DTEST_EXE=${test_exe}"
    "-DTEST_CASE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/test_cases"
    "-DTEST_CASE=${mode}_client" "-DTEST_FILTER=*" -DTEST_TIMEOUT=179
    -P "${CMAKE_SOURCE_DIR}/src/app/test/RunIsolatedTest.cmake")
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 180
    RESOURCE_LOCK "motapp_${mode}_${port}" LABELS "zml_client;${mode}_client")
endfunction()
