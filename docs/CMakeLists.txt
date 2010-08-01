# Configure the documentation guide with the library/program version number
CONFIGURE_FILE( "${MicroscopeSimulator_SOURCE_DIR}/docs/Configuration.in.tex"
  "${MicroscopeSimulator_BINARY_DIR}/docs/Configuration.tex" )

# Change this to a simple file copy
CONFIGURE_FILE( "${MicroscopeSimulator_SOURCE_DIR}/docs/MicroscopeSimulator.tex"
  "${MicroscopeSimulator_BINARY_DIR}/docs/MicroscopeSimulator.tex" )
