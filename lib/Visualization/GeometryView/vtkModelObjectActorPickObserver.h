#ifndef _VTK_MODEL_OBJECT_ACTOR_PICK_OBSERVER_H_
#define _VTK_MODEL_OBJECT_ACTOR_PICK_OBSERVER_H_


#include <vtkCommand.h>
#include <vtkModelObjectGeometryRepresentation.h>


class vtkModelObjectActorPickObserver : public vtkCommand {
 public:
  static vtkModelObjectActorPickObserver* New()
  { return new vtkModelObjectActorPickObserver; }
  
  virtual void Execute(vtkObject* caller, unsigned long eventId, void* data);

  void SetRepresentation(vtkModelObjectGeometryRepresentation* rep);

 protected:

  vtkSmartPointer<vtkModelObjectGeometryRepresentation> m_Representation;

 private:
  vtkModelObjectActorPickObserver() { }  
  ~vtkModelObjectActorPickObserver() { }
};


#endif // _VTK_MODEL_OBJECT_ACTOR_PICK_OBSERVER_H_
