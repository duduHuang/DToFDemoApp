get_filename_component(VCPKG_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mgl" for configuration "Release"
set_property(TARGET mgl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mgl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/mgl.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/mgl.dll"
  )

list(APPEND _cmake_import_check_targets mgl )
list(APPEND _cmake_import_check_files_for_mgl "${_IMPORT_PREFIX}/lib/mgl.lib" "${_IMPORT_PREFIX}/bin/mgl.dll" )

# Import target "mgltask" for configuration "Release"
set_property(TARGET mgltask APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mgltask PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/tools/mathgl/mgltask.exe"
  )

list(APPEND _cmake_import_check_targets mgltask )
list(APPEND _cmake_import_check_files_for_mgltask "${_IMPORT_PREFIX}/tools/mathgl/mgltask.exe" )

# Import target "mglconv" for configuration "Release"
set_property(TARGET mglconv APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mglconv PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/tools/mathgl/mglconv.exe"
  )

list(APPEND _cmake_import_check_targets mglconv )
list(APPEND _cmake_import_check_files_for_mglconv "${_IMPORT_PREFIX}/tools/mathgl/mglconv.exe" )

# Import target "mgl.cgi" for configuration "Release"
set_property(TARGET mgl.cgi APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mgl.cgi PROPERTIES
  IMPORTED_LOCATION_RELEASE "${VCPKG_IMPORT_PREFIX}/lib/cgi-bin/mgl.cgi.exe"
  )

list(APPEND _cmake_import_check_targets mgl.cgi )
list(APPEND _cmake_import_check_files_for_mgl.cgi "${VCPKG_IMPORT_PREFIX}/lib/cgi-bin/mgl.cgi.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
