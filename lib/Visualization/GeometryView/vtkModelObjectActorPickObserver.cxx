#include "vtkModelObjectActorPickObserver.h"


void vtkModelObjectActorPickObserver::Execute(vtkObject* caller, unsigned long eventId, void* data) {
  if (m_Representation) {
    m_Representation->InvokeEvent(eventId, data);
  }
}


void vtkModelObjectActorPickObserver::SetRepresentation(vtkModelObjectGeometryRepresentation* rep) {
  m_Representation = rep;
}
