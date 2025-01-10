get_filename_component(VCPKG_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mgl" for configuration "Debug"
set_property(TARGET mgl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mgl PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/mgld.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/mgld.dll"
  )

list(APPEND _cmake_import_check_targets mgl )
list(APPEND _cmake_import_check_files_for_mgl "${_IMPORT_PREFIX}/debug/lib/mgld.lib" "${_IMPORT_PREFIX}/debug/bin/mgld.dll" )

# Import target "mgltask" for configuration "Debug"
set_property(TARGET mgltask APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mgltask PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/tools/mathgl/mgltask.exe"
  )

list(APPEND _cmake_import_check_targets mgltask )
list(APPEND _cmake_import_check_files_for_mgltask "${_IMPORT_PREFIX}/tools/mathgl/mgltask.exe" )

# Import target "mglconv" for configuration "Debug"
set_property(TARGET mglconv APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mglconv PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/tools/mathgl/mglconv.exe"
  )

list(APPEND _cmake_import_check_targets mglconv )
list(APPEND _cmake_import_check_files_for_mglconv "${_IMPORT_PREFIX}/tools/mathgl/mglconv.exe" )

# Import target "mgl.cgi" for configuration "Debug"
set_property(TARGET mgl.cgi APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mgl.cgi PROPERTIES
  IMPORTED_LOCATION_DEBUG "${VCPKG_IMPORT_PREFIX}/debug/lib/cgi-bin/mgl.cgi.exe"
  )

list(APPEND _cmake_import_check_targets mgl.cgi )
list(APPEND _cmake_import_check_files_for_mgl.cgi "${VCPKG_IMPORT_PREFIX}/debug/lib/cgi-bin/mgl.cgi.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
