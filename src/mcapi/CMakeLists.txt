include_directories(".")
include_directories("include")
include_directories("include/plc")
include_directories("../fb")
include_directories("../fb/include")
include_directories("../fb/include/fb")
include_directories("../fb/include/axis")
include_directories("../traj/include")
include_directories("../vel/include")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set( lib_name mcapi)
# file(GLOB plcfb_src_temp0 *.h*)
file(GLOB_RECURSE plcfb_src_temp1 source/*.h* source/*.c* source/plc/*.c* ) 
# source/*.c* 
set(plcfb_src "${plcfb_src_temp1}" CACHE STRING INTERNAL FORCE)
add_library(mcapi SHARED  ${plcfb_src} )
# add_definitions(-DBUILD_SHARED_LIBS)

target_compile_definitions(mcapi PRIVATE ZMC_DLLBUILD)
target_link_libraries(mcapi plcfb vel traj)

install(TARGETS mcapi DESTINATION lib/${BUILD_OPT})
install(TARGETS mcapi DESTINATION bin/${BUILD_OPT})
file(GLOB LIB_HEADFILES ${PROJECT_SOURCE_DIR}/*.h* )
install(FILES ${LIB_HEADFILES} DESTINATION include/mcapi)

include_directories(".")
add_subdirectory(demo)

if (ENABLE_DEV_UNITTEST)
    file(GLOB_RECURSE mcapi_test_src test/*.h* test/*.c*)
    include_directories("../vel/include")
    include_directories("../fb/include/misc")
    include_directories("../fb/include/impl")

    add_executable(test_mcapi ${mcapi_test_src})
    target_link_libraries(test_mcapi ${THIRD_LIBS} mcapi)
    # target_include_directories(base PRIVATE ${THIRD_INCLUDE})
    # add_test(test_mcapi)
    install(TARGETS test_mcapi DESTINATION bin/${BUILD_OPT})
endif()