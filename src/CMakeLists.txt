cmake_minimum_required(VERSION 3.0)

#-------------------------------------------------------------------------------
# Target projects:
#-------------------------------------------------------------------------------
add_library(sqlite3_wrapper SHARED
  ../lib/sqlite3.c
  ../lib/extension-functions.c
  sqlite3_wrapper.c
)
target_include_directories(sqlite3_wrapper PUBLIC "${CMAKE_SOURCE_DIR}/lib")
target_compile_definitions(sqlite3_wrapper PUBLIC _USRDLL _WINDLL)
#target_compile_options(sqlite3_wrapper PRIVATE /wd26451 /wd6011 /wd4244)
target_link_libraries(sqlite3_wrapper Shlwapi)
set_target_properties(sqlite3_wrapper PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")


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
install(TARGETS sqlite3_wrapper sqlite3_dll sqlite3 RUNTIME DESTINATION .)
#install(TARGETS sqlite3_wrapper sqlite3_dll sqlite3 RUNTIME DESTINATION bin)