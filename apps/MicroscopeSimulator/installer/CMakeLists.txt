IF ( WIN32 )
  INCLUDE( "${MicroscopeSimulator_SOURCE_DIR}/apps/MicroscopeSimulator/installer/Win32/CMake/Win32Package.txt" )
  SET( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/installer/Win32/CMake" ${CMAKE_MODULE_PATH} )
ENDIF ( WIN32 )

IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  INCLUDE( "${MicroscopeSimulator_SOURCE_DIR}/apps/MicroscopeSimulator/installer/MacOSX/CMake/MacOSXPackage.txt" )
ENDIF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )