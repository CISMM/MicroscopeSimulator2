#ifndef _FLUORESCENCE_OPTIMIZER_H_
#define _FLUORESCENCE_OPTIMIZER_H_

#include <string>
#include <vector>

#include <DirtyListener.h>
#include <XMLStorable.h>

class FluorescenceSimulation;
class ImageModelObject;
class ModelObject;
class ModelObjectList;


class FluorescenceOptimizer : public DirtyListener, public XMLStorable {

 public:

  typedef enum {
    INT_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE
  } NumericType;

  typedef union _Variant {
    int    iValue;
    float  fValue;
    double dValue;
  } Variant;

  typedef struct _Parameter {
    std::string name;
    NumericType type;
    Variant     value;
  } Parameter;
  
  /** Constructor/destructor. */
  FluorescenceOptimizer();
  virtual ~FluorescenceOptimizer();

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  virtual void Sully();
  virtual void SetStatusMessage(const std::string& status);

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);

  void SetModelObjectList(ModelObjectList* list);
  void SetComparisonImageModelObject(ModelObject* object);
  void SetComparisonImageModelObjectIndex(int index);
  ImageModelObject* GetComparisonImageModelObject();

  virtual void Optimize() = 0;

  void SetParameters(double* params);

  virtual int GetNumberOfAvailableObjectiveFunctions() const;
  virtual std::string GetAvailableObjectiveFunctionName(int index);

  int GetNumberOfOptimizerParameters();

  void        SetOptimizerParameterNumericType(int index, NumericType type);
  NumericType GetOptimizerParameterNumericType(int index);

  void    SetOptimizerParameterValue(int index, Variant value);
  Variant GetOptimizerParameterValue(int index);

  Parameter GetOptimizerParameter(int index);

 protected:
  FluorescenceSimulation* m_FluoroSim;

  ModelObjectList* m_ModelObjectList;
  ImageModelObject* m_ComparisonImageModelObject;

  // List of available objective functions
  std::vector<std::string> m_ObjectiveFunctionNames;

  // The chosen objective function
  std::string m_ObjectiveFunction;

  // Optimizer parameters
  std::vector<Parameter> m_OptimizerParameters;

  void AddObjectiveFunctionName(const std::string& name);

  void AddOptimizerParameter(const std::string& name, NumericType type, Variant value);

};

#endif
