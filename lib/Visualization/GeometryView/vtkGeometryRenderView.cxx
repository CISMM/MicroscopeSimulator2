#include <vtkGeometryRenderView.h>

#include <AFMSimulation.h>
#include <Simulation.h>

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCaptionActor2D.h>
#include <vtkImageData.h>
#include <vtkInteractorObserver.h>
#include <vtkFloatArray.h>
#include <vtkFramebufferObjectTexture.h>
#include <vtkFramebufferObjectRenderer.h>
#include <vtkImageNoiseSource.h>
#include <vtkObjectFactory.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderedRepresentation.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTexture.h>
#include <vtkTextProperty.h>

#include <vtkSphereSource.h>
#include <vtkPNGWriter.h>
#include <vtkImageShiftScale.h>

vtkStandardNewMacro(vtkGeometryRenderView);


vtkGeometryRenderView::vtkGeometryRenderView() {
  this->AFMTexture = vtkSmartPointer<vtkFramebufferObjectTexture>::New();
  this->AFMTexture->AutomaticDimensionsOff();
  this->AFMTexture->SetTextureWidth(200.0);
  this->AFMTexture->SetTextureHeight(200.0);

  this->AFMRenderer = vtkSmartPointer<vtkFramebufferObjectRenderer>::New();
  this->AFMRenderer->SetFramebufferTexture(this->AFMTexture);
  this->AFMRenderer->SetBackground(0.0, 0.0, 1.0);

  this->RenderWindow->RemoveRenderer(this->Renderer);

  this->RenderWindow->AddRenderer(this->AFMRenderer);
  this->RenderWindow->AddRenderer(this->Renderer);

  this->AFMPlaneSource = vtkSmartPointer<vtkPlaneSource>::New();

  this->AFMPlaneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->AFMPlaneMapper->SetInputConnection(this->AFMPlaneSource->GetOutputPort());

  this->AFMPlaneActor = vtkSmartPointer<vtkActor>::New();
  this->AFMPlaneActor->PickableOff();

  this->AFMPlaneActor->GetProperty()->SetTexture(0, this->AFMTexture);
  this->AFMPlaneActor->SetMapper(this->AFMPlaneMapper);

  //this->Renderer->AddViewProp(this->AFMPlaneActor);

  //this->AFMRenderer->AddViewProp(actor);

  vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
  axes->SetShaftTypeToCylinder();
  axes->SetXAxisLabelText("x");
  axes->SetYAxisLabelText("y");
  axes->SetZAxisLabelText("z");
  axes->SetTotalLength(2.0, 2.0, 2.0);

  vtkSmartPointer<vtkTextProperty> prop1 = vtkSmartPointer<vtkTextProperty>::New();
  prop1->ItalicOn();
  prop1->ShadowOn();
  prop1->SetFontSize(1);
  axes->GetXAxisCaptionActor2D()->SetCaptionTextProperty(prop1);
  vtkSmartPointer<vtkTextProperty> prop2 = vtkSmartPointer<vtkTextProperty>::New();
  prop2->ShallowCopy(prop1);
  axes->GetYAxisCaptionActor2D()->SetCaptionTextProperty(prop2);
  vtkSmartPointer<vtkTextProperty> prop3 = vtkSmartPointer<vtkTextProperty>::New();
  prop3->ShallowCopy(prop1);
  axes->GetZAxisCaptionActor2D()->SetCaptionTextProperty(prop3);

  this->OrientationWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->OrientationWidget->SetOutlineColor(1.0, 1.0, 0.0);
  this->OrientationWidget->SetOrientationMarker(axes);
  this->OrientationWidget->SetViewport(0, 0, 0.15, 0.15);
}


vtkGeometryRenderView::~vtkGeometryRenderView() {
  // If you leave this to ~vtkView, downcasting the vtkView to a vtkRenderView
  // will fail for some reason, so do it here.
  this->RemoveAllRepresentations();
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


void vtkGeometryRenderView::SetSimulation(Simulation* simulation) {
  this->Sim    = simulation;
  this->AFMSim = simulation->GetAFMSimulation();
}


Simulation* vtkGeometryRenderView::GetSimulation() {
  return this->Sim;
}


void vtkGeometryRenderView::SetShowOrientationWidget(bool show) {
  this->OrientationWidget->SetEnabled(show ? 1 : 0);
}


void vtkGeometryRenderView::PrepareForRendering() {
  this->Superclass::PrepareForRendering();

  if (this->Renderer->GetRenderWindow()) {
    this->OrientationWidget->
      SetInteractor(this->Renderer->GetRenderWindow()->GetInteractor());
  }

  // WARNING: The AFM simulation implementation is incomplete
  double        pixelSize   = this->AFMSim->GetPixelSize();
  unsigned int imageWidth   = this->AFMSim->GetImageWidth();
  unsigned int imageHeight  = this->AFMSim->GetImageHeight();
  double       dImageWidth  = static_cast<double>(imageWidth);
  double       dImageHeight = static_cast<double>(imageHeight);

  this->AFMTexture->SetTextureWidth(imageWidth);
  this->AFMTexture->SetTextureHeight(imageHeight);

  this->AFMRenderer->ResetCameraClippingRange();

#if 0
  this->AFMTexture->UpdateWholeExtent();
  std::cout << "num comps: " << this->AFMTexture->GetOutput()->GetNumberOfScalarComponents() << std::endl;
  if (this->AFMTexture->GetOutput()->GetNumberOfPoints() >= 10) {
    for (int i = 0; i < 10; i++) {
      std::cout << AFMTexture->GetOutput()->GetScalarComponentAsFloat(i, 0, 0, 0) <<
        ", " << AFMTexture->GetOutput()->GetScalarComponentAsFloat(i, 0, 0, 1) <<
        ", " << AFMTexture->GetOutput()->GetScalarComponentAsFloat(i, 0, 0, 2) << std::endl;
    }
  }
#endif



  this->AFMPlaneSource->SetXResolution(imageWidth);
  this->AFMPlaneSource->SetYResolution(imageHeight);
  this->AFMPlaneSource->SetOrigin(0.5 * pixelSize, 0.5 * pixelSize, 0.0);
  this->AFMPlaneSource->SetPoint1(pixelSize * (dImageWidth-0.5), 0.5 * pixelSize, 0.0);
  this->AFMPlaneSource->SetPoint2(0.5 * pixelSize, pixelSize * (dImageHeight-0.5), 0.0);
  this->AFMPlaneSource->Update();

#if 0
  // Re-write texture coordinates
  vtkPolyData* newSurface = vtkPolyData::New();
  newSurface->DeepCopy(this->AFMPlaneSource->GetOutput());


  vtkFloatArray* newTCoords = vtkFloatArray::New();
  newTCoords->SetNumberOfComponents(2);
  newTCoords->Allocate(2*newSurface->GetPoints()->GetNumberOfPoints());

  double tc[2];
  for (int numPts = 0, i = 0; i < (this->AFMPlaneSource->GetYResolution()+1); i++) {
    tc[1] = static_cast<double>(i);
    for (int j = 0; j < (this->AFMPlaneSource->GetXResolution()+1); j++) {
      tc[0] = static_cast<double>(j);
      newTCoords->InsertTuple(numPts++, tc);
    }
  }
  newTCoords->SetName("TextureCoordinates");
  newSurface->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();

  this->AFMPlaneMapper->SetInput(newSurface);
#endif
}
