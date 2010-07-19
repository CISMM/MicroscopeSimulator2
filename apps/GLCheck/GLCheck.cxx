#include <GLCheck.h>

#include <cstdio>
#include <string>


GLCheck
::GLCheck() {

}


GLCheck
::~GLCheck() {

}


#define ADD_TEST(testName) { \
    if (argc == 1 || (argc >= 2 && std::string(argv[1]) == std::string(#testName))) { \
      GLCheck checker;                                                  \
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
