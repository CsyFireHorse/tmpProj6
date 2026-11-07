include_directories("..")
file(GLOB demo_motapp_source *.cpp)

# set( lib_list ${lib_list} motapp)
add_executable(zmc_pcie_demo ${demo_motapp_source})

if (NOT USE_PCI_ZECM_FAK)
  set(ZECM_APP_LIBS pci_zecm)
  target_compile_definitions(zmc_pcie_demo PRIVATE DISABLE_PCI_ZECM_FAK)
endif()

target_link_libraries(zmc_pcie_demo ${lib_list} ${ZECM_APP_LIBS} ${OS_PLATFORM_LIBS} ) 
add_dependencies(zmc_pcie_demo ${lib_list})



install(TARGETS zmc_pcie_demo DESTINATION  bin/${BUILD_OPT})
