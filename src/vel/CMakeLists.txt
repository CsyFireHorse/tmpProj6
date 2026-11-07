include_directories(".")
include_directories("include")
include_directories("../base/include")
set( lib_name vel)
file(GLOB plcfb_src source/*.c*)
# file(GLOB_RECURSE plcfb_src_temp1 source/*.h* source/*.c*)
# set(plcfb_src "${plcfb_src_temp1}" CACHE STRING INTERNAL FORCE)
add_library(vel ${plcfb_src} )

target_compile_definitions(vel PRIVATE ZMC_DLLBUILD)
target_link_libraries(vel base)

install(TARGETS vel DESTINATION lib/${BUILD_OPT})
install(TARGETS vel DESTINATION bin/${BUILD_OPT})
file(GLOB LIB_HEADFILES ${PROJECT_SOURCE_DIR}/*.h* )
install(FILES ${LIB_HEADFILES} DESTINATION include/vel)

add_subdirectory(demo)
# add_subdirectory(cli)

if (ENABLE_DEV_UNITTEST)
    enable_testing()
    file(GLOB_RECURSE vel_test_src test/*.h* test/*.c*)

    add_executable(test_vel ${vel_test_src})
    target_link_libraries(test_vel ${THIRD_LIBS} vel)
    # target_include_directories(base PRIVATE ${THIRD_INCLUDE})
    add_test(vel test_vel )
    install(TARGETS test_vel DESTINATION bin/${BUILD_OPT})
endif()

if (USE_BENCH)
    add_subdirectory(bench)
endif()