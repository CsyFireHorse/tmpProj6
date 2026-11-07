include_directories("..")
file(GLOB traj_demo_source *.cpp )
add_executable(zmc_traj_demo ${traj_demo_source})
target_link_libraries(zmc_traj_demo ${lib_name} motion_buffer)
add_dependencies(zmc_traj_demo ${lib_name} motion_buffer)

install(TARGETS zmc_traj_demo DESTINATION  bin/${BUILD_OPT})
