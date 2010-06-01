#include <vtkModelObjectGeometryRepresentation.h>

#include <FluorophoreModelObjectProperty.h>
#include <ModelObject.h>
#include <ModelObjectProperty.h>
#include <ModelObjectPropertyList.h>

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkApplyColors.h>
#include <vtkCollection.h>
#include <vtkGeometryFilter.h>
#include <vtkGlyph3D.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderView.h>
#include <vtkRenderedSurfaceRepresentation.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkTransformFilter.h>

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

  this->ShowFluorophores = 0;
  
  this->FluorophoreActors = vtkSmartPointer<vtkActorCollection>::New();
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

  this->Actor->SetPickable(m_ModelObject->GetPickable() ? 1 : 0);

  this->SetInputConnection(m_ModelObject->GetGeometrySubAssembly("All")->GetOutputPort());

  // Clear out fluorophore actors and generate new ones
  this->FluorophoreActors->RemoveAllItems();

  ModelObjectPropertyList* mopList = m_ModelObject->GetFluorophorePropertyList();
  for (int i = 0; i < mopList->GetSize(); i++) {
    FluorophoreModelObjectProperty* fmop = 
      dynamic_cast<FluorophoreModelObjectProperty*>(mopList->GetProperty(i));

    if (!fmop)
      continue;

    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetRadius(20.0);

    vtkSmartPointer<vtkGlyph3D> glypher = vtkSmartPointer<vtkGlyph3D>::New();
    glypher->ScalingOff();
    glypher->SetInputConnection(fmop->GetFluorophoreOutput()->GetOutputPort());
    glypher->SetSourceConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glypher->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->PickableOff();
    actor->SetMapper(mapper);

    this->FluorophoreActors->AddItem(actor);
  }
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

  UpdateFluorophoreRepresentation();

}

//----------------------------------------------------------------------------
bool vtkModelObjectGeometryRepresentation::AddToView(vtkView* view) {
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (!rv)
    {
    vtkErrorMacro("Can only add to a subclass of vtkRenderView.");
    return false;
    }
  rv->GetRenderer()->AddActor(this->Actor);

  vtkCollectionSimpleIterator iter;
  this->FluorophoreActors->InitTraversal(iter);
  vtkActor* fluorophoreActor;
  while ((fluorophoreActor = this->FluorophoreActors->GetNextActor(iter)) != NULL) {
    rv->GetRenderer()->AddActor(fluorophoreActor);
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkModelObjectGeometryRepresentation::RemoveFromView(vtkView* view) {
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (!rv)
    {
    vtkErrorMacro("Can only add to a subclass of vtkRenderView.");
    return false;
    }

  rv->GetRenderer()->RemoveViewProp(this->Actor);

  vtkCollectionSimpleIterator iter;
  this->FluorophoreActors->InitTraversal(iter);
  vtkActor* fluorophoreActor;
  while ((fluorophoreActor = this->FluorophoreActors->GetNextActor(iter)) != NULL) {
    rv->GetRenderer()->RemoveViewProp(fluorophoreActor);
  }

  return true;
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

  UpdateFluorophoreRepresentation();
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::UpdateFluorophoreRepresentation() {
  if (this->m_ModelObject) {
    double* position;
    double* rotation;

    position = this->Actor->GetPosition();
    rotation = this->Actor->GetOrientationWXYZ();

    vtkCollectionSimpleIterator iter;
    this->FluorophoreActors->InitTraversal(iter);
    int fluorophorePropertyIndex = 0;
    vtkActor* fluorophoreActor;
    while ((fluorophoreActor = this->FluorophoreActors->GetNextActor(iter)) != NULL) {
      fluorophoreActor->SetOrientation(0.0, 0.0, 0.0);
      fluorophoreActor->RotateWXYZ(rotation[0], rotation[1], rotation[2], rotation[3]);
      fluorophoreActor->SetPosition(position);

      FluorophoreModelObjectProperty* fmop = dynamic_cast<FluorophoreModelObjectProperty*>
        (m_ModelObject->GetFluorophorePropertyList()->GetProperty(fluorophorePropertyIndex));
      fluorophorePropertyIndex++;

      bool visible = fmop->GetEnabled() &&
        m_ModelObject->GetProperty(ModelObject::VISIBLE_PROP)->GetBoolValue() &&
        (bool) this->ShowFluorophores;
      fluorophoreActor->SetVisibility(visible ? 1 : 0);

      vtkProperty* property = fluorophoreActor->GetProperty();
      switch (fmop->GetFluorophoreChannel()) {
      case RED_CHANNEL:
        property->SetColor(1.0, 0.0, 0.0);
        break;

      case GREEN_CHANNEL:
        property->SetColor(0.0, 1.0, 0.0);
        break;

      case BLUE_CHANNEL:
        property->SetColor(0.0, 0.0, 1.0);
        break;

      default:
        property->SetColor(1.0, 1.0, 1.0);
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkModelObjectGeometryRepresentation::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
}
