#ifndef _GL_CHECK_H_
#define _GL_CHECK_H_

class GLCheck {

 public:
  GLCheck();
  virtual ~GLCheck();

  void VerboseOn();
  void VerboseOff();
  void SetVerbose(bool verbose);

  bool TestRequiredExtensions();
  bool Test16BitFloatingPointBlend();
  bool Test32BitFloatingPointBlend();
  bool TestFloatingPointTextureTrilinearInterpolation();
  bool TestGLSLUnsignedInts();

 protected:
  bool m_Verbose;

};


#endif // _GL_CHECK_H_
