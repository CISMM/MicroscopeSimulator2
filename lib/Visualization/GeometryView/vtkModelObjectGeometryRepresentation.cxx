#include <vtkModelObjectGeometryRepresentation.h>

#include <ModelObject.h>

#include <vtkActor.h>
#include <vtkApplyColors.h>
#include <vtkGeometryFilter.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTransformFilter.h>
#include <vtkRenderView.h>
#include <vtkRenderedSurfaceRepresentation.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkModelObjectActorPickObserver.h>


vtkCxxRevisionMacro(vtkModelObjectGeometryRepresentation, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkModelObjectGeometryRepresentation);

//----------------------------------------------------------------------------
vtkModelObjectGeometryRepresentation::vtkModelObjectGeometryRepresentation()
  : vtkRenderedSurfaceRepresentation() {

  this->m_ModelObject = NULL;

  this->ActorPickObserver = vtkSmartPointer<vtkModelObjectActorPickObserver>::New();
  this->ActorPickObserver->SetRepresentation(this);

  this->Actor->AddObserver(vtkCommand::PickEvent, ActorPickObserver);

  this->Actor->GetProperty()->SetInterpolationToPhong();
  this->Actor->GetProperty()->ShadingOff();
}

//----------------------------------------------------------------------------
vtkModelObjectGeometryRepresentation::~vtkModelObjectGeometryRepresentation() {
}

//----------------------------------------------------------------------------
vtkActor* vtkModelObjectGeometryRepresentation::GetActor() {
  return this->Actor;
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::SetModelObject(ModelObject* modelObject) {
  m_ModelObject = modelObject;

  this->SetInputConnection(m_ModelObject->GetGeometrySubAssembly("All")->GetOutputPort());
}

//----------------------------------------------------------------------------
ModelObject* vtkModelObjectGeometryRepresentation::GetModelObject() {
  return m_ModelObject;
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::SetPosition(double position[3]) {
  this->Actor->SetPosition(position);
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::SetRotationWXYZ(double rotation[4]) {
  this->Actor->SetOrientation(0.0, 0.0, 0.0);
  this->Actor->RotateWXYZ(rotation[0], rotation[1], rotation[2], rotation[3]);
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::SetColor(double color[3]) {
  this->Actor->GetProperty()->SetColor(color);
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::PrepareForRendering(vtkRenderView* view) {
  this->Superclass::PrepareForRendering(view);

  this->m_ModelObject->Update();

  this->Actor->GetProperty()->SetInterpolationToGouraud();

  vtkInteractorStyle* style =
    vtkInteractorStyle::SafeDownCast(view->GetRenderWindow()->GetInteractor()->
                                     GetInteractorStyle());
  int state = style->GetState();

  vtkInteractorStyleTrackballActor* isActorStyle =
    vtkInteractorStyleTrackballActor::SafeDownCast(style);

  if (!isActorStyle) {
    UpdateRepresentation();
  } else {
    if (state != VTKIS_ROTATE && state != VTKIS_PAN) {
      UpdateRepresentation();
    }
    if (state == VTKIS_USCALE) {
      // Quick way to disable scaling
      this->Actor->SetScale(1.0);
    }
  }

}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::UpdateRepresentation() {
  if (this->m_ModelObject) {

    this->Actor->
      SetVisibility(this->m_ModelObject->GetProperty(ModelObject::VISIBLE_PROP)->GetBoolValue() ? 1 : 0);

    double position[3];
    this->m_ModelObject->GetPosition(position);
    this->SetPosition(position);

    double rotation[4];
    this->m_ModelObject->GetRotation(rotation);
    this->SetRotationWXYZ(rotation);
  }
  
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
}
