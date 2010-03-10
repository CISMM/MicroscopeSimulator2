#include <vtkFluorescenceRenderView.h>

#include <FluorescenceSimulation.h>
#include <PointSpreadFunction.h>

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


vtkCxxRevisionMacro(vtkFluorescenceRenderView, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkFluorescenceRenderView);


vtkFluorescenceRenderView::vtkFluorescenceRenderView() {
  vtkSmartPointer<vtkFramebufferObjectTexture> renderTexture =
    vtkSmartPointer<vtkFramebufferObjectTexture>::New();
  renderTexture->AutomaticDimensionsOn();

  this->Renderer = vtkFluorescenceRenderer::New();
  this->Renderer->AddFramebufferTexture(renderTexture);

  this->RenderWindow = vtkRenderWindow::New();
  this->RenderWindow->AddRenderer(this->Renderer);

  this->PSFImage = NULL;

  this->PSFTexture = vtkSmartPointer<vtkOpenGL3DTexture>::New();
}


vtkFluorescenceRenderView::~vtkFluorescenceRenderView() {
  if (this->Renderer) {
    //this->Renderer->Delete(); // This lines causes a crash for unknown reasons
  }

  if (this->RenderWindow) {
    this->RenderWindow->Delete();
  }
}


vtkCxxSetObjectMacro(vtkFluorescenceRenderView, PSFImage, vtkImageData);


void vtkFluorescenceRenderView::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  this->Simulation = simulation;
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
  
  this->PSFTexture->SetInput(this->PSFImage);
  PointSpreadFunction* psf = this->Simulation->GetActivePointSpreadFunction();

  if (psf) {
    this->PSFTexture->SetInput(psf->GetOutput());
  } else {
    this->PSFTexture->SetInput(NULL);
  }
  this->PSFTexture->Update();

  this->Renderer->SetMapsToZero(this->Simulation->GetMinimumIntensityLevel());
  this->Renderer->SetMapsToOne(this->Simulation->GetMaximumIntensityLevel());

  for (int i = 0; i < this->GetNumberOfRepresentations(); ++i) {
    vtkModelObjectFluorescenceRepresentation* rep = 
      vtkModelObjectFluorescenceRepresentation::SafeDownCast(this->GetRepresentation(i));
    if (rep) {
      rep->PrepareForRendering(this);

      vtkActor* actor = rep->GetActor();
      actor->SetTexture(this->PSFTexture);

      vtkFluorescencePolyDataMapper* fluorMapper = vtkFluorescencePolyDataMapper::SafeDownCast(actor->GetMapper());
      if (fluorMapper) {
        fluorMapper->SetFocalPlaneDepth(this->Simulation->GetFocalPlaneDepth());
        fluorMapper->SetExposure(this->Simulation->GetExposure());
        fluorMapper->SetPixelSize(this->Simulation->GetPixelSize(),
                                  this->Simulation->GetPixelSize());
      }
    }
  }
}


void vtkFluorescenceRenderView::PrintSelf(ostream& os, vtkIndent indent) {
  // TODO - print fluorescence renderer
}
