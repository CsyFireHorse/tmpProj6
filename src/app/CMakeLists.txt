cmake_minimum_required(VERSION 2.6.4)
# AI modification signature: GPT-6 Codex.


set(MAJOR_VERSION 1)
set(MINOR_VERSION 1)
set(PATCH_VERSION 0)


if (BUILD_SHARED_LIBS)
  set(LIB_DLLEXPORT "ZMC_DLLPORT ")
endif()
set(LIB_NAME_UPPER "MOTAPP")
set(LIB_NAME_LOWER "motapp")


set( lib_name ${LIB_NAME_LOWER})
set( func_name "test_cs_${lib_name}")


project( ${func_name} CXX C )


file(GLOB ${func_name}_src_temp *.h* *.c* source/*.c* source/http/*.c* source/srv/*.c* source/qcmd/*.c*)
file(GLOB ${func_name}_rpc_src source/rpc/*.c*)
file(GLOB_RECURSE ${func_name}_test_src test_dev/*.h* test_dev/*.c*)
set(${func_name}_src "${${func_name}_src_temp}" CACHE STRING INTERNAL FORCE)

include_directories(".")
include_directories("../vel")
include_directories("../fb")
include_directories("../fb/include")
include_directories("../fb/include/fb")
include_directories("../fb/include/axis")
include_directories("../fb/include/impl")
include_directories("../fb/include/axfb")
include_directories("../fb/include/pou")
include_directories("../fb/include/motbuf")
include_directories("../fb/include/sdo")
include_directories("../fb/include/bridge")
include_directories("include")
include_directories("include/http")
include_directories("include/srv")
include_directories("include/rpc")
include_directories("../zecm/include")

set(LIBRARY_SRC "${LIBRARY_SRC}" "${${func_name}_src}" CACHE STRING INTERNAL FORCE)

# lib list composed from common libs plus ZECM_APP_LIBS decided in top-level CMakeLists.txt
set( lib_list vel base traj plcfb  mhelper)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_library(${lib_name} ${${func_name}_src} )
set_target_properties(${lib_name} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
find_package(Threads REQUIRED)
if(MSVC)
    # The large HTTP route translation unit exceeds MSVC2015's regular object limit (GPT-6).
    set_source_files_properties(source/http/HttpScriptHelper.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif()
add_dependencies(${lib_name} plcfb)
target_compile_definitions(${lib_name} PRIVATE ZMC_DLLBUILD)
target_include_directories(${lib_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/base/include)
target_link_libraries(${lib_name} PRIVATE ${lib_list} Threads::Threads)

if(ENABLE_DEV_UNITTEST)
    enable_testing()
    # Share lifetime setup, without exposing service headers to cases.
    add_library(motapp_test_runtime STATIC test/SrvTestRuntime.cpp)
    target_include_directories(motapp_test_runtime PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")
    target_link_libraries(motapp_test_runtime PUBLIC motapp Threads::Threads)
    set_target_properties(motapp_test_runtime PROPERTIES
        CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    include("${CMAKE_CURRENT_SOURCE_DIR}/demo/rpc_client/test/ZmlClientTests.cmake")
endif()

if(ENABLE_RPC)
    target_sources(${lib_name} PRIVATE ${${func_name}_rpc_src})
    target_include_directories(${lib_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/thirdlib/rpclib/include)
    target_link_libraries(${lib_name} PRIVATE rpc)
    target_compile_definitions(${lib_name} PUBLIC ZMC_ENABLE_RPC)
    add_subdirectory(demo/rpc_client)
endif()

install(TARGETS ${lib_name} DESTINATION lib/${BUILD_OPT})
install(TARGETS ${lib_name} DESTINATION bin/${BUILD_OPT})
file(GLOB LIB_HEADFILES ${PROJECT_SOURCE_DIR}/*.h* )
install(FILES ${LIB_HEADFILES} DESTINATION include/${lib_name})
install(FILES include/LocalVfs.hpp DESTINATION include/${lib_name})

add_subdirectory("demo")
if(ENABLE_RPC)
    add_subdirectory("demo/pyclient")
endif()
add_subdirectory("demo/http_client")

option(ENABLE_PCIE_DEMO "Builds pcie demo (zmc_pcie_demo)" OFF)
option(ENABLE_PCIE_HOST_DEMO "Builds pcieHost demo (zmc_pcie_host_demo)" OFF)

if (ENABLE_PCIE_DEMO)
    add_subdirectory("demo/pcie")
endif()
# pcieHost demo currently relies on POSIX <unistd.h>; gate on non-Windows.
if (ENABLE_PCIE_HOST_DEMO AND NOT WIN32)
    add_subdirectory("demo/pcieHost")
endif()
if (USE_BENCH)
    # add_subdirectory(bench)
endif()

if (ENABLE_DEV_UNITTEST)
    file(GLOB_RECURSE ${func_name}_test_src test/*.h* test/*.c*)
    list(FILTER ${func_name}_test_src EXCLUDE REGEX "/test/rpc/")
    list(FILTER ${func_name}_test_src EXCLUDE REGEX "/test/http/")
    list(FILTER ${func_name}_test_src EXCLUDE REGEX "/test/SrvTestRuntime.cpp$")
    add_executable(${func_name} ${${func_name}_test_src})
    target_compile_definitions(${func_name} PRIVATE ZML_EXPORTS)
    target_link_libraries(${func_name} PRIVATE ${THIRD_LIBS} ${lib_name} plcfb ${ZECM_APP_LIBS}
        Threads::Threads)
    set_target_properties(${func_name} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    target_include_directories(${func_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/base/include ${CMAKE_SOURCE_DIR}/src/base/test)
    target_include_directories(${func_name} PRIVATE ${THIRD_INCLUDE} ../zecm/source)

    # MSVC multi-config: CTest needs explicit per-config exe path (no $<TARGET_FILE> in add_test on CMake 3.9)
    if(MSVC)
      # VS multi-config generators may leave CMAKE_BUILD_TYPE empty.
      # Prefer BUILD_OPT (debug/release) from top-level CI/config flags.
      set(_motapp_ctest_cfg "Debug")
      if(BUILD_OPT STREQUAL "release")
        set(_motapp_ctest_cfg "Release")
      elseif(BUILD_OPT STREQUAL "debug")
        set(_motapp_ctest_cfg "Debug")
      elseif(CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
        set(_motapp_ctest_cfg "${CMAKE_BUILD_TYPE}")
      endif()
      set(MOTAPP_EXE_FOR_CTEST "${CMAKE_CURRENT_BINARY_DIR}/${_motapp_ctest_cfg}/${func_name}.exe")
      set(MOTAPP_DEMO_EXE_FOR_CTEST "${CMAKE_CURRENT_BINARY_DIR}/demo/${_motapp_ctest_cfg}/zmc_motapp_demo.exe")
    else()
      set(MOTAPP_EXE_FOR_CTEST "${CMAKE_CURRENT_BINARY_DIR}/${func_name}")
      set(MOTAPP_DEMO_EXE_FOR_CTEST "${CMAKE_CURRENT_BINARY_DIR}/demo/zmc_motapp_demo")
    endif()

    install(TARGETS ${func_name} DESTINATION bin/${BUILD_OPT})

    set(MOTAPP_TEST_CS_NAME ${func_name})
    add_subdirectory(test)
    target_link_libraries(${func_name} PRIVATE motapp_test_runtime)
endif()
