#ifndef _GL_CHECK_H_
#define _GL_CHECK_H_

class GLCheck {

 public:
  GLCheck();
  virtual ~GLCheck();

  bool Test16BitFloatingPointBlend();
  bool Test32BitFloatingPointBlend();
  bool TestFloatingPointTextureTrilinearInterpolation();
  bool TestGLSLUnsignedInts();

 protected:
  

};


#endif // _GL_CHECK_H_
