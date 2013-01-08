#include <string>

bool ShouldTestBeRun( const char * testName, int argc, char* argv[] ) {
  if ((argc == 1) ||
      (argc == 2 && (strcmp(argv[1],"-v") == 0 || strcmp(argv[1],"--verbose") == 0)) ||
      (argc >= 2 && std::string(argv[argc-1]) == std::string(testName))) {
    bool verbose = false; 
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[1],"-v") == 0 || strcmp(argv[1],"--verbose") == 0)
        verbose = true;
    }
    
    return true;
  }
}