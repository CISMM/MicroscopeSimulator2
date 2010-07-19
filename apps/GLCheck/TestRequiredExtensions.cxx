#include <GLCheck.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLExtensionManager.h>
#include <vtkSmartPointer.h>


bool
GLCheck
::TestRequiredExtensions() {
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();

  vtkSmartPointer<vtkOpenGLExtensionManager> extManager =
    vtkSmartPointer<vtkOpenGLExtensionManager>::New();
  extManager->SetRenderWindow(renWin);

  if (!extManager->ExtensionSupported("GL_ARB_imaging") ||
      !extManager->ExtensionSupported("GL_ARB_multitexture") ||
      !extManager->ExtensionSupported("GL_ARB_occlusion_query") ||
      !extManager->ExtensionSupported("GL_ARB_shadow") ||
      !extManager->ExtensionSupported("GL_ARB_texture_rectangle") ||
      !extManager->ExtensionSupported("GL_EXT_framebuffer_object") ||
      !extManager->ExtensionSupported("GL_VERSION_1_2") ||
      !extManager->ExtensionSupported("GL_VERSION_1_3") ||
      !extManager->ExtensionSupported("GL_VERSION_1_4") ||
      !extManager->ExtensionSupported("GL_VERSION_1_5") ||
      !extManager->ExtensionSupported("GL_VERSION_2_0")
      ) {
    return false;
  }

  return true;
}
