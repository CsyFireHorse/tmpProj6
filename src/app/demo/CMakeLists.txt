include_directories("..")
file(GLOB demo_motapp_source *.cpp)

set( lib_list ${lib_list} motapp)
add_executable(zmc_motapp_demo ${demo_motapp_source})
target_link_libraries(zmc_motapp_demo ${lib_list} ${ZECM_APP_LIBS} ${OS_PLATFORM_LIBS} ) 
add_dependencies(zmc_motapp_demo ${lib_list})

if(ENABLE_RPC)
    target_compile_definitions(zmc_motapp_demo PRIVATE ZMC_ENABLE_RPC)
    target_include_directories(zmc_motapp_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src/thirdlib/rpclib/include)
endif()

if (NOT USE_PCI_ZECM_FAK)
  target_compile_definitions(zmc_motapp_demo PRIVATE DISABLE_PCI_ZECM_FAK)
endif()

install(TARGETS zmc_motapp_demo DESTINATION  bin/${BUILD_OPT})
