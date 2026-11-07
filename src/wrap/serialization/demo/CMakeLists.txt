include_directories("..")
file(GLOB vel_demo_source *.cpp )
add_executable(zmc_jser_demo ${vel_demo_source})
target_link_libraries(zmc_jser_demo ${lib_name} vel) 
add_dependencies(zmc_jser_demo  ${lib_name})

install(TARGETS zmc_jser_demo DESTINATION  bin/${BUILD_OPT})
