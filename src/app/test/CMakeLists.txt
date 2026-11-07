# CTest registration for motapp (invoked from ../CMakeLists.txt when ENABLE_DEV_UNITTEST)

if(NOT MOTAPP_EXE_FOR_CTEST OR NOT MOTAPP_TEST_CS_NAME)
  message(FATAL_ERROR "motapp test: MOTAPP_EXE_FOR_CTEST and MOTAPP_TEST_CS_NAME must be set by parent")
endif()

set(SERVO_FAK_FIXTURE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fixtures)
set(SERVO_FAK_FIXTURE_BIN ${CMAKE_CURRENT_BINARY_DIR}/servo_fak_fixtures)
file(MAKE_DIRECTORY ${SERVO_FAK_FIXTURE_BIN}/case_env_file)
file(MAKE_DIRECTORY ${SERVO_FAK_FIXTURE_BIN}/case_env_override)
file(COPY ${SERVO_FAK_FIXTURE_SRC}/case_load_sparse DESTINATION ${SERVO_FAK_FIXTURE_BIN})
file(COPY ${SERVO_FAK_FIXTURE_SRC}/case_save_append DESTINATION ${SERVO_FAK_FIXTURE_BIN})
file(COPY ${SERVO_FAK_FIXTURE_SRC}/case_env_override/from_env.jsonl
     DESTINATION ${SERVO_FAK_FIXTURE_BIN}/case_env_override)
file(COPY ${SERVO_FAK_FIXTURE_SRC}/case_env_override/from_process.jsonl
     DESTINATION ${SERVO_FAK_FIXTURE_BIN}/case_env_override)
file(COPY ${SERVO_FAK_FIXTURE_SRC}/case_env_file/servo_fak_seed.jsonl
     DESTINATION ${SERVO_FAK_FIXTURE_BIN}/case_env_file)

set(SERVO_FAK_JSONL ${SERVO_FAK_FIXTURE_BIN}/case_env_file/servo_fak_seed.jsonl)
configure_file(
  ${SERVO_FAK_FIXTURE_SRC}/case_env_file/zecm_servo_fak.env.in
  ${SERVO_FAK_FIXTURE_BIN}/case_env_file/zecm_servo_fak.env
  @ONLY)
set(SERVO_FAK_JSONL_ENV ${SERVO_FAK_FIXTURE_BIN}/case_env_override/from_env.jsonl)
configure_file(
  ${SERVO_FAK_FIXTURE_SRC}/case_env_override/zecm_servo_fak.env.in
  ${SERVO_FAK_FIXTURE_BIN}/case_env_override/zecm_servo_fak.env
  @ONLY)

# Use one shell for every script runner on both hosts. AI modification signature: GPT-6.
if(WIN32)
  find_program(MOTAPP_TEST_SH
    NAMES sh.exe sh
    HINTS
      "$ENV{ProgramFiles}/Git/usr/bin"
      "$ENV{LOCALAPPDATA}/Programs/Git/usr/bin")
else()
  find_program(MOTAPP_TEST_SH NAMES sh)
endif()
if(NOT MOTAPP_TEST_SH)
  message(FATAL_ERROR
    "motapp tests require sh; install Git Bash on Windows and ensure sh.exe is discoverable")
endif()
set(SERVO_FAK_RUNNER ${MOTAPP_TEST_SH}
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_servo_fak_ctest.sh)

# Legacy add_test() form (not add_test(NAME ...)) so MSVC multi-config does not emit NOT_AVAILABLE stubs.
add_test(dev_test_servo_fak_env_file
  ${SERVO_FAK_RUNNER} ${MOTAPP_EXE_FOR_CTEST} ${SERVO_FAK_FIXTURE_BIN} env_file)
add_test(dev_test_servo_fak_env_override
  ${SERVO_FAK_RUNNER} ${MOTAPP_EXE_FOR_CTEST} ${SERVO_FAK_FIXTURE_BIN} env_override)
add_test(dev_test_servo_fak_save_append
  ${SERVO_FAK_RUNNER} ${MOTAPP_EXE_FOR_CTEST} ${SERVO_FAK_FIXTURE_BIN} save_append)
add_test(dev_test_servo_fak_load_sparse
  ${MOTAPP_EXE_FOR_CTEST} --gtest_filter=ServoFakFileState.LoadSparseAxes)
set_tests_properties(dev_test_servo_fak_load_sparse PROPERTIES ENVIRONMENT
  "ZECM_SERVO_FAK_STATE_FILE=${SERVO_FAK_FIXTURE_BIN}/case_load_sparse/servo_fak_seed.jsonl")

# Slow integration tests: register last so default ctest finishes quickly.
add_test(dev_test_${MOTAPP_TEST_CS_NAME} ${MOTAPP_EXE_FOR_CTEST} --gtest_filter=-ServoFakFileState.*)

if(WIN32)
  # CTest launches sh directly, without Git Bash's usual PATH initialization.
  get_filename_component(MOTAPP_TEST_SH_DIR "${MOTAPP_TEST_SH}" DIRECTORY)
  string(REPLACE ";" "\\;" MOTAPP_TEST_PATH "${MOTAPP_TEST_SH_DIR};$ENV{PATH}")
  set_tests_properties(dev_test_servo_fak_env_file dev_test_servo_fak_env_override
    dev_test_servo_fak_save_append
    PROPERTIES ENVIRONMENT "PATH=${MOTAPP_TEST_PATH}")
endif()

add_subdirectory(http)

if(ENABLE_RPC)
  add_subdirectory(rpc)
endif()
