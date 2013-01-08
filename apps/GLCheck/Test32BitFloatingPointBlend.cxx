#include <Common.h>

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


#include <FluorescenceSimulation.h>
#include <GaussianPointSpreadFunction.h>
#include <ModelObjectList.h>
#include <PointSetModelObject.h>
#include <PointSpreadFunction.h>
#include <PointSpreadFunctionList.h>
#include <Simulation.h>
#include <Visualization.h>


bool Test32BitFloatingPointBlend(bool verbose) {
  Visualization* vis = new Visualization();
  vis->SetBlendingTo32Bit();
  
  vis->GetFluorescenceRenderWindow()->SetSize(200,200);
  
  Simulation*sim = new Simulation(NULL);
  vis->SetSimulation(sim);

  FluorescenceSimulation* fluoroSim = sim->GetFluorescenceSimulation();
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
  mo->GetProperty(PointSetModelObject::NUMBER_OF_POINTS_PROP)->SetIntValue(2048);
  mo->Update();

  // Render the scene
  vis->RefreshModelObjectView();
  
  // Read back the pixel value
  double range[2];
  vis->FluorescenceViewRender();
  vis->Get2DFluorescenceImageScalarRange(range);

  // Set the contrast levels
  fluoroSim->SetMinimumIntensityLevel(range[0]);
  fluoroSim->SetMaximumIntensityLevel(range[1]);

  // Set the gain to produce an image with max intensity 2048
  double gain2048 = 2048.0 / range[1];
  fluoroSim->SetGain(gain2048);

  // Now add another model object with one point. This should add an intensity
  // of 1 to the maximum intensity value in the image. Because results from
  // different model objects are blended together, this tests the precision
  // of floating-point blending.
  sim->AddNewModelObject("PointSetModel");
  mo = sim->GetModelObjectList()->GetModelObjectAtIndex(1);
  mo->GetProperty(PointSetModelObject::NUMBER_OF_POINTS_PROP)->SetIntValue(1);
  mo->Update();

  vis->RefreshModelObjectView();

  vis->FluorescenceViewRender();
  vis->Get2DFluorescenceImageScalarRange(range);
  if (verbose) {
    std::cout << "Image intensity range: " << range[0] << ", " << range[1] << std::endl;
  }

  if (fabs(range[1] - 2049.0) > 1e-3) {
    return false;
  }

  return true;
}
