PROJECT (MicroscopeSimulator)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# Set version information. Bump this up with every release.
SET (MicroscopeSimulator_MAJOR_NUMBER "2")
SET (MicroscopeSimulator_MINOR_NUMBER "0")
SET (MicroscopeSimulator_REVISION_NUMBER "0")
SET (MicroscopeSimulator_VERSION_STRING "${MicroscopeSimulator_MAJOR_NUMBER}.${MicroscopeSimulator_MINOR_NUMBER}.${MicroscopeSimulator_REVISION_NUMBER}")

SET (EXECUTABLE_OUTPUT_PATH "${MicroscopeSimulator_BINARY_DIR}/bin")
SET (LIBRARY_OUTPUT_PATH "${MicroscopeSimulator_BINARY_DIR}/lib")

OPTION (CMAKE_VERBOSE_MAKEFILE  "Enable/Disable verbose compiler output" ON)
OPTION (CMAKE_COLOR_MAKEFILE "Enable/Disable color cues when building" ON)
MARK_AS_ADVANCED (CLEAR CMAKE_VERBOSE_MAKEFILE CMAKE_COLOR_MAKEFILE)

#######################################
# Include ITK
#######################################

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR
     "Cannot build without ITK.  Please set ITK_DIR.")
ENDIF(ITK_FOUND)

#######################################
# Include VTK
#######################################

FIND_PACKAGE(VTK)
IF (VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ELSE(VTK_FOUND)
  MESSAGE(FATAL_ERROR
     "Cannot build without VTK.  Please set VTK_DIR.")
ENDIF (VTK_FOUND)

#######################################
# Include VTKMS
#######################################

FIND_PACKAGE(VTKMS)
IF (VTKMS_FOUND)
  INCLUDE(${VTKMS_USE_FILE})
ELSE(VTKMS_FOUND)
  MESSAGE(FATAL_ERROR
    "Cannot build without VTKMS. Please set VTKMS_DIR.")
ENDIF (VTKMS_FOUND)

#######################################
# Personal filter library and support classes.
#
# FibrinAnalysis is split up into several logical groups.
# apps      - a place for various applications related to the main application
#######################################
INCLUDE_DIRECTORIES (
    ${MicroscopeSimulator_SOURCE_DIR}/lib
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Filters
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Filters/itk
    ${MicroscopeSimulator_SOURCE_DIR}/lib/IO
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Model
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Model/FluoroSim
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Model/ModelObjects
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization/GeometrySources
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization/GeometryView
    ${MicroscopeSimulator_SOURCE_DIR}/lib/Visualization/FluorescenceView
    ${MicroscopeSimulator_BINARY_DIR}/lib
    ${VTK_KITS_DIR}/vtklibxml2/)

ADD_SUBDIRECTORY(lib)

#######################################
# Applications
#######################################
ADD_SUBDIRECTORY(apps)


#######################################
# Testing components
#######################################
#OPTION (BUILD_IT_TESTS "Include MicroscopeSimulator unit tests" ON)
#IF (BUILD_IT_TESTS)
#    SUBDIRS(TestSuite Tests)
#ENDIF (BUILD_IT_TESTS)
