# zmc.axfb module and Lua runtime composition root.
set(ZMC_LUA_ROOT "${PROJECT_SOURCE_DIR}/src/thirdlib/lua" CACHE PATH
    "Vendored Lua 5.5 source root used by zmc.axfb")

add_subdirectory(runtime)

add_library(zmc_axfb MODULE
    source/zmc_axfb_lua_context.cpp
    source/zmc_axfb_lua_bridge.cpp
    source/zmc_axfb_lua_command.cpp
    source/zmc_axfb_lua_status.cpp
    source/zmc_axfb_lua_memory_pou.cpp
    source/zmc_axfb_lua_module.cpp)
target_link_libraries(zmc_axfb PRIVATE mcapi zmc_lua55_runtime)
target_include_directories(zmc_axfb PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src/common"
    "${PROJECT_SOURCE_DIR}/src/fb/include"
    "${PROJECT_SOURCE_DIR}/src/fb/include/impl"
    "${PROJECT_SOURCE_DIR}/src/fb/include/axis"
    "${PROJECT_SOURCE_DIR}/src/mcapi/include"
    "${PROJECT_SOURCE_DIR}/src/utils"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/source")

set_target_properties(zmc_axfb PROPERTIES
    PREFIX ""
    OUTPUT_NAME "axfb"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lua/zmc"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lua/zmc")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set_target_properties(zmc_axfb PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/lua/zmc"
      RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/lua/zmc")
endforeach()

install(TARGETS zmc_axfb DESTINATION bin/${BUILD_OPT}/lua/zmc)

# Lua integration tests (registered only when ENABLE_LUA_AXFB builds this tree).
# Legacy add_test() form (not add_test(NAME ...) + $<TARGET_FILE>) so MSVC
# multi-config does not emit NOT_AVAILABLE stubs when ctest runs without -C.
set(ZMC_LUA_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")
set(ZMC_LUA_CLI_EXE "${CMAKE_BINARY_DIR}/lua/lua${CMAKE_EXECUTABLE_SUFFIX}")
if(WIN32)
  set(ZMC_LUA_CPATH_GLOB "${CMAKE_BINARY_DIR}/lua/?.dll")
else()
  set(ZMC_LUA_CPATH_GLOB "${CMAKE_BINARY_DIR}/lua/?.so")
endif()
set(ZMC_LUA_TEST_BOOTSTRAP
    "package.cpath='${ZMC_LUA_CPATH_GLOB};'..package.cpath; package.path='${ZMC_LUA_TEST_DIR}/?.lua;'..package.path")

set(_zmc_lua_ctest_cfg "Debug")
if(BUILD_OPT STREQUAL "release")
  set(_zmc_lua_ctest_cfg "Release")
elseif(BUILD_OPT STREQUAL "debug")
  set(_zmc_lua_ctest_cfg "Debug")
elseif(CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
  set(_zmc_lua_ctest_cfg "${CMAKE_BUILD_TYPE}")
endif()
if(MSVC)
  set(_zmc_lua_mcapi_dir "${CMAKE_BINARY_DIR}/src/mcapi/${_zmc_lua_ctest_cfg}")
else()
  set(_zmc_lua_mcapi_dir "${CMAKE_BINARY_DIR}/src/mcapi")
endif()

function(zmc_add_lua_axfb_test test_name script_name)
  add_test(${test_name}
      ${ZMC_LUA_CLI_EXE}
      -e "${ZMC_LUA_TEST_BOOTSTRAP}"
      "${ZMC_LUA_TEST_DIR}/${script_name}")
  set_tests_properties(${test_name} PROPERTIES
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  if(WIN32)
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "PATH=${_zmc_lua_mcapi_dir};${CMAKE_BINARY_DIR}/lua;$ENV{PATH}")
  else()
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${_zmc_lua_mcapi_dir}:${CMAKE_BINARY_DIR}/lua:$ENV{LD_LIBRARY_PATH}")
  endif()
endfunction()

zmc_add_lua_axfb_test(dev_test_lua_axfb_packet_init_reset test_packet_init_reset.lua)
zmc_add_lua_axfb_test(dev_test_lua_axfb_bridge_status_pop test_bridge_status_pop.lua)
zmc_add_lua_axfb_test(dev_test_lua_axfb_bridge_power_move test_bridge_power_move.lua)
zmc_add_lua_axfb_test(dev_test_lua_memory_pou_fields test_memory_pou_fields.lua)
set_tests_properties(dev_test_lua_axfb_bridge_power_move PROPERTIES TIMEOUT 120)
set_tests_properties(dev_test_lua_memory_pou_fields PROPERTIES TIMEOUT 120)
