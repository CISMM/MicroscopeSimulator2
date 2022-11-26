CMAKE_MINIMUM_REQUIRED(VERSION 3.23.2)
MESSAGE("Running super build")

INCLUDE(ExternalProject)

SET(EXTERNAL_PREFIX ${MicroscopeSimulator_BINARY_DIR}/External)

#---------------------------------------------------------------------------
# CLAPACK
#---------------------------------------------------------------------------
IF(NOT DEFINED CLAPACK_DIR)
  EXTERNALPROJECT_ADD( CLAPACK
    PREFIX ${EXTERNAL_PREFIX}
    URL    http://www.netlib.org/clapack/clapack-3.2.1-CMAKE.tgz
    INSTALL_COMMAND ""
    CMAKE_ARGS
      -D BUILD_TESTING:BOOL=OFF
      -D BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    #BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
    INSTALL_COMMAND ""
    )

  EXTERNALPROJECT_GET_PROPERTY(CLAPACK BINARY_DIR)
  SET(CLAPACK_DIR ${BINARY_DIR})
ENDIF()

#---------------------------------------------------------------------------
# ITK
#---------------------------------------------------------------------------
IF(NOT DEFINED ITK_DIR)
  EXTERNALPROJECT_ADD( ITK
    PREFIX    ${EXTERNAL_PREFIX}
    GIT_REPOSITORY https://github.com/InsightSoftwareConsortium/ITK.git
    # Original code used version 4.0a07 of ITK, but it can't compile with gcc above 4.
    # Version 4.9.1 didn't know about gcc above 6.
    #GIT_TAG "v4.0a07"
    GIT_TAG "v5.2.0"
    GIT_SHALLOW ON
    GIT_SUBMODULES_RECURSE OFF
    GIT_SUBMODULES ""
    CMAKE_ARGS
      -D BUILD_TESTING:BOOL=OFF
      -D BUILD_EXAMPLES:BOOL=OFF
      -D BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -D ITK_BUILD_ALL_MODULES:BOOL=ON
    INSTALL_COMMAND ""
    )

  EXTERNALPROJECT_GET_PROPERTY(ITK BINARY_DIR)
  SET(ITK_DIR ${BINARY_DIR})
ENDIF()

#---------------------------------------------------------------------------
# VTK
#---------------------------------------------------------------------------
IF(NOT DEFINED VTK_DIR)
  EXTERNALPROJECT_ADD( VTK
    PREFIX    ${EXTERNAL_PREFIX}
    GIT_REPOSITORY https://github.com/Kitware/VTK.git
    # Version v5.6.1 does not seem to be able to compile using gcc 7.5
    # Neither did v6.0.0 or v7.0.0
    #GIT_TAG "v5.6.1"
    GIT_TAG "v8.0.0"
    GIT_SHALLOW TRUE
    CMAKE_ARGS
      -D BUILD_TESTING:BOOL=OFF
      -D BUILD_EXAMPLES:BOOL=OFF
      -D BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -D VTK_RENDERING_BACKEND=OpenGL
      -D VTK_USE_VIEWS:BOOL=ON
      -D VTK_USE_GUISUPPORT:BOOL=ON
      -D VTK_Group_Qt:BOOL=ON
      -D VTK_USE_SYSTEM_ZLIB:BOOL=OFF
      -D VTK_USE_SYSTEM_LIBXML2:BOOL=ON
      -D CMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    INSTALL_COMMAND ""
    )

  EXTERNALPROJECT_GET_PROPERTY(VTK BINARY_DIR)
  SET(VTK_DIR ${BINARY_DIR})
ENDIF()

#---------------------------------------------------------------------------
# MicroscopeSimulator2 configuration
#---------------------------------------------------------------------------
EXTERNALPROJECT_ADD( MicroscopeSimulator2-Configure
  DOWNLOAD_COMMAND ""
  CMAKE_ARGS
    -D USE_SUPERBUILD:BOOL=OFF
    -D CLAPACK_DIR:PATH=${CLAPACK_DIR}
    -D VTK_DIR:PATH=${VTK_DIR}
    -D ITK_DIR:PATH=${ITK_DIR}
    -D DESIRED_QT_VERSION:STRING=4
    -D CMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  #BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
  INSTALL_COMMAND ""
  DEPENDS
    CLAPACK
    VTK
    ITK
)

#---------------------------------------------------------------------------
# MicroscopeSimulator2 itself
#---------------------------------------------------------------------------
#EXTERNALPROJECT_ADD( MicroscopeSimulator2-Build
#  DOWNLOAD_COMMAND ""
#  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
#  INSTALL_COMMAND ""
#  DEPENDS
#    "MicroscopeSimulator2-Configure"
#)
