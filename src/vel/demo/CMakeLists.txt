include_directories("..")
file(GLOB vel_demo_source *.cpp )
add_executable(zmc_vel_demo ${vel_demo_source})
target_link_libraries(zmc_vel_demo ${lib_name}) 
add_dependencies(zmc_vel_demo  ${lib_name})

install(TARGETS zmc_vel_demo DESTINATION  bin/${BUILD_OPT})
