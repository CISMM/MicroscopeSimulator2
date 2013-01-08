#include <Common.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLExtensionManager.h>
#include <vtkSmartPointer.h>

#include <iostream>
#include <vector>


bool TestRequiredExtensions(bool verbose) {
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();

  vtkSmartPointer<vtkOpenGLExtensionManager> extManager =
    vtkSmartPointer<vtkOpenGLExtensionManager>::New();
  extManager->SetRenderWindow(renWin);

  std::vector<std::string> extensions;
  extensions.push_back("GL_ARB_imaging");
  extensions.push_back("GL_ARB_multitexture");
  extensions.push_back("GL_ARB_occlusion_query");
  extensions.push_back("GL_ARB_shadow");
  extensions.push_back("GL_ARB_texture_rectangle");
  extensions.push_back("GL_EXT_framebuffer_object");
  extensions.push_back("GL_VERSION_1_2");
  extensions.push_back("GL_VERSION_1_3");
  extensions.push_back("GL_VERSION_1_4");
  extensions.push_back("GL_VERSION_1_5");
  extensions.push_back("GL_VERSION_2_0");

  for (size_t i = 0; i < extensions.size(); i++) {
    if (!extManager->ExtensionSupported(extensions[i].c_str())) {
      if (verbose) {
        std::cout << "OpenGL extension '" << extensions[i].c_str() << "' not supported."
                  << std::endl;
      }
      return false;
    }
  }

  return true;
}
