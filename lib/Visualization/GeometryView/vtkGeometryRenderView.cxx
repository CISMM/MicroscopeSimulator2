#include <vtkGeometryRenderView.h>

#include <vtkImageData.h>
#include <vtkInteractorObserver.h>
#include <vtkFramebufferObjectTexture.h>
#include <vtkFramebufferObjectRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderedRepresentation.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


vtkCxxRevisionMacro(vtkGeometryRenderView, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkGeometryRenderView);


vtkGeometryRenderView::vtkGeometryRenderView() {
  this->AFMTexture = vtkSmartPointer<vtkFramebufferObjectTexture>::New();

  this->AFMRenderer = vtkSmartPointer<vtkFramebufferObjectRenderer>::New();
  this->AFMRenderer->SetFramebufferTexture(this->AFMTexture);

  this->RenderWindow->RemoveRenderer(this->Renderer);

  this->RenderWindow->AddRenderer(this->AFMRenderer);
  this->RenderWindow->AddRenderer(this->Renderer);
}


vtkGeometryRenderView::~vtkGeometryRenderView() {
}


#if 0
vtkImageData* vtkGeometryRenderView::GetImage() {
  this->Renderer->GetFramebufferTexture()->UpdateWholeExtent();
  return this->Renderer->GetFramebufferTexture()->GetOutput();
}


vtkAlgorithmOutput* vtkGeometryRenderView::GetImageOutputPort() {
  this->Renderer->GetFramebufferTexture()->UpdateWholeExtent();
  return this->Renderer->GetFramebufferTexture()->GetOutputPort();
}
#endif


void vtkGeometryRenderView::PrintSelf(ostream& os, vtkIndent indent) {
  // TODO - print fluorescence renderer
}
