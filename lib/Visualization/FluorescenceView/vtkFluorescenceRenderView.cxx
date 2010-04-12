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


vtkCxxRevisionMacro(vtkFluorescenceRenderView, "$Revision: 1.0 $");
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

  this->ComputeGradients = 1;
}


vtkFluorescenceRenderView::~vtkFluorescenceRenderView() {
  if (this->RenderWindow) {
    this->RenderWindow->Delete();
  }
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
  this->PSFTexture->Update();
  this->PSFGradientTexture->Update();

  this->Renderer->SetMapsToZero(this->FluoroSim->GetMinimumIntensityLevel());
  this->Renderer->SetMapsToOne(this->FluoroSim->GetMaximumIntensityLevel());

  ImageModelObject* comparisonImage = this->Sim->
    GetFluorescenceOptimizer()->GetComparisonImageModelObject();
  if (comparisonImage) {
    this->ExperimentalImageTexture->SetInput(comparisonImage->GetImageData());
    this->GradientRenderer->
      SetExperimentalImageTexture(this->ExperimentalImageTexture);
  } else {
    this->GradientRenderer->SetExperimentalImageTexture(NULL);
  }

  for (int i = 0; i < this->GetNumberOfRepresentations(); ++i) {
    vtkModelObjectFluorescenceRepresentation* rep = 
      vtkModelObjectFluorescenceRepresentation::SafeDownCast(this->GetRepresentation(i));
    if (rep) {
      rep->PrepareForRendering(this);

      vtkFluorescencePolyDataMapper* fluorMapper = 
        vtkFluorescencePolyDataMapper::SafeDownCast(rep->GetActor()->GetMapper());
      if (fluorMapper) {
        fluorMapper->SetFocalPlaneDepth(this->FluoroSim->GetFocalPlaneDepth());
        fluorMapper->SetExposure(this->FluoroSim->GetExposure());
        fluorMapper->SetPixelSize(this->FluoroSim->GetPixelSize(),
                                  this->FluoroSim->GetPixelSize());
        fluorMapper->SetPSFTexture(this->PSFTexture);
      } else {
        vtkErrorMacro(<< "Expected a vtkFluorescencePolyDataMapper");
      }

      if (this->ComputeGradients) {
        vtkFluorescencePointsGradientPolyDataMapper* gradientMapper =
          vtkFluorescencePointsGradientPolyDataMapper::SafeDownCast(rep->GetGradientActor()->GetMapper());
        if (gradientMapper) {
          gradientMapper->SetFocalPlaneDepth(this->FluoroSim->GetFocalPlaneDepth());
          gradientMapper->SetExposure(this->FluoroSim->GetExposure());
          gradientMapper->SetPixelSize(this->FluoroSim->GetPixelSize(),
                                       this->FluoroSim->GetPixelSize());
          gradientMapper->SetPSFTexture(this->PSFGradientTexture);
        }
      }
    }
  }
}


void vtkFluorescenceRenderView::PrintSelf(ostream& os, vtkIndent indent) {
  // TODO - print fluorescence renderer
}
