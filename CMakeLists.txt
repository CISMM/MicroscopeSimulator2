PROJECT (MicroscopeSimulator)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)

# Set version information. Bump this up with every release.
SET (MicroscopeSimulator_MAJOR_NUMBER "2")
SET (MicroscopeSimulator_MINOR_NUMBER "3")
SET (MicroscopeSimulator_REVISION_NUMBER "1")
SET (MicroscopeSimulator_VERSION_STRING "${MicroscopeSimulator_MAJOR_NUMBER}.${MicroscopeSimulator_MINOR_NUMBER}.${MicroscopeSimulator_REVISION_NUMBER}")

SET (EXECUTABLE_OUTPUT_PATH "${MicroscopeSimulator_BINARY_DIR}/bin")
SET (LIBRARY_OUTPUT_PATH "${MicroscopeSimulator_BINARY_DIR}/lib")

OPTION (CMAKE_VERBOSE_MAKEFILE  "Enable/Disable verbose compiler output" ON)
OPTION (CMAKE_COLOR_MAKEFILE "Enable/Disable color cues when building" ON)
MARK_AS_ADVANCED (CLEAR CMAKE_VERBOSE_MAKEFILE CMAKE_COLOR_MAKEFILE)

# Superbuild off by default.
OPTION (USE_SUPERBUILD "Enable superbuild" OFF)
IF (${USE_SUPERBUILD})
  INCLUDE(SuperBuild.cmake)
  RETURN()
ENDIF()

if(WIN32)
  # To avoid warnings in case we're using Visual Studio
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_CRT_NONSTDC_NO_DEPRECATE )
endif(WIN32)

#######################################
# Include ITK
#######################################
FIND_PACKAGE(ITK
  REQUIRED
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_PACKAGE_REGISTRY
  PATHS ${MicroscopeSimulator_BINARY_DIR}/External/src/ITK-build
  )
INCLUDE(${ITK_USE_FILE})

#######################################
# Include VTK
#######################################
FIND_PACKAGE(VTK
  REQUIRED
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_PACKAGE_REGISTRY
  PATHS ${MicroscopeSimulator_BINARY_DIR}/External/src/VTK-build
  )
INCLUDE(${VTK_USE_FILE})

#######################################
# Include CLAPACK
#######################################
FIND_PACKAGE(CLAPACK)
if (CLAPACK_FOUND)
  #INCLUDE( "${CLAPACK_CONFIG}" )
  #MESSAGE(${CLAPACK_SOURCE_DIR})

  # Find the source directory for CLapack
  FILE( READ "${CLAPACK_DIR}/CMakeCache.txt" CLAPACK_CMAKE_CACHE )
  STRING( REGEX REPLACE ".*CLAPACK_SOURCE_DIR:STATIC=([^\n]+).*" "\\1" CLAPACK_SOURCE_DIR ${CLAPACK_CMAKE_CACHE} )
  MESSAGE( ${CLAPACK_SOURCE_DIR} )

  INCLUDE_DIRECTORIES( ${CLAPACK_SOURCE_DIR}/INCLUDE )
  LINK_DIRECTORIES( ${CLAPACK_DIR}/SRC )

ELSE(CLAPACK_FOUND)
  MESSAGE(FATAL_ERROR
  "Cannot build without CLAPACK. Please set CLAPACK_DIR.")
ENDIF(CLAPACK_FOUND)

#######################################
# Need to link against OpenGL
#######################################
FIND_PACKAGE( OpenGL REQUIRED )

#######################################
# Find include files for libxml2
#######################################
# Find the include directory for libxml2
LOAD_CACHE( "${VTK_DIR}" READ_WITH_PREFIX "" LIBXML2_INCLUDE_DIR LIBXML2_LIBRARIES )

INCLUDE_DIRECTORIES( ${LIBXML2_INCLUDE_DIR} )
#TARGET_LINK_LIBRARIES( ${LIBXML2_LIBRARIES} )

#######################################
# Personal filter library and support classes.
#
# Microscope Simulator is split up into several logical groups.
# apps      - a place for various applications related to the main application
#######################################
INCLUDE_DIRECTORIES (
    ${MicroscopeSimulator_SOURCE_DIR}/NanoITK/Code/Algorithms
    ${MicroscopeSimulator_SOURCE_DIR}/NanoITK/Code/BasicFilters
    ${MicroscopeSimulator_SOURCE_DIR}/NanoITK/Code/Common
    ${MicroscopeSimulator_SOURCE_DIR}/lib
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Filters
    ${MicroscopeSimulator_SOURCE_DIR}/lib/IO
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Model
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Model/AFMSim
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Model/FluoroSim
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Model/ModelObjects
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization/GeometrySources
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization/GeometryView
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization/FluorescenceView
    ${MicroscopeSimulator_SOURCE_DIR}/lib/VTK/Common
    ${MicroscopeSimulator_SOURCE_DIR}/lib/VTK/Filtering
    ${MicroscopeSimulator_SOURCE_DIR}/lib/VTK/Graphics
    ${MicroscopeSimulator_SOURCE_DIR}/lib/VTK/Imaging
    ${MicroscopeSimulator_SOURCE_DIR}/lib/VTK/Rendering
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Utilities
    ${MicroscopeSimulator_BINARY_DIR}/lib)

ADD_SUBDIRECTORY(lib)

#######################################
# Applications
#######################################
ADD_SUBDIRECTORY(apps)

INCLUDE(CTest)
ENABLE_TESTING()

#######################################
# Documentation
#######################################
ADD_SUBDIRECTORY(docs)
