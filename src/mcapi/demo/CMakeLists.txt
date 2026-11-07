include_directories("..")
include_directories("../../fb/include/impl")
file(GLOB_RECURSE mcapi_demo_source *.cpp )
add_executable(zmc_mcapi_demo ${mcapi_demo_source})
target_link_libraries(zmc_mcapi_demo ${lib_name}) 
add_dependencies(zmc_mcapi_demo  ${lib_name})

install(TARGETS zmc_mcapi_demo DESTINATION  bin/${BUILD_OPT})
