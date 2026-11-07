# pcieHost demo: drives mc_pdo_thread against the ecatFak/pci_zecm facade.

include_directories(".")
include_directories("..")

file(GLOB demo_pcieHost_source *.cpp mc_core/*.cpp)
list(APPEND demo_pcieHost_source
    ${CMAKE_CURRENT_SOURCE_DIR}/../httpServer_demo.cpp
)

# motapp provides AxisFbBridge; mhelper provides ENI/PDO map helpers for mc_motion.
set(lib_list ${lib_list} motapp mhelper)

add_executable(zmc_pcie_host_demo ${demo_pcieHost_source})

if (NOT USE_PCI_ZECM_FAK)
  target_compile_definitions(zmc_pcie_host_demo PRIVATE DISABLE_PCI_ZECM_FAK)
endif()

target_link_libraries(zmc_pcie_host_demo ${lib_list} ${ZECM_APP_LIBS} ${OS_PLATFORM_LIBS})
add_dependencies(zmc_pcie_host_demo ${lib_list})

install(TARGETS zmc_pcie_host_demo DESTINATION bin/${BUILD_OPT})
