#include <GLCheck.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFramebufferObjectRenderer.h>
#include <vtkFramebufferObjectTexture.h>
#include <vtkGatherFluorescencePolyDataMapper.h>
#include <vtkImageConstantSource.h>
#include <vtkOpenGL3DTexture.h>
#include <vtkPointSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLExtensionManager.h>
#include <vtkSmartPointer.h>


bool
GLCheck
::Test16BitFloatingPointBlend() {
  vtkSmartPointer<vtkFramebufferObjectTexture> renTexture =
    vtkSmartPointer<vtkFramebufferObjectTexture>::New();
  renTexture->SetQualityTo16Bit();

  vtkSmartPointer<vtkFramebufferObjectRenderer> renderer =
    vtkSmartPointer<vtkFramebufferObjectRenderer>::New();
  renderer->AddFramebufferTexture(renTexture);
  renderer->SetMapsToOne(2048.0);

  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->SetSize(20, 20);
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkOpenGLExtensionManager> extManager =
    vtkSmartPointer<vtkOpenGLExtensionManager>::New();
  extManager->SetRenderWindow(renWin);

  if (!extManager->ExtensionSupported("GL_VERSION_2_0")) {
    return false;
  }
  extManager->LoadExtension("GL_VERSION_2_0");

  vtkSmartPointer<vtkPointSource> points = vtkSmartPointer<vtkPointSource>::New();
  points->SetNumberOfPoints(2000);
  points->SetCenter(0.0, 0.0, 0.0);
  points->SetRadius(0.0);

  vtkSmartPointer<vtkImageConstantSource> psf =
    vtkSmartPointer<vtkImageConstantSource>::New();
  psf->SetConstant(1.0);
  psf->SetSpacing(1.0,1.0,1.0);
  psf->SetWholeExtent(0,3,0,3,0,3);
  psf->SetOrigin(-1.5, -1.5, -1.5);
  psf->UpdateWholeExtent();

  vtkSmartPointer<vtkOpenGL3DTexture> psfTexture =
    vtkSmartPointer<vtkOpenGL3DTexture>::New();
  psfTexture->InterpolateOn();
  psfTexture->SetInputConnection(psf->GetOutputPort());

  vtkSmartPointer<vtkGatherFluorescencePolyDataMapper> mapper =
    vtkSmartPointer<vtkGatherFluorescencePolyDataMapper>::New();
  mapper->SetPixelSize(1.0/20.0, 1/20.0);
  mapper->SetInputConnection(points->GetOutputPort());
  mapper->SetPSFTexture(psfTexture);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  renderer->AddActor(actor);
  renWin->Render();

  vtkCamera *cam = renderer->GetActiveCamera();
  cam->ParallelProjectionOn();
  cam->SetParallelScale(20.0);
  renderer->ResetCameraClippingRange();

  for (int i = 2000; i <= 2048; i++) {
    points->SetNumberOfPoints(i);
    renWin->Render();

    // Read out FBO texture value.
    vtkSmartPointer<vtkImageData> texOutput = renTexture->GetOutput();
    texOutput->Update();

    // Get the red value at pixel (10,10)
    float textureValue = ((float*) texOutput->GetScalarPointer(10,10,0))[0];

    if (fabs(textureValue - (float) i) > 1e-5) {
      //std::cout << "16BitFloatingPointBlend texture value (" << textureValue <<
      //  ") is different from the expected value (" << i << ")" << std::endl;
      return false;
    }
  }

  return true;
}
