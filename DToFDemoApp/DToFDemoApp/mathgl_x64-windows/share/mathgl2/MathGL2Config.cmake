get_filename_component(VCPKG_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
# - Config file for the MathGL package
# It defines the following variables
 
set(MathGL2_INCLUDE_DIRS "${VCPKG_IMPORT_PREFIX}/include")
if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
  set(MathGL2_LIBRARIES_DIRS "${VCPKG_IMPORT_PREFIX}/debug/lib")
else()
  set(MathGL2_LIBRARIES_DIRS "${VCPKG_IMPORT_PREFIX}/lib")
endif()
set(MathGL2_HAVE_QT5 "OFF")
set(MathGL2_HAVE_QT4 "")
set(MathGL2_HAVE_WX "OFF")
set(MathGL2_HAVE_FLTK "OFF")
set(MathGL2_HAVE_GLUT "OFF")
set(MathGL2_HAVE_PTHREAD "")
set(MathGL2_HAVE_OPENMP "ON")

include(CMakeFindDependencyMacro)
# Adding dependency for Threads imported target
if (MathGL2_HAVE_PTHREAD STRGREATER "")
  find_dependency(Threads)
endif()

# Adding dependency for OpenMP imported target
if (MathGL2_HAVE_OPENMP STRGREATER "")
  find_dependency(OpenMP)
endif()
if("OFF")
  find_dependency(Armadillo CONFIG)
endif()
if(MathGL2_HAVE_FLTK)
  find_dependency(FLTK CONFIG)
endif()

# Compute paths
get_filename_component(MathGL2_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Our library dependencies (contains definitions for IMPORTED targets)
if(NOT TARGET mgl AND NOT MathGL_BINARY_DIR)
  include("${MathGL2_CMAKE_DIR}/MathGLTargets.cmake")
endif()
