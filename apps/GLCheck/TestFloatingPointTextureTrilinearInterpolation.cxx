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
::TestFloatingPointTextureTrilinearInterpolation() {
  vtkSmartPointer<vtkFramebufferObjectTexture> renTexture =
    vtkSmartPointer<vtkFramebufferObjectTexture>::New();
  renTexture->SetQualityTo16Bit();

  vtkSmartPointer<vtkFramebufferObjectRenderer> renderer =
    vtkSmartPointer<vtkFramebufferObjectRenderer>::New();
  renderer->AddFramebufferTexture(renTexture);
  renderer->SetMapsToOne(2.0);

  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->SetSize(200, 200);
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkOpenGLExtensionManager> extManager =
    vtkSmartPointer<vtkOpenGLExtensionManager>::New();
  extManager->SetRenderWindow(renWin);

  if (!extManager->ExtensionSupported("GL_VERSION_2_0")) {
    return false;
  }
  extManager->LoadExtension("GL_VERSION_2_0");

  vtkSmartPointer<vtkPointSource> points = vtkSmartPointer<vtkPointSource>::New();
  points->SetNumberOfPoints(1);
  points->SetCenter(100.0, 100.0, 0.0);
  points->SetRadius(0.0);

  ////////
  vtkSmartPointer<vtkImageConstantSource> psf =
    vtkSmartPointer<vtkImageConstantSource>::New();
  psf->SetConstant(2.0);
  psf->SetSpacing(50.0,50.0,50.0);
  psf->SetWholeExtent(0,3,0,3,0,3);
  psf->SetOrigin(-75.0, -75.0, -75.0);
  psf->UpdateWholeExtent();

  vtkSmartPointer<vtkImageData> psfImage = vtkSmartPointer<vtkImageData>::New();
  psfImage->DeepCopy(psf->GetOutput());

  for (int z = 0; z < 2; z++) {
    for (int y = 0; y < 2; y++) {
      for (int x = 0; x < 4; x++) {
        psfImage->SetScalarComponentFromFloat(x,y,z,0,0.0);
      }
    }
  }

  vtkSmartPointer<vtkOpenGL3DTexture> psfTexture =
    vtkSmartPointer<vtkOpenGL3DTexture>::New();
  psfTexture->InterpolateOn();
  psfTexture->SetInput(psfImage);

  vtkSmartPointer<vtkGatherFluorescencePolyDataMapper> mapper =
    vtkSmartPointer<vtkGatherFluorescencePolyDataMapper>::New();
  mapper->SetPixelSize(1.0,1.0);
  mapper->SetPointsPerPass(1);
  mapper->SetInputConnection(points->GetOutputPort());
  mapper->SetPSFTexture(psfTexture);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  renderer->AddActor(actor);
  renWin->Render();

  vtkCamera *cam = renderer->GetActiveCamera();
  cam->ParallelProjectionOn();
  cam->SetParallelScale(200.0);
  renderer->ResetCameraClippingRange();

  for (int i = 0; i < 2; i++) {
    renWin->Render();
  }
    
  // Read out FBO texture value.
  vtkSmartPointer<vtkImageData> texOutput = renTexture->GetOutput();
  texOutput->Update();

  // Get the red value at pixel (100,120)
  float textureValue = ((float*) texOutput->GetScalarPointer(100,120,0))[0];

  // The interpolated value should be about 1.0, not 0 or 2
  if ((fabs(textureValue - (float) 0) < 1e-5 ||
       fabs(textureValue - (float) 2) < 1e-5) &&
      textureValue >= 0.0 && textureValue <= 2.0) {
    //std::cout << "FloatingPointTextureTrilinearInterpolation texture value ("
    //          << textureValue << ") appears to not be interpolated" 
    //          << std::endl;
    return false;
  }

  return true;
}
