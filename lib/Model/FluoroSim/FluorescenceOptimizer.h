#ifndef _FLUORESCENCE_OPTIMIZER_H_
#define _FLUORESCENCE_OPTIMIZER_H_

class FluorescenceSimulation;
class ModelObjectList;


class FluorescenceOptimizer {

 public:
  FluorescenceOptimizer();
  virtual ~FluorescenceOptimizer();

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  void SetModelObjectList(ModelObjectList* list);

  void Optimize();

  void SetParameters(double params[]);

 protected:
  FluorescenceSimulation* m_FluoroSim;

  ModelObjectList* m_ModelObjectList;
};

#endif
