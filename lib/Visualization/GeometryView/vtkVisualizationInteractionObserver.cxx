#include "vtkVisualizationInteractionObserver.h"

#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkObject.h>

#include <vtkModelObjectGeometryRepresentation.h>
#include <ModelObject.h>


vtkVisualizationInteractionObserver::vtkVisualizationInteractionObserver() {
  m_Representation = NULL;
}


void vtkVisualizationInteractionObserver::Execute(vtkObject* caller, unsigned long eventId, void* data) {

  if (eventId == vtkCommand::PickEvent) {

    m_Representation = vtkModelObjectGeometryRepresentation::SafeDownCast(caller);

  } else if (eventId == vtkCommand::InteractionEvent) {

    if (m_Representation) {
      vtkActor* actor = m_Representation->GetActor();

      // Modify ModelObject properties.
      ModelObjectPtr mo = m_Representation->GetModelObject();
      mo->SetPosition(actor->GetPosition());
      mo->SetRotation(actor->GetOrientationWXYZ());
    }
  }
  
}
