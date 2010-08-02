# Configure the documentation guide with the library/program version number
CONFIGURE_FILE( "${MicroscopeSimulator_SOURCE_DIR}/docs/Configuration.in.tex"
  "${MicroscopeSimulator_BINARY_DIR}/docs/Configuration.tex" @ONLY )

# This is a simple file copy
CONFIGURE_FILE( "${MicroscopeSimulator_SOURCE_DIR}/docs/MicroscopeSimulator-UserGuide.tex"
  "${MicroscopeSimulator_BINARY_DIR}/docs/MicroscopeSimulator-${MicroscopeSimulator_VERSION_STRING}-UserGuide.tex"
  COPYONLY )
