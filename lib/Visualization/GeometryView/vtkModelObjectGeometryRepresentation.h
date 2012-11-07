#ifndef _VTK_MODEL_OBJECT_GEOMETRY_REPRESENTATION_H_
#define _VTK_MODEL_OBJECT_GEOMETRY_REPRESENTATION_H_

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCommand.h>
#include <vtkRenderedSurfaceRepresentation.h>


// Forward declarations
class ModelObject;
class vtkRenderedSurfaceRepresentation;
class vtkModelObjectActorPickObserver;


class vtkModelObjectGeometryRepresentation : public vtkRenderedSurfaceRepresentation {
 public:
  static vtkModelObjectGeometryRepresentation *New();
  vtkTypeMacro(vtkModelObjectGeometryRepresentation, vtkRenderedSurfaceRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkActor* GetActor();

  void SetModelObject(ModelObject* modelObject);
  ModelObject* GetModelObject();

  vtkSetMacro(ShowFluorophores, int);
  vtkGetMacro(ShowFluorophores, int);
  vtkBooleanMacro(ShowFluorophores, int);

 protected:
  vtkModelObjectGeometryRepresentation();
  ~vtkModelObjectGeometryRepresentation();

  ModelObject* m_ModelObject;

  vtkSmartPointer<vtkModelObjectActorPickObserver> ActorPickObserver;

  int ShowFluorophores;

  vtkSmartPointer<vtkActorCollection> FluorophoreActors;

  virtual void PrepareForRendering(vtkRenderView* view);

  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);

  virtual void UpdateRepresentation();
  virtual void UpdateFluorophoreRepresentation();

  void SetPosition(double position[3]);
  void SetRotationWXYZ(double rotation[4]);

  void SetColor(double color[3]);


 private:
  vtkModelObjectGeometryRepresentation(const vtkModelObjectGeometryRepresentation&); // Purposely not implemented.
  void operator=(const vtkModelObjectGeometryRepresentation&); // Purposely not implemented
};

#endif // _VTK_MODEL_OBJECT_GEOMETRY_REPRESENTATION_H_
