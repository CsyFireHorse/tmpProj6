include_directories("..")
include_directories(".")
file(GLOB base_demo_source *.c* stl/*.cpp container/*.cpp common/*.c* tmpl/*.c* misc/*.c* cpp/*.c* pattern/*.c* common/misc/*.c*)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(zmc_base_demo ${base_demo_source})
target_link_libraries(zmc_base_demo ${lib_name}) 
add_dependencies(zmc_base_demo  ${lib_name})

install(TARGETS zmc_base_demo DESTINATION  bin/${BUILD_OPT})
