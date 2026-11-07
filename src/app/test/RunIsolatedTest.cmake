# Date: 2026-10-22; Author: GPT-6 Codex.
# Reset only a named child of the build-owned test directory before each CTest run.
# TEST_TIMEOUT overrides the per-case binary budget; callers keep their own ctest TIMEOUT.
get_filename_component(case_root "${TEST_CASE_ROOT}" ABSOLUTE)
get_filename_component(case_dir "${case_root}/${TEST_CASE}" ABSOLUTE)
get_filename_component(case_parent "${case_dir}" DIRECTORY)
get_filename_component(root_name "${case_root}" NAME)
if(NOT root_name STREQUAL "test_cases" OR NOT case_parent STREQUAL case_root
   OR NOT TEST_CASE MATCHES "^[A-Za-z][A-Za-z0-9_.]*$")
  message(FATAL_ERROR "Invalid test directory: ${case_dir}")
endif()
if(NOT DEFINED TEST_FILTER)
  set(TEST_FILTER "${TEST_CASE}")
endif()
if(NOT DEFINED TEST_TIMEOUT)
  set(TEST_TIMEOUT 55)
endif()
file(REMOVE_RECURSE "${case_dir}")
file(MAKE_DIRECTORY "${case_dir}")
execute_process(COMMAND "${TEST_EXE}" "--gtest_filter=${TEST_FILTER}"
  WORKING_DIRECTORY "${case_dir}" RESULT_VARIABLE result TIMEOUT ${TEST_TIMEOUT})
if(NOT result STREQUAL "0")
  message(FATAL_ERROR "${TEST_CASE}: ${result}")
endif()
