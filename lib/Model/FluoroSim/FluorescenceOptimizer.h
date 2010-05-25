#ifndef _FLUORESCENCE_OPTIMIZER_H_
#define _FLUORESCENCE_OPTIMIZER_H_

#include <string>
#include <vector>

class FluorescenceSimulation;
class ImageModelObject;
class ModelObjectList;

class FluorescenceOptimizer {

 public:

  /** Constructor/destructor. */
  FluorescenceOptimizer();
  virtual ~FluorescenceOptimizer();

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  void SetModelObjectList(ModelObjectList* list);
  void SetComparisonImageModelObjectIndex(int index);
  ImageModelObject* GetComparisonImageModelObject();

  virtual void Optimize() = 0;

  void SetParameters(double* params);

  virtual int GetNumberOfAvailableObjectiveFunctions() const;
  virtual std::string GetAvailableObjectiveFunctionName(int index);

 protected:
  FluorescenceSimulation* m_FluoroSim;

  ModelObjectList* m_ModelObjectList;
  ImageModelObject* m_ComparisonImageModelObject;

  // List of available objective functions
  std::vector<std::string> m_ObjectiveFunctionNames;

  // The chosen objective function
  std::string m_ObjectiveFunction;

  void AddObjectiveFunctionName(const std::string& name);

};

#endif
