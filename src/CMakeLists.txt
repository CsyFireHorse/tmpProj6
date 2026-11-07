if (BUILD_SHARED_LIBS)
  set(LIB_DLLEXPORT "ZMC_DLLPORT ")
endif()

set(THIRD_LIBS gmock gtest)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(".")
include_directories("common")
include_directories("common/zmc")
include_directories("thirdlib")
include_directories("pfb/include")
include_directories("pfb")
include_directories("mc")
enable_testing()

add_subdirectory(base)
add_subdirectory(vel)
add_subdirectory(traj)
add_subdirectory(fb)
add_subdirectory(mcapi)
add_subdirectory(plcapi)
# add_subdirectory(pfb)
# add_subdirectory(fb)
add_subdirectory(mhelper)

option(USE_ZECM_FAK "Builds zecm fak " ON)
option(USE_PCI_ZECM_FAK "Builds pci_zecm fak " ON)

# Decide which libs app/demo should link against (variable passed down)
# Default: both faks enabled -> use local ecatFak only
set(ZECM_APP_LIBS ecatFak)

# Case: ecatFak{zecm_fak.cpp} + external pci_zecm.lib
if (USE_ZECM_FAK AND NOT USE_PCI_ZECM_FAK)
  set(ZECM_APP_LIBS ecatFak pci_zecm)
endif()

# Case: external zecm.lib + ecatFak{pci_zecm_fak.cpp}
if (NOT USE_ZECM_FAK AND USE_PCI_ZECM_FAK)
  set(ZECM_APP_LIBS ecatFak zecm)
endif()
message( "ZECM_APP_LIBS=" ${ZECM_APP_LIBS})


if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  if(DEFINED CMAKE_SYSTEM_PROCESSOR)
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
      set(USE_ZECM_FAK OFF)
      set(USE_PCI_ZECM_FAK ON)
      set(ZECM_APP_LIBS ecatFak zecm)
      # guess the architecture of the compiler
      if(CMAKE_CXX_COMPILER MATCHES "aarch64")
        # if(${CMAKE_CXX_COMPILER} STREQUAL "aarch64-none-linux-gnu-g++")
        set(ZECM_LIB_PATH "${CMAKE_SOURCE_DIR}/src/zecm/lib/aarch64")
        message("use zecm aarch64")
      else()
        set(ZECM_LIB_PATH "${CMAKE_SOURCE_DIR}/src/zecm/lib/armv7")
        message("use zecm armv7")
      endif()
    else()
      if (NOT USE_PCI_ZECM_FAK)
        set(ZECM_LIB_PATH "${CMAKE_SOURCE_DIR}/src/zecm/lib/amd64")
        message("use pci_zecm amd64")
      endif()
    endif()
  endif()
endif()


if (WIN32)
  # if (MINGW OR MSYS)
  #   message(WARNING "Detected MSYS2/MINGW environment - please make sure the prebuilt win32/win64 DLLs are compatible.")
  # endif()
  if (NOT USE_PCI_ZECM_FAK)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(ZECM_LIB_PATH "${CMAKE_SOURCE_DIR}/src/zecm/lib/win64")
      message("use pci_zecm win64")
    else()
      set(ZECM_LIB_PATH "${CMAKE_SOURCE_DIR}/src/zecm/lib/win32")
      message("use pci_zecm win32")
    endif()
  endif()
endif()



# Always build local ecatFak/zecm faker library

if (NOT USE_ZECM_FAK OR NOT USE_PCI_ZECM_FAK)
  message("ZECM_LIB_PATH=" ${ZECM_LIB_PATH})  
  link_directories(${ZECM_LIB_PATH})
endif()

add_subdirectory("zecm")


if (ENABLE_PYBIND11)
  add_subdirectory("pyfb")
endif()

if (ENABLE_LUA_AXFB)
  add_subdirectory("wrap/lua")
endif()

include_directories("zecm")
add_subdirectory(app)
# add_subdirectory(wrap)
