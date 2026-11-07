include_directories(".")
include_directories("include")
include_directories("../vel")
include_directories("../../vel")
set( lib_name traj)
file(GLOB plcfb_src source/*.c*)
# file(GLOB_RECURSE plcfb_src_temp1 source/*.h* source/*.c*)
# set(plcfb_src "${plcfb_src_temp1}" CACHE STRING INTERNAL FORCE)
add_library(traj ${plcfb_src} )

target_compile_definitions(traj PRIVATE ZMC_DLLBUILD)
target_link_libraries(traj base)


file(GLOB motion_buf_src source/motion_buffer/*.c*)
add_library(motion_buffer ${motion_buf_src} )

# Keep the trajectory and velocity libraries independent; the motion buffer composes both.

target_link_libraries(motion_buffer traj vel)

install(TARGETS traj DESTINATION lib/${BUILD_OPT})
install(TARGETS traj DESTINATION bin/${BUILD_OPT})
install(TARGETS motion_buffer DESTINATION lib/${BUILD_OPT})
install(TARGETS motion_buffer DESTINATION bin/${BUILD_OPT})
file(GLOB LIB_HEADFILES ${PROJECT_SOURCE_DIR}/*.h* )
install(FILES ${LIB_HEADFILES} DESTINATION include/traj)

add_subdirectory(demo)
# add_subdirectory(cli)

if (ENABLE_DEV_UNITTEST)
    enable_testing()
    file(GLOB_RECURSE traj_test_src test/*.h* test/*.c*)

    add_executable(test_traj ${traj_test_src})
    target_link_libraries(test_traj ${THIRD_LIBS} traj motion_buffer)
    # target_include_directories(base PRIVATE ${THIRD_INCLUDE})
    add_test(traj test_traj )
    install(TARGETS test_traj DESTINATION bin/${BUILD_OPT})
endif()

if (USE_BENCH)
    # add_subdirectory(bench)
endif()
