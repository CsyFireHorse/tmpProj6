include_directories("..")
file(GLOB_RECURSE base_plcfb_source *.cpp axfb/*.cpp axgrp/*.cpp motbuf/*.cpp cam/*.cpp)
add_executable(zmc_plcfb_demo ${base_plcfb_source})
target_link_libraries(zmc_plcfb_demo ${lib_name}) 
add_dependencies(zmc_plcfb_demo  ${lib_name})

install(TARGETS zmc_plcfb_demo DESTINATION  bin/${BUILD_OPT})
