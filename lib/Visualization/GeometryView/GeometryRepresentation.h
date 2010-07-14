#ifndef _GEOMETRY_REPRESENTATION_H_
#define _GEOMETRY_REPRESENTATION_H_

#include <list>

// Forward declarations
class ModelObject;
class ModelObjectList;
class vtkCommand;
class vtkPolyDataAlgorithm;
class vtkModelObjectGeometryRepresentation;
class vtkView;


class GeometryRepresentation {

 public:
  GeometryRepresentation();
  virtual ~GeometryRepresentation();

  void SetModelObjectList(ModelObjectList* list);
  ModelObjectList* GetModelObjectList();

  void SetInteractionObserver(vtkCommand* observer);

  void SetShowFluorophores(bool value);
  bool GetShowFluorophores();

  void AddToView(vtkView* view);

  void Update();

 protected:
  ModelObjectList* m_ModelObjectList;

  bool m_ShowFluorophores;

  vtkCommand* m_Observer;

  std::list<vtkModelObjectGeometryRepresentation*> m_GeometryReps;

  vtkModelObjectGeometryRepresentation* 
    CreateRepresentation(ModelObject* modelObject);

};


#endif // _GEOMETRY_REPRESENTATION_H_
