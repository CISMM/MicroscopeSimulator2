#include <vtkFluorescenceRenderView.h>

#include <FluorescenceOptimizer.h>
#include <FluorescenceSimulation.h>
#include <ImageModelObject.h>
#include <PointSpreadFunction.h>
#include <Simulation.h>

#include <vtkImageData.h>
#include <vtkFluorescenceRenderer.h>
#include <vtkFramebufferObjectTexture.h>
#include <vtkModelObjectFluorescenceRepresentation.h>
#include <vtkFramebufferObjectTexture.h>
#include <vtkHardwareSelector.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGL3DTexture.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <vtkFluorescencePolyDataMapper.h>
#include <vtkGatherFluorescencePolyDataMapper.h>
#include <vtkBlendingFluorescencePolyDataMapper.h>

#include <vtkFluorescencePointsGradientRenderer.h>
#include <vtkFluorescencePointsGradientPolyDataMapper.h>

vtkStandardNewMacro(vtkFluorescenceRenderView);

vtkFluorescenceRenderView::vtkFluorescenceRenderView() {
  this->SyntheticImageTexture = vtkFramebufferObjectTexture::New();

  this->SyntheticImageTexture->AutomaticDimensionsOn();

  this->Renderer = vtkFluorescenceRenderer::New();
  this->Renderer->AddFramebufferTexture(this->SyntheticImageTexture);

  this->GradientRenderer = vtkFluorescencePointsGradientRenderer::New();
  this->GradientRenderer->AddFramebufferTexture(this->SyntheticImageTexture);

  this->RenderWindow = vtkRenderWindow::New();
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->AddRenderer(this->GradientRenderer);

  this->PSFTexture = vtkSmartPointer<vtkOpenGL3DTexture>::New();
  this->PSFTexture->InterpolateOn();
  this->PSFTexture->RepeatOff();

  this->PSFGradientTexture = vtkSmartPointer<vtkOpenGL3DTexture>::New();
  this->PSFGradientTexture->InterpolateOn();
  this->PSFGradientTexture->RepeatOff();

  this->ExperimentalImageTexture = vtkSmartPointer<vtkOpenGL3DTexture>::New();
  this->ExperimentalImageTexture->InterpolateOn();
  this->ExperimentalImageTexture->RepeatOff();

  this->ComputePointGradients = 0;
}


vtkFluorescenceRenderView::~vtkFluorescenceRenderView() {
  if (this->RenderWindow) {
    this->RenderWindow->Delete();
  }
}


void vtkFluorescenceRenderView::SetBlendingTo16Bit() {
  this->SyntheticImageTexture->SetQualityTo16Bit();
}


void vtkFluorescenceRenderView::SetBlendingTo32Bit() {
  this->SyntheticImageTexture->SetQualityTo32Bit();
}


void vtkFluorescenceRenderView::SetSimulation(Simulation* simulation) {
  this->Sim = simulation;
  this->FluoroSim = simulation->GetFluorescenceSimulation();
}


void vtkFluorescenceRenderView::Render() {
  this->Update();
  this->PrepareForRendering();
  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->RenderWindow->Render();
  }


void vtkFluorescenceRenderView::ResetCamera() {
  this->Update();
  this->PrepareForRendering();
  this->Renderer->ResetCamera();
  this->RenderWindow->Render();
}


void vtkFluorescenceRenderView::ResetCameraClippingRange() {
  this->Update();
  this->PrepareForRendering();
  this->Renderer->ResetCameraClippingRange();
}


vtkImageData* vtkFluorescenceRenderView::GetImage() {
  this->Renderer->GetFramebufferTexture()->UpdateWholeExtent();
  return this->Renderer->GetFramebufferTexture()->GetOutput();
}


vtkAlgorithmOutput* vtkFluorescenceRenderView::GetImageOutputPort() {
  this->Renderer->GetFramebufferTexture()->UpdateWholeExtent();
  return this->Renderer->GetFramebufferTexture()->GetOutputPort();
}


