include_directories("include")
# AI modification signature: GPT-6 Codex.

if (BUILD_SHARED_LIBS)
  set(LIB_DLLEXPORT "ZMC_DLLPORT ")
endif()
set(LIB_NAME_UPPER "BASE")
set(LIB_NAME_LOWER "base")


set( lib_name ${LIB_NAME_LOWER})
set( func_name "test_cs_${lib_name}")


file(GLOB ${func_name}_src_temp *.h* *.c* source/*.c*)
set(${func_name}_src "${${func_name}_src_temp}" CACHE STRING INTERNAL FORCE)


add_library(${lib_name} ${${func_name}_src})
target_include_directories(${lib_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(${lib_name} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
find_package(Threads REQUIRED)
target_link_libraries(${lib_name} PRIVATE Threads::Threads)
target_compile_definitions(${lib_name} PRIVATE ZMC_DLLBUILD)
install(TARGETS ${lib_name} DESTINATION lib/${BUILD_OPT})
install(TARGETS ${lib_name} DESTINATION bin/${BUILD_OPT})
file(GLOB LIB_HEADFILES ${PROJECT_SOURCE_DIR}/*.h* )
install(FILES ${LIB_HEADFILES} DESTINATION include/${lib_name})
install(FILES include/IVfs.hpp include/MemoryVfs.hpp DESTINATION include/${lib_name})

include_directories(".")
add_subdirectory(demo)

if (ENABLE_DEV_UNITTEST)
    enable_testing()
    file(GLOB_RECURSE base_test_src test/*.h* test/*.c*)

    add_executable(${func_name} ${base_test_src})
    target_link_libraries(${func_name} PRIVATE ${THIRD_LIBS} ${lib_name} Threads::Threads)
    set_target_properties(${func_name} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
    target_include_directories(${func_name} PRIVATE ${THIRD_INCLUDE})


    add_test(dev_test_${func_name} ${func_name})
    install(TARGETS ${func_name} DESTINATION bin/${BUILD_OPT})

endif()

if (USE_BENCH)
    add_subdirectory(bench)
endif()
