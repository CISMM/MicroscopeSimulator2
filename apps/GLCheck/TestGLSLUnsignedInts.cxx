#include <GLCheck.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLExtensionManager.h>
#include <vtkSmartPointer.h>


bool
GLCheck
::TestGLSLUnsignedInts() {
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();

  vtkSmartPointer<vtkOpenGLExtensionManager> extManager =
    vtkSmartPointer<vtkOpenGLExtensionManager>::New();
  extManager->SetRenderWindow(renWin);

  // GL Version 3.0 supports GLSL 1.3, which has unsigned ints
  if (!extManager->ExtensionSupported("GL_VERSION_3_0")) {
    if (m_Verbose) {
      std::cout << "GL_VERSION_3_0 is not supported" << std::endl;
    }
    return false;
  }

  return true;
}
