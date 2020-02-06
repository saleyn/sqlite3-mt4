cmake_minimum_required(VERSION 3.0)

set( MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO")

project(sqlite3 VERSION 1.0)

#===============================================================================
# CMAKE options customization
#===============================================================================
OPTION(NO_MQT_LM OFF)
set(CMAKE_SYSTEM_PROCESSOR "i686")
set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "install")
endif()

if (WIN32)

  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#    if (CMAKE_BUILD_TYPE MATCHES "(Debug|debug|DEBUG)")
#      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
#    else()
#      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
#    endif()

  add_compile_definitions(
  	_CRT_SECURE_NO_WARNINGS
    UNICODE _UNICODE _CONSOLE
  )

  set(CMAKE_MFC_FLAG 0)

endif()

add_compile_options(/wd26451 /wd6011 /wd4244)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO" )

#-------------------------------------------------------------------------------
# Target projects:
#-------------------------------------------------------------------------------
add_subdirectory(src)

