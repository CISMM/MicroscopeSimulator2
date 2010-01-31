ADD_SUBDIRECTORY(Filters)
ADD_SUBDIRECTORY(IO)
ADD_SUBDIRECTORY(Model)
ADD_SUBDIRECTORY(Visualization)

CONFIGURE_FILE( "${MicroscopeSimulator_SOURCE_DIR}/lib/Version.h.in"
  "${MicroscopeSimulator_BINARY_DIR}/lib/Version.h" )
  
