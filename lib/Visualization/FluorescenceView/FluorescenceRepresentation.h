#ifndef _FLUORESCENCE_REPRESENTATION_H_
#define _FLUORESCENCE_REPRESENTATION_H_

#include <vector>

class FluorophoreModelObjectProperty;
class FluorescenceSimulation;
class ModelObject;
class ModelObjectList;
class vtkCommand;
class vtkPolyDataAlgorithm;
class vtkModelObjectFluorescenceRepresentation;
class vtkView;


class FluorescenceRepresentation {
  
 public:
  FluorescenceRepresentation();
  virtual ~FluorescenceRepresentation();

  void SetModelObjectList(ModelObjectList* list);
  ModelObjectList* GetModelObjectList();

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  void AddToView(vtkView* view);

  void Update();

  float* GetPointsGradientForRepresentation(int repIndex, int& numPoints);

 protected:
  ModelObjectList* m_ModelObjectList;
  FluorescenceSimulation* m_Simulation;

  std::vector<vtkModelObjectFluorescenceRepresentation*> m_FluorescenceReps;

  vtkModelObjectFluorescenceRepresentation*
    CreateRepresentation(ModelObject* object,
                         FluorophoreModelObjectProperty* property);

  void AddRepresentations(ModelObject* modelObject);

  void UpdateRepresentation();
  void UpdateSources();

};

#endif // _FLUORESCENCE_REPRESENTATION_H_
