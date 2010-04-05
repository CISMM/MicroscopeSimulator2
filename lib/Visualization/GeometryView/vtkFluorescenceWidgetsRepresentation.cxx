#include <vtkFluorescenceWidgetsRepresentation.h>

#include <vtkActor.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkReferenceGridSource.h>
#include <vtkRenderer.h>
#include <vtkRenderView.h>


vtkCxxRevisionMacro(vtkFluorescenceWidgetsRepresentation, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkFluorescenceWidgetsRepresentation);


vtkFluorescenceWidgetsRepresentation
::vtkFluorescenceWidgetsRepresentation() {
  this->Simulation = NULL;

  vtkSmartPointer<vtkProperty> focalPlaneProperty = vtkSmartPointer<vtkProperty>::New();
  focalPlaneProperty->SetRepresentationToWireframe();
  focalPlaneProperty->SetColor(1.0, 1.0, 1.0);
  focalPlaneProperty->LightingOff();

  this->FocalPlaneGrid = vtkSmartPointer<vtkReferenceGridSource>::New();
  this->FocalPlaneGrid->SetOrigin(0.0, 0.0, 0.0);

  this->FocalPlaneGridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->FocalPlaneGridMapper->SetInputConnection(this->FocalPlaneGrid->GetOutputPort());
  
  this->FocalPlaneGridActor = vtkSmartPointer<vtkActor>::New();
  this->FocalPlaneGridActor->SetMapper(this->FocalPlaneGridMapper);
  this->FocalPlaneGridActor->PickableOff();
  this->FocalPlaneGridActor->SetProperty(focalPlaneProperty);

  this->ImageVolumeOutlineSource = vtkSmartPointer<vtkOutlineSource>::New();

  this->ImageVolumeOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ImageVolumeOutlineMapper->SetInputConnection(this->ImageVolumeOutlineSource->GetOutputPort());
  
  this->ImageVolumeOutlineActor = vtkSmartPointer<vtkActor>::New();
  this->ImageVolumeOutlineActor->SetMapper(this->ImageVolumeOutlineMapper);
}


vtkFluorescenceWidgetsRepresentation
::~vtkFluorescenceWidgetsRepresentation() {

}


void
vtkFluorescenceWidgetsRepresentation
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  this->Simulation = simulation;
}


int
vtkFluorescenceWidgetsRepresentation
::RequestData(vtkInformation* request, vtkInformationVector** inputVector,
              vtkInformationVector* outputVector) {
  return 1;
}


void
vtkFluorescenceWidgetsRepresentation
::PrepareForRendering(vtkRenderView* view) {
  if (this->Simulation) {
    double pixelSize = this->Simulation->GetPixelSize();
    double width  = static_cast<double>(this->Simulation->GetImageWidth()) * pixelSize;
    double height = static_cast<double>(this->Simulation->GetImageHeight()) * pixelSize;
    double depth  = this->Simulation->GetFocalPlaneDepth();

    this->FocalPlaneGrid->SetPoint1(0.0, height, 0.0);
    this->FocalPlaneGrid->SetPoint2(width, 0.0, 0.0);
    this->FocalPlaneGridActor->SetPosition(0.0, 0.0, depth);
    this->FocalPlaneGridActor->
      SetVisibility(this->Simulation->GetShowReferencePlane() ? 1 : 0);
    
    if (this->Simulation->GetShowReferenceGrid()) {
      double spacing = this->Simulation->GetReferenceGridSpacing();
      this->FocalPlaneGrid->SetSpacing(spacing, spacing);
    } else {
      this->FocalPlaneGrid->SetSpacing(width, height);
    }

    this->ImageVolumeOutlineSource->
      SetBounds(0.0, width, 0.0, height, 
                this->Simulation->GetFocalPlaneDepthMinimum(),
                this->Simulation->GetFocalPlaneDepthMaximum());
    this->ImageVolumeOutlineActor->SetVisibility(this->Simulation->GetShowImageVolumeOutline() ? 1 : 0);
  }

}


bool
vtkFluorescenceWidgetsRepresentation
::AddToView(vtkView* view) {
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (!rv) {
    vtkErrorMacro("Can only add to a subclass of vtkRenderView.");
    return false;
  }
  rv->GetRenderer()->AddActor(this->FocalPlaneGridActor);
  rv->GetRenderer()->AddActor(this->ImageVolumeOutlineActor);

  return true;
}


bool
vtkFluorescenceWidgetsRepresentation
::RemoveFromView(vtkView* view) {
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (!rv) {
    return false;
  }
  rv->GetRenderer()->RemoveActor(this->FocalPlaneGridActor);
  rv->GetRenderer()->RemoveActor(this->ImageVolumeOutlineActor);

  return true;
}


void
vtkFluorescenceWidgetsRepresentation
::PrintSelf(ostream& os, vtkIndent indent) {

}
