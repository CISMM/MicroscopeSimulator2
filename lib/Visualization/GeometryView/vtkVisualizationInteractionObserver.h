#ifndef _VTK_VISUALIZATION_INTERACTION_OBSERVER_H_
#define _VTK_VISUALIZATION_INTERACTION_OBSERVER_H_

#include <vtkCommand.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkObject;

class vtkModelObjectGeometryRepresentation;


class vtkVisualizationInteractionObserver : public vtkCommand
{
 public:
  static vtkVisualizationInteractionObserver* New()
  { return new vtkVisualizationInteractionObserver(); }
  
  virtual void Execute(vtkObject* caller, unsigned long eventId, void* data);

 protected:
  vtkSmartPointer<vtkModelObjectGeometryRepresentation> m_Representation;

 private:
  vtkVisualizationInteractionObserver();

  ~vtkVisualizationInteractionObserver() { }
};


#endif // _VTK_VISUALIZATION_INTERACTION_OBSERVER_H_
