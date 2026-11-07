# Lua 5.5 runtime and command-line interpreter.
set(ZMC_LUA55_INCLUDE_DIR "${ZMC_LUA_ROOT}")
set(ZMC_LUA55_HEADER "${ZMC_LUA55_INCLUDE_DIR}/lua.h")

if (NOT EXISTS "${ZMC_LUA55_HEADER}" OR
    NOT EXISTS "${ZMC_LUA55_INCLUDE_DIR}/lauxlib.h")
  message(FATAL_ERROR
      "ENABLE_LUA_AXFB=ON requires vendored Lua 5.5 headers under ${ZMC_LUA_ROOT}.")
endif()

file(READ "${ZMC_LUA55_HEADER}" ZMC_LUA55_HEADER_CONTENT)
string(REGEX MATCH "#define[ \t]+LUA_VERSION_MAJOR_N[ \t]+5" ZMC_LUA55_MAJOR_MATCH
       "${ZMC_LUA55_HEADER_CONTENT}")
string(REGEX MATCH "#define[ \t]+LUA_VERSION_MINOR_N[ \t]+5" ZMC_LUA55_MINOR_MATCH
       "${ZMC_LUA55_HEADER_CONTENT}")
if (NOT ZMC_LUA55_MAJOR_MATCH OR NOT ZMC_LUA55_MINOR_MATCH)
  message(FATAL_ERROR
      "zmc.axfb requires Lua 5.5 sources under ${ZMC_LUA_ROOT}.")
endif()

file(GLOB ZMC_LUA55_SOURCES "${ZMC_LUA55_INCLUDE_DIR}/*.c")
list(REMOVE_ITEM ZMC_LUA55_SOURCES
     "${ZMC_LUA55_INCLUDE_DIR}/lua.c"
     "${ZMC_LUA55_INCLUDE_DIR}/luac.c"
     "${ZMC_LUA55_INCLUDE_DIR}/ltests.c"
     "${ZMC_LUA55_INCLUDE_DIR}/onelua.c")

add_library(zmc_lua55_runtime SHARED ${ZMC_LUA55_SOURCES})
target_include_directories(zmc_lua55_runtime PUBLIC "${ZMC_LUA55_INCLUDE_DIR}")
if (WIN32)
  target_compile_definitions(zmc_lua55_runtime
      PRIVATE LUA_BUILD_AS_DLL _CRT_SECURE_NO_WARNINGS
      PUBLIC LUA_BUILD_AS_DLL)
endif()
if (UNIX)
  target_compile_definitions(zmc_lua55_runtime PUBLIC LUA_USE_LINUX)
  target_link_libraries(zmc_lua55_runtime PUBLIC m ${CMAKE_DL_LIBS})
endif()

add_executable(zmc_lua_cli "${ZMC_LUA55_INCLUDE_DIR}/lua.c")
target_include_directories(zmc_lua_cli PRIVATE "${ZMC_LUA55_INCLUDE_DIR}")
target_link_libraries(zmc_lua_cli PRIVATE zmc_lua55_runtime)
set_target_properties(zmc_lua55_runtime PROPERTIES
    OUTPUT_NAME "lua55"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lua"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lua")
set_target_properties(zmc_lua_cli PROPERTIES
    OUTPUT_NAME "lua"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lua")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set_target_properties(zmc_lua55_runtime PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/lua"
      LIBRARY_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/lua")
  set_target_properties(zmc_lua_cli PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/lua")
endforeach()

install(TARGETS zmc_lua55_runtime zmc_lua_cli
    RUNTIME DESTINATION bin/${BUILD_OPT}/lua
    LIBRARY DESTINATION bin/${BUILD_OPT}/lua
    ARCHIVE DESTINATION lib/${BUILD_OPT}/lua)
