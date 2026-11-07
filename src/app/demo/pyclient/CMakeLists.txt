# Date: 2026-10-22. Drafted by: GPT-6 (Codex).
option(ENABLE_RPC_PYTHON "Build CPython zmc_rpc extension" OFF)
option(ENABLE_RPC_PYTHON_LIVE_TESTS "Run CPython zmc_rpc tests against a live RPC server" OFF)
set(ZMC_RPC_PYTHON_TEST_HOST "127.0.0.1" CACHE STRING "Live zmc_rpc test host")
set(ZMC_RPC_PYTHON_TEST_PORT "" CACHE STRING "Live zmc_rpc test port")

if(ENABLE_RPC_PYTHON)
    find_package(PythonInterp REQUIRED)
    find_package(PythonLibs REQUIRED)

    add_library(zmc_rpc MODULE
        ZmcRpcPy.cpp
        ../rpc_client/source/zml_motion_api.cpp)
    target_include_directories(zmc_rpc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../rpc_client/include
        ${PYTHON_INCLUDE_DIRS})
    target_compile_definitions(zmc_rpc PRIVATE ZML_EXPORTS)
    target_link_libraries(zmc_rpc rpc ${PYTHON_LIBRARIES})
    set_target_properties(zmc_rpc PROPERTIES PREFIX "")
    if(WIN32)
        set_target_properties(zmc_rpc PROPERTIES SUFFIX ".pyd")
    endif()

    # Legacy add_test() + fixed paths: avoid $<TARGET_FILE*> NOT_AVAILABLE without -C.
    set(_zmc_rpc_ctest_cfg "Debug")
    if(BUILD_OPT STREQUAL "release")
        set(_zmc_rpc_ctest_cfg "Release")
    elseif(BUILD_OPT STREQUAL "debug")
        set(_zmc_rpc_ctest_cfg "Debug")
    elseif(CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
        set(_zmc_rpc_ctest_cfg "${CMAKE_BUILD_TYPE}")
    endif()
    if(MSVC)
        set(_zmc_rpc_py_dir "${CMAKE_CURRENT_BINARY_DIR}/${_zmc_rpc_ctest_cfg}")
        set(_zmc_rpc_demo_exe
            "${CMAKE_CURRENT_BINARY_DIR}/../${_zmc_rpc_ctest_cfg}/zmc_motapp_demo.exe")
    else()
        set(_zmc_rpc_py_dir "${CMAKE_CURRENT_BINARY_DIR}")
        set(_zmc_rpc_demo_exe "${CMAKE_CURRENT_BINARY_DIR}/../zmc_motapp_demo")
    endif()

    set(_zmc_rpc_py_test_env "PYTHONPATH=${_zmc_rpc_py_dir}")
    if(ENABLE_RPC_PYTHON_LIVE_TESTS)
        if("${ZMC_RPC_PYTHON_TEST_PORT}" STREQUAL "")
            message(FATAL_ERROR "ZMC_RPC_PYTHON_TEST_PORT is required when ENABLE_RPC_PYTHON_LIVE_TESTS=ON")
        endif()
        add_dependencies(zmc_rpc zmc_motapp_demo)
        set(_zmc_rpc_py_test_env
            "${_zmc_rpc_py_test_env};ZMC_RPC_HOST=${ZMC_RPC_PYTHON_TEST_HOST};ZMC_RPC_PORT=${ZMC_RPC_PYTHON_TEST_PORT};ZMC_RPC_SERVER_EXE=${_zmc_rpc_demo_exe};ZMC_RPC_TEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/live;ZMC_RPC_TEST_DATA_DIR=${CMAKE_SOURCE_DIR}/src/mhelper/data")
    endif()

    add_test(dev_test_zmc_rpc_py
        ${PYTHON_EXECUTABLE} -B -m unittest -v tests.test_zmc_rpc)
    set_tests_properties(dev_test_zmc_rpc_py PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        ENVIRONMENT "${_zmc_rpc_py_test_env}"
        TIMEOUT 180
        RUN_SERIAL TRUE)

    install(TARGETS zmc_rpc DESTINATION bin/${BUILD_OPT})
    install(FILES zmc_rpc.pyi DESTINATION bin/${BUILD_OPT})
endif()
