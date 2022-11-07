CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
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
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
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
    GIT_TAG "v4.0a07"
    CMAKE_ARGS
      -D BUILD_TESTING:BOOL=OFF
      -D BUILD_EXAMPLES:BOOL=OFF
      -D BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -D ITK_BUILD_ALL_MODULES:BOOL=ON
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
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
    GIT_TAG "v5.6.1"
    CMAKE_ARGS
      -D BUILD_TESTING:BOOL=OFF
      -D BUILD_EXAMPLES:BOOL=OFF
      -D BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -D VTK_USE_VIEWS:BOOL=ON
      -D VTK_USE_GUISUPPORT:BOOL=ON
      -D VTK_USE_QT:BOOL=ON
      -D VTK_USE_SYSTEM_ZLIB:BOOL=OFF
      -D VTK_USE_SYSTEM_LIBXML2:BOOL=ON
      -D LIBXML2_INCLUDE_DIR:PATH=/usr/include/libxml2
      -D LIBXML2_LIBRARY:PATH=/usr/lib/libxml2.dylib
      -D CMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
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
  BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -j 4
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
