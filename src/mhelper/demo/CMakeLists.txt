include_directories("..")
file(GLOB demo_mhelper_source *.cpp)

set( lib_list ${lib_list} mhelper)
add_executable(zmc_mhelper_demo ${demo_mhelper_source})
target_link_libraries(zmc_mhelper_demo ${lib_list} ${OS_PLATFORM_LIBS}) 
add_dependencies(zmc_mhelper_demo ${lib_list})


install(TARGETS zmc_mhelper_demo DESTINATION  bin/${BUILD_OPT})
