#include <GLCheck.h>

#include <cstdio>
#include <string>


GLCheck
::GLCheck() {

}


GLCheck
::~GLCheck() {

}


bool
GLCheck
::Test16BitFloatingPointBlend() {
  return true;
}


bool
GLCheck
::Test32BitFloatingPointBlend() {
  return true;
}


bool
GLCheck
::TestFloatingPointTextureTrilinearInterpolation() {
  return true;
}


bool
GLCheck
::TestGLSLUnsignedInts() {
  return false;
}


#define RUN_TEST(testName) { \
  GLCheck checker;           \
  RunTest(checker.Test##testName(), #testName);  \
  }                          \


void RunTest(bool passed, const std::string& name) {
  printf("%s: %s\n", passed ? "PASSED" : "FAILED", name.c_str());
}


int main(int argc, char* argv[]) {
  
  RUN_TEST(16BitFloatingPointBlend);
  RUN_TEST(32BitFloatingPointBlend);
  RUN_TEST(FloatingPointTextureTrilinearInterpolation);
  RUN_TEST(GLSLUnsignedInts);

  return 0;
}
