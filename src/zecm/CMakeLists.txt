cmake_minimum_required(VERSION 2.6.4)


set(MAJOR_VERSION 1)
set(MINOR_VERSION 1)
set(PATCH_VERSION 0)


if (BUILD_SHARED_LIBS)
  set(LIB_DLLEXPORT "ZMC_DLLPORT ")
endif()

# Rename to ecatFak library
set(LIB_NAME_UPPER "ECATFAK")
set(LIB_NAME_LOWER "ecatFak")

set( lib_name ${LIB_NAME_LOWER})
set( func_name "test_cs_${lib_name}")


project(${func_name} CXX C)

include_directories(".")
include_directories("include")
include_directories("..")
include_directories("../mhelper")

# Always compile all sources in this directory (including all cpp in source/)
file(GLOB ${func_name}_src_temp *.h* *.c* source/*.c*)
set(${func_name}_src "${${func_name}_src_temp}" CACHE STRING INTERNAL FORCE)

set(LIBRARY_SRC "${LIBRARY_SRC}" "${${func_name}_src}" CACHE STRING INTERNAL FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_library(${lib_name} ${${func_name}_src})
target_compile_definitions(${lib_name} PRIVATE ZMC_DLLBUILD)

# Macros for conditional compilation in fak sources (controlled inside cpp via #ifdef)
if (NOT USE_ZECM_FAK)
  target_compile_definitions(${lib_name} PRIVATE DISABLE_ZECM_FAK)
endif()
if (NOT USE_PCI_ZECM_FAK)
  # use it with  `ifndef DISABLE_PCI_ZECM_FAK` in the source file
  # it is more readable than using #ifdef USE_PCI_ZECM_FAK
  target_compile_definitions(${lib_name} PRIVATE DISABLE_PCI_ZECM_FAK)
endif()

target_link_libraries(${lib_name} base mhelper)

install(TARGETS ${lib_name} DESTINATION lib/${BUILD_OPT})
install(TARGETS ${lib_name} DESTINATION bin/${BUILD_OPT})
file(GLOB LIB_HEADFILES ${PROJECT_SOURCE_DIR}/*.h* )
install(FILES ${LIB_HEADFILES} DESTINATION include/${lib_name})
# add_subdirectory(demo)

