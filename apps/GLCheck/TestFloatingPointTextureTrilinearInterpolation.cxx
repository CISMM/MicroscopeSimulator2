#include <GLCheck.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFramebufferObjectRenderer.h>
#include <vtkFramebufferObjectTexture.h>
#include <vtkGatherFluorescencePolyDataMapper.h>
#include <vtkImageConstantSource.h>
#include <vtkImageMathematics.h>
#include <vtkOpenGL3DTexture.h>
#include <vtkPointSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLExtensionManager.h>
#include <vtkSmartPointer.h>


#include <FluorescenceSimulation.h>
#include <GaussianPointSpreadFunction.h>
#include <ModelObjectList.h>
#include <PointSetModelObject.h>
#include <PointSpreadFunction.h>
#include <PointSpreadFunctionList.h>
#include <Simulation.h>
#include <Visualization.h>


bool
GLCheck
::TestFloatingPointTextureTrilinearInterpolation() {
  Visualization* vis = new Visualization();
  vis->SetBlendingTo16Bit();
  
  vis->GetFluorescenceRenderWindow()->SetSize(200,200);
  
  Simulation*sim = new Simulation(NULL);
  vis->SetSimulation(sim);

  FluorescenceSimulation* fluoroSim = sim->GetFluorescenceSimulation();
  fluoroSim->SetGain(1.0e7);
  double pixelSize = fluoroSim->GetPixelSize();

  fluoroSim->GetPSFList()->AddGaussianPointSpreadFunction("PSF");
  fluoroSim->SetActivePSFByName("PSF");
  PointSpreadFunction* psf = fluoroSim->GetActivePointSpreadFunction();
  GaussianPointSpreadFunction* gaussianPSF = dynamic_cast<GaussianPointSpreadFunction*>(psf);
  gaussianPSF->SetParameterValue(7, 1000.0); // Std dev in X
  gaussianPSF->SetParameterValue(8, 1000.0); // Std dev in Y
  gaussianPSF->SetParameterValue(9, 1000.0); // Std dev in Z

  psf->SetSummedIntensity(1.0);
  psf->Update(); // Must call this for the above PSF setting to take effect

  // Add a point set object
  sim->AddNewModelObject("PointSetModel");
  ModelObject* mo = sim->GetModelObjectList()->GetModelObjectAtIndex(0);
  mo->GetProperty(PointSetModelObject::NUMBER_OF_POINTS_PROP)->SetIntValue(1);
  mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(0.3*pixelSize);
  mo->Update();

  // Render the scene
  vis->RefreshModelObjectView();
  
  // Read back the pixel value
  vis->FluorescenceViewRender();

  vtkSmartPointer<vtkImageData> initialImage = vis->GenerateFluorescenceImage();

  // Now shift the model object by a sub-pixel amount. If there is no
  // interpolation, there should be no change between the previous image and
  // the next image.
  mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(0.4*pixelSize);
  mo->Update();
  vis->FluorescenceViewRender();

  vtkSmartPointer<vtkImageData> shiftedImage = vis->GenerateFluorescenceImage();
  vtkSmartPointer<vtkImageMathematics> diffFilter = 
    vtkSmartPointer<vtkImageMathematics>::New();
  diffFilter->SetOperationToSubtract();
  diffFilter->SetInput1Data(initialImage);
  diffFilter->SetInput2Data(shiftedImage);
  diffFilter->Update();
  vtkImageData* difference = diffFilter->GetOutput();

  double* range = difference->GetScalarRange();
  
  if (m_Verbose) {
    std::cout << "Image difference range: " << range[0] << ", " << range[1] << std::endl;
  }

  if (range[0] == 0.0 && range[1] == 0.0) {
    return false;
  }

  return true;
}
