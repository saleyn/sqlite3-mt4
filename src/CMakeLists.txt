cmake_minimum_required(VERSION 3.0)

#-------------------------------------------------------------------------------
# Target projects:
#-------------------------------------------------------------------------------
add_library(mqt-splite3 SHARED
  ../lib/sqlite3.c
  ../lib/extension-functions.c
  sqlite3_wrapper.c
)
target_include_directories(mqt-splite3 PUBLIC "${CMAKE_SOURCE_DIR}/lib")
target_compile_definitions(mqt-splite3 PUBLIC _USRDLL _WINDLL SQLITE3WRAPPER_EXPORTS)
#target_compile_options(mqt-splite3 PRIVATE /wd26451 /wd6011 /wd4244)
target_link_libraries(mqt-splite3 Shlwapi)
set_target_properties(mqt-splite3 PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
if(WIN32)
  if (CMAKE_CL_64 OR CMAKE_GENERATOR_PLATFORM EQUAL "x64")
    MESSAGE("==> 64 bit build")
    set_target_properties(mqt-splite3 PROPERTIES SUFFIX ".x64.dll")
  else()
    MESSAGE("==> 32 bit build")
    set_target_properties(mqt-splite3 PROPERTIES SUFFIX ".x86.dll")
  endif()
endif()

add_library(sqlite3_dll SHARED
  ../lib/sqlite3.c
  ../lib/extension-functions.c
)
set_target_properties(sqlite3_dll PROPERTIES OUTPUT_NAME "sqlite3")
target_include_directories(sqlite3_dll PUBLIC "${CMAKE_SOURCE_DIR}/lib")
target_compile_definitions(sqlite3_dll PUBLIC _USRDLL _WINDLL)
#target_compile_options(sqlite3_dll PRIVATE /wd26451 /wd6011 /wd4244)
set_target_properties(sqlite3_dll PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")

add_executable(sqlite3
  ../lib/shell.c
  ../lib/sqlite3.c
  ../lib/extension-functions.c
)
target_include_directories(sqlite3 PUBLIC "${CMAKE_SOURCE_DIR}/lib")
#target_compile_options(sqlite3 PRIVATE /wd26451 /wd6011 /wd4244)
set_target_properties(sqlite3 PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")


# "RUNTIME" forces to copy the *.dll only and not *.lib file
# If the *.lib is needed then use the "ARCHIVE" specification
install(TARGETS mqt-splite3 sqlite3_dll sqlite3 RUNTIME DESTINATION .)
#install(TARGETS mqt-splite3 sqlite3_dll sqlite3 RUNTIME DESTINATION bin)