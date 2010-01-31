# - Find a VTKMS installation or build tree.
# The following variables are set if VTKMS is found.  If VTKMS is not
# found, VTKMS_FOUND is set to false.
#  VTKMS_FOUND         - Set to true when VTKMS is found.
#  VTKMS_USE_FILE      - CMake file to use VTKMS.
#  VTKMS_MAJOR_VERSION - The VTKMS major version number.
#  VTKMS_MINOR_VERSION - The VTKMS minor version number 
#                       (odd non-release).
#  VTKMS_BUILD_VERSION - The VTKMS patch level 
#                       (meaningless for odd minor).
#  VTKMS_INCLUDE_DIRS  - Include directories for VTKMS
#  VTKMS_LIBRARY_DIRS  - Link directories for VTKMS libraries
#  VTKMS_KITS          - List of VTKMS kits, in CAPS 
#                      (COMMON,IO,) etc.
#  VTKMS_LANGUAGES     - List of wrapped languages, in CAPS
#                      (TCL, PYHTON,) etc.
# The following cache entries must be set by the user to locate VTKMS:
#  VTKMS_DIR  - The directory containing VTKMSConfig.cmake.  
#             This is either the root of the build tree,
#             or the lib/vtkms directory.  This is the 
#             only cache entry.
# The following variables are set for backward compatibility and
# should not be used in new code:
#  USE_VTKMS_FILE - The full path to the UseVTKMS.cmake file.
#                 This is provided for backward 
#                 compatibility.  Use VTKMS_USE_FILE 
#                 instead.
#

# Construct consitent error messages for use below.
SET(VTKMS_DIR_DESCRIPTION "directory containing VTKMSConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/VTKMS for an installation.  For VTKMS 4.0, this is the location of UseVTKMS.cmake.  This is either the root of the build tree or PREFIX/include/vtkms for an installation.")
SET(VTKMS_DIR_MESSAGE "VTKMS not found.  Set the VTKMS_DIR cmake cache entry to the ${VTKMS_DIR_DESCRIPTION}")

# Search only if the location is not already known.
IF(NOT VTKMS_DIR)
  # Get the system search path as a list.
  IF(UNIX)
    STRING(REGEX MATCHALL "[^:]+" VTKMS_DIR_SEARCH1 "$ENV{PATH}")
  ELSE(UNIX)
    STRING(REGEX REPLACE "\\\\" "/" VTKMS_DIR_SEARCH1 "$ENV{PATH}")
  ENDIF(UNIX)
  STRING(REGEX REPLACE "/;" ";" VTKMS_DIR_SEARCH2 "${VTKMS_DIR_SEARCH1}")

  # Construct a set of paths relative to the system search path.
  SET(VTKMS_DIR_SEARCH "")
  FOREACH(dir ${VTKMS_DIR_SEARCH2})
    SET(VTKMS_DIR_SEARCH ${VTKMS_DIR_SEARCH}
      ${dir}/../lib/vtkms-5.2
      ${dir}/../lib/vtkms-5.1
      ${dir}/../lib/vtkms-5.0
      ${dir}/../lib/vtkms
      )
  ENDFOREACH(dir)

  # Old scripts may set these directories in the CMakeCache.txt file.
  # They can tell us where to find VTKMSConfig.cmake.
  SET(VTKMS_DIR_SEARCH_LEGACY "")
  IF(VTKMS_BINARY_PATH AND USE_BUILT_VTKMS)
    SET(VTKMS_DIR_SEARCH_LEGACY ${VTKMS_DIR_SEARCH_LEGACY} ${VTKMS_BINARY_PATH})
  ENDIF(VTKMS_BINARY_PATH AND USE_BUILT_VTKMS)
  IF(VTKMS_INSTALL_PATH AND USE_INSTALLED_VTKMS)
    SET(VTKMS_DIR_SEARCH_LEGACY ${VTKMS_DIR_SEARCH_LEGACY}
        ${VTKMS_INSTALL_PATH}/lib/vtkms)
  ENDIF(VTKMS_INSTALL_PATH AND USE_INSTALLED_VTKMS)

  #
  # Look for an installation or build tree.
  #
  FIND_PATH(VTKMS_DIR UseVTKMS.cmake
    # Support legacy cache files.
    ${VTKMS_DIR_SEARCH_LEGACY}

    # Look for an environment variable VTKMS_DIR.
    $ENV{VTKMS_DIR}

    # Look in places relative to the system executable search path.
    ${VTKMS_DIR_SEARCH}

    # Look in standard UNIX install locations.
    /usr/local/lib/vtkms
    /usr/lib/vtkms

    # Read from the CMakeSetup registry entries.  It is likely that
    # VTKMS will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]

    # Help the user find it if we cannot.
    DOC "The ${VTKMS_DIR_DESCRIPTION}"
  )
ENDIF(NOT VTKMS_DIR)

# If VTKMS was found, load the configuration file to get the rest of the
# settings.
IF(VTKMS_DIR)
  # Make sure the VTKMSConfig.cmake file exists in the directory provided.
  IF(EXISTS ${VTKMS_DIR}/VTKMSConfig.cmake)

    # We found VTKMS.  Load the settings.
    SET(VTKMS_FOUND 1)
    INCLUDE(${VTKMS_DIR}/VTKMSConfig.cmake)

  ELSE(EXISTS ${VTKMS_DIR}/VTKMSConfig.cmake)
    IF(EXISTS ${VTKMS_DIR}/UseVTKMS.cmake)
      # We found VTKMS 4.0 (UseVTKMS.cmake exists, but not VTKMSConfig.cmake).
      SET(VTKMS_FOUND 1)
      # Load settings for VTKMS 4.0.
      INCLUDE(UseVTKMSConfig40)
    ELSE(EXISTS ${VTKMS_DIR}/UseVTKMS.cmake)
      # We did not find VTKMS.
      SET(VTKMS_FOUND 0)
    ENDIF(EXISTS ${VTKMS_DIR}/UseVTKMS.cmake)
  ENDIF(EXISTS ${VTKMS_DIR}/VTKMSConfig.cmake)
ELSE(VTKMS_DIR)
  # We did not find VTKMS.
  SET(VTKMS_FOUND 0)
ENDIF(VTKMS_DIR)

#-----------------------------------------------------------------------------
IF(VTKMS_FOUND)
  # Set USE_VTKMS_FILE for backward-compatability.
  SET(USE_VTKMS_FILE ${VTKMS_USE_FILE})
ELSE(VTKMS_FOUND)
  # VTKMS not found, explain to the user how to specify its location.
  IF(VTKMS_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR ${VTKMS_DIR_MESSAGE})
  ELSE(VTKMS_FIND_REQUIRED)
    IF(NOT VTKMS_FIND_QUIETLY)
      MESSAGE(STATUS ${VTKMS_DIR_MESSAGE})
    ENDIF(NOT VTKMS_FIND_QUIETLY)
  ENDIF(VTKMS_FIND_REQUIRED)
ENDIF(VTKMS_FOUND)
