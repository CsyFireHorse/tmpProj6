include_directories(".")
include_directories("../..")
include_directories("include")
include_directories("../../vel")
include_directories("../../vel/include")

set( lib_name jserialize)
file(GLOB_RECURSE jserialize_src_src_temp1 source/*.h* source/*.c*)
set(jserialize_src "${jserialize_src_src_temp1}" CACHE STRING INTERNAL FORCE)
add_library(jserialize ${jserialize_src} )

target_compile_definitions(jserialize PRIVATE ZMC_DLLBUILD)
target_link_libraries(jserialize base )

add_subdirectory("demo")
