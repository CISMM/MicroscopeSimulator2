#ifndef _VTK_MODEL_OBJECT_GEOMETRY_REPRESENTATION_H_
#define _VTK_MODEL_OBJECT_GEOMETRY_REPRESENTATION_H_

#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkRenderedSurfaceRepresentation.h>


// Forward declarations
class ModelObject;
class vtkRenderedSurfaceRepresentation;
class vtkModelObjectActorPickObserver;


class vtkModelObjectGeometryRepresentation : public vtkRenderedSurfaceRepresentation {
 public:
  static vtkModelObjectGeometryRepresentation *New();
  vtkTypeRevisionMacro(vtkModelObjectGeometryRepresentation, vtkRenderedSurfaceRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkActor* GetActor();

  void SetModelObject(ModelObject* modelObject);
  ModelObject* GetModelObject();

 protected:
  vtkModelObjectGeometryRepresentation();
  ~vtkModelObjectGeometryRepresentation();

  ModelObject* m_ModelObject;

  vtkSmartPointer<vtkModelObjectActorPickObserver> ActorPickObserver;

  virtual void PrepareForRendering(vtkRenderView* view);

  virtual void UpdateRepresentation();

  void SetPosition(double position[3]);
  void SetRotationWXYZ(double rotation[4]);

  void SetColor(double color[3]);


 private:
  vtkModelObjectGeometryRepresentation(const vtkModelObjectGeometryRepresentation&); // Purposely not implemented.
  void operator=(const vtkModelObjectGeometryRepresentation&); // Purposely not implemented
};

#endif // _VTK_MODEL_OBJECT_GEOMETRY_REPRESENTATION_H_
