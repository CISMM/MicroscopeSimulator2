#include <GLCheck.h>

#include <cstdio>
#include <string>


GLCheck
::GLCheck() {
  m_Verbose = false;
}


GLCheck
::~GLCheck() {

}


void
GLCheck
::VerboseOn() {
  m_Verbose = true;
}


void
GLCheck
::SetVerbose(bool verbose) {
  m_Verbose = verbose;
}


void
GLCheck
::VerboseOff() {
  m_Verbose = false;
}


#define ADD_TEST(testName) { \
    if ((argc == 1) ||                                                  \
        (argc == 2 && (strcmp(argv[1],"-v") == 0 || strcmp(argv[1],"--verbose") == 0)) || \
        (argc >= 2 && std::string(argv[argc-1]) == std::string(#testName))) { \
      bool verbose = false;                                             \
      for (int i = 1; i < argc; i++) {                                  \
        if (strcmp(argv[1],"-v") == 0 || strcmp(argv[1],"--verbose") == 0) \
          verbose = true;                                               \
      }                                                                 \
      GLCheck checker;                                                  \
      checker.SetVerbose(verbose);                                      \
      RunTest(checker.Test##testName(), #testName);                     \
    }                                                                   \
  }                                                                     
    

void RunTest(bool passed, const std::string& name) {
  printf("%s %s\n", name.c_str(), passed ? "PASSED" : "FAILED");
}


int main(int argc, char* argv[]) {

  ADD_TEST(RequiredExtensions);
  ADD_TEST(16BitFloatingPointBlend);
  ADD_TEST(32BitFloatingPointBlend);
  ADD_TEST(FloatingPointTextureTrilinearInterpolation);
  ADD_TEST(GLSLUnsignedInts);

  return 0;
}