void vtkFluorescenceRenderView::PrepareForRendering() {
  this->Update();
  
  PointSpreadFunction* psf = this->FluoroSim->GetActivePointSpreadFunction();

  if (psf) {
    this->PSFTexture->SetInputConnection(psf->GetOutputPort());
    this->PSFGradientTexture->SetInputConnection(psf->GetGradientOutputPort());
  } else {
    this->PSFTexture->SetInputConnection(NULL);
    this->PSFGradientTexture->SetInputConnection(NULL);
  }

  this->Renderer->SetMapsToZero(this->FluoroSim->GetMinimumIntensityLevel());
  this->Renderer->SetMapsToOne(this->FluoroSim->GetMaximumIntensityLevel());
  this->Renderer->SetBackgroundIntensity(this->FluoroSim->GetOffset());
  if (this->FluoroSim->GetAddGaussianNoise()) {
    this->Renderer->GenerateNoiseOn();
    this->Renderer->SetNoiseStdDev(this->FluoroSim->GetNoiseStdDev());
  } else {
    this->Renderer->GenerateNoiseOff();
  }
  

  ImageModelObject* comparisonImage = this->Sim->
    GetFluorescenceOptimizer()->GetComparisonImageModelObject();
  if (comparisonImage) {
    this->ExperimentalImageTexture->SetInputData(comparisonImage->GetImageData());
    this->GradientRenderer->
      SetExperimentalImageTexture(this->ExperimentalImageTexture);
    this->GradientRenderer->SetBackgroundIntensity(this->FluoroSim->GetOffset());
  } else {
    this->GradientRenderer->SetExperimentalImageTexture(NULL);
  }

  if (this->ComputePointGradients) {
    if (!this->RenderWindow->HasRenderer(this->GradientRenderer))
      this->RenderWindow->AddRenderer(this->GradientRenderer);
  } else {
    if (this->RenderWindow->HasRenderer(this->GradientRenderer))
      this->RenderWindow->RemoveRenderer(this->GradientRenderer);
  }

  for (int i = 0; i < this->GetNumberOfRepresentations(); ++i) {
    vtkModelObjectFluorescenceRepresentation* rep = 
      vtkModelObjectFluorescenceRepresentation::SafeDownCast(this->GetRepresentation(i));
    if (rep) {
      rep->PrepareForRendering(this);

      vtkFluorescencePolyDataMapper* fluorMapper = 
        vtkFluorescencePolyDataMapper::SafeDownCast(rep->GetActor()->GetMapper());
      if (fluorMapper) {
        fluorMapper->SetFocalPlaneDepth(this->FluoroSim->GetFocalPlanePosition());
        fluorMapper->SetGain(this->FluoroSim->GetGain());
        fluorMapper->SetPixelSize(this->FluoroSim->GetPixelSize(),
                                  this->FluoroSim->GetPixelSize());
        fluorMapper->SetShear(this->FluoroSim->GetShearInX(),
                              this->FluoroSim->GetShearInY());
        fluorMapper->SetPSFTexture(this->PSFTexture);
      } else {
        vtkErrorMacro(<< "Expected a vtkFluorescencePolyDataMapper");
      }

      if (this->ComputePointGradients) {
        vtkFluorescencePointsGradientPolyDataMapper* gradientMapper =
          vtkFluorescencePointsGradientPolyDataMapper::SafeDownCast(rep->GetGradientActor()->GetMapper());
        if (gradientMapper) {
          gradientMapper->SetFocalPlaneDepth(this->FluoroSim->GetFocalPlanePosition());
          gradientMapper->SetGain(this->FluoroSim->GetGain());
          gradientMapper->SetPixelSize(this->FluoroSim->GetPixelSize(),
                                       this->FluoroSim->GetPixelSize());
          gradientMapper->SetShear(this->FluoroSim->GetShearInX(),
                                   this->FluoroSim->GetShearInY());
          gradientMapper->SetPSFTexture(this->PSFGradientTexture);
          gradientMapper->SetClearTextureTarget(this->ClearPointsGradientBuffers);
        }
      }
    }
  }
}


void vtkFluorescenceRenderView::PrintSelf(ostream& os, vtkIndent indent) {
  // TODO - print fluorescence renderer
}
