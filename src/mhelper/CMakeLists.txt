cmake_minimum_required(VERSION 2.6.4)


set(MAJOR_VERSION 1)
set(MINOR_VERSION 1)
set(PATCH_VERSION 0)


if (BUILD_SHARED_LIBS)
  set(LIB_DLLEXPORT "ZMC_DLLPORT ")
endif()
set(LIB_NAME_UPPER "MHELPER")
set(LIB_NAME_LOWER "mhelper")


set( lib_name ${LIB_NAME_LOWER})
set( func_name "test_cs_${lib_name}")


project( ${func_name} CXX C )


file(GLOB ${func_name}_src_temp *.h* *.c* source/*.c*   ../utils/source/*.c*)
set(${func_name}_src "${${func_name}_src_temp}" CACHE STRING INTERNAL FORCE)

include_directories(".")
include_directories("..")
include_directories("../vel")
include_directories("../thirdlib")
include_directories("../utils")


set(LIBRARY_SRC "${LIBRARY_SRC}" "${${func_name}_src}" CACHE STRING INTERNAL FORCE)

set( lib_list  base )


add_library(${lib_name} ${${func_name}_src} )
target_compile_definitions(${lib_name} PRIVATE ZMC_DLLBUILD)
target_link_libraries(${lib_name} ${lib_list})

install(TARGETS ${lib_name} DESTINATION lib/${BUILD_OPT})
install(TARGETS ${lib_name} DESTINATION bin/${BUILD_OPT})
file(GLOB LIB_HEADFILES ${PROJECT_SOURCE_DIR}/*.h* ${PROJECT_BINARY_DIR}/_config/version.h)
install(FILES ${LIB_HEADFILES} DESTINATION include/${lib_name})


if (ENABLE_DEV_UNITTEST)
    enable_testing()
    file(GLOB_RECURSE ${func_name}_test_src test/*.h* test/*.c*)

    # ${TEST_MAIN} 
    add_executable(${func_name} ${${func_name}_test_src})
    target_link_libraries(${func_name} ${THIRD_LIBS} ${lib_name})
    target_include_directories(${func_name} PRIVATE ${THIRD_INCLUDE})


    if (MSVC)
        # install(FILES $<TARGET_FILE:velocity> DESTINATION ${PROJECT_BINARY_DIR}/${BUILD_OPT})
        # install(FILES $<TARGET_FILE:trajectory> DESTINATION ${PROJECT_BINARY_DIR}/${BUILD_OPT})
    endif()


    add_test(dev_test_${func_name} ${func_name})
    install(TARGETS ${func_name} DESTINATION bin/${BUILD_OPT})

endif()

add_subdirectory("demo")
