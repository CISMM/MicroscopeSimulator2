#include <cstdio>
#include <string>

#include "Common.h"

// Headers for individual tests
#include "TestRequiredExtensions.h"
#include "Test16BitFloatingPointBlend.h"
#include "Test32BitFloatingPointBlend.h"
#include "TestFloatingPointTextureTrilinearInterpolation.h"
#include "TestGLSLUnsignedInts.h"


void RunTest(bool passed, const std::string& name) {
  printf("%s %s\n", name.c_str(), passed ? "PASSED" : "FAILED");
  fflush(stdout);
}


int main(int argc, char* argv[]) {
  // Verbose mode?
  bool verbose = false;
  if ( argc >= 2 &&
       ( argv[1] == std::string("-v") || argv[1] == std::string("--verbose") ) ) {
    verbose = true;
  }

  // Process tests
  if ( ShouldTestBeRun( "RequiredExtensions", argc, argv ) ) {
    RunTest( TestRequiredExtensions( verbose ),
             "RequiredExtensions" );
  }
  if ( ShouldTestBeRun( "16BitFloatingPointBlend", argc, argv ) ) {
    RunTest( Test16BitFloatingPointBlend( verbose ),
             "16BitFloatingPointBlend" );
  }
  if ( ShouldTestBeRun( "32BitFloatingPointBlend", argc, argv ) ) {
    RunTest( Test32BitFloatingPointBlend( verbose ),
             "32BitFloatingPointBlend" );
  }
  if ( ShouldTestBeRun( "FloatingPointTextureTrilinearInterpolation", argc, argv ) ) {
    RunTest( TestFloatingPointTextureTrilinearInterpolation( verbose ),
             "FloatingPointTextureTrilinearInterpolation" );
  }
  if ( ShouldTestBeRun( "GLSLUnsignedInts", argc, argv ) ) {
    RunTest( TestGLSLUnsignedInts( verbose ),
             "GLSLUnsignedInts" );
  }

  return 0;
}
