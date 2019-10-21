#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mycat" for configuration ""
set_property(TARGET mycat APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(mycat PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "/home/midren/Documents/ucu/os/myshell/mycat/bin/mycat"
  )

list(APPEND _IMPORT_CHECK_TARGETS mycat )
list(APPEND _IMPORT_CHECK_FILES_FOR_mycat "/home/midren/Documents/ucu/os/myshell/mycat/bin/mycat" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
