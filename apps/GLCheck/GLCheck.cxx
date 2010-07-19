#include <GLCheck.h>

#include <cstdio>
#include <string>


GLCheck
::GLCheck() {

}


GLCheck
::~GLCheck() {

}


#define RUN_TEST(testName) { \
  GLCheck checker;           \
  RunTest(checker.Test##testName(), #testName);  \
  }                          \


void RunTest(bool passed, const std::string& name) {
  printf("%s: %s\n", passed ? "PASSED" : "FAILED", name.c_str());
}


int main(int argc, char* argv[]) {

  RUN_TEST(RequiredExtensions);  
  RUN_TEST(16BitFloatingPointBlend);
  RUN_TEST(32BitFloatingPointBlend);
  RUN_TEST(FloatingPointTextureTrilinearInterpolation);
  RUN_TEST(GLSLUnsignedInts);

  return 0;
}
