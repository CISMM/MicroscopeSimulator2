#include <vtkImageData.h>

#include <FluorescenceImageSource.h>
#include <FluorescenceSimulation.h>
#include <ImageModelObject.h>
#include <ModelObject.h>
#include <ModelObjectList.h>
#include <StringUtils.h>
#include <VisualizationFluorescenceImageSource.h>
#include <XMLHelper.h>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <FluorescenceOptimizer.h>


FluorescenceOptimizer
::FluorescenceOptimizer(DirtyListener* listener) {
  m_DirtyListener = listener;
  m_FluoroSim = NULL;
  m_ModelObjectList = NULL;
  m_ComparisonImageModelObject = NULL;
}


FluorescenceOptimizer
::~FluorescenceOptimizer() {
}


void
FluorescenceOptimizer
::GetXMLConfiguration(xmlNodePtr node) {
  char buf[128];
  for (int i = 0; i < GetNumberOfOptimizerParameters(); i++) {
    Parameter p = GetOptimizerParameter(i);

    switch (p.type) {
    case INT_TYPE:
      sprintf(buf, "%d", p.value.iValue);
      break;

    case FLOAT_TYPE:
      sprintf(buf, "%g", p.value.fValue);
      break;

    case DOUBLE_TYPE:
      sprintf(buf, "%g", p.value.dValue);
    break;

    case INVALID_TYPE:
      sprintf(buf, "invalid");
      break;

    default:
      break;
    }

    std::string nodeName = SqueezeString(p.name);
    
    xmlNodePtr paramNode = xmlNewChild(node, NULL, BAD_CAST nodeName.c_str(), NULL);
    xmlNewProp(paramNode, BAD_CAST "value", BAD_CAST buf);
  }
}


void
FluorescenceOptimizer
::RestoreFromXML(xmlNodePtr node) {
  for (int i = 0; i < GetNumberOfOptimizerParameters(); i++) {
    Parameter p = GetOptimizerParameter(i);

    std::string nodeName = SqueezeString(p.name);
    
    xmlNodePtr parameterNode =
      xmlGetFirstElementChildWithName(node, BAD_CAST nodeName.c_str());

    if (!parameterNode)
      continue;
    
    char* value = (char *) xmlGetProp(parameterNode, BAD_CAST "value");
    if (!value)
      continue;

    FluorescenceOptimizer::Variant newValue = p.value;
    switch (p.type) {
    case INT_TYPE:
      newValue.iValue = atoi(value);
      break;

    case FLOAT_TYPE:
      newValue.fValue = atof(value);
      break;

    case DOUBLE_TYPE:
      newValue.dValue = atof(value);
      break;

    default:
      newValue.iValue = 0;
      newValue.fValue = 0.0f;
      newValue.dValue = 0.0;
      break;
    }

    SetOptimizerParameterValue(i, newValue);
  }
}


void
FluorescenceOptimizer
::Sully() {
  if (m_DirtyListener)
    m_DirtyListener->Sully();
}


void
FluorescenceOptimizer
::SetStatusMessage(const std::string& status) {
  if (m_DirtyListener)
    m_DirtyListener->SetStatusMessage(status);
}


void
FluorescenceOptimizer
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  m_FluoroSim = simulation;
}


int
FluorescenceOptimizer
::GetNumberOfOptimizerParameters() {
  return m_OptimizerParameters.size();
}


void
FluorescenceOptimizer
::SetOptimizerParameterNumericType(int index, NumericType type) {
  m_OptimizerParameters[index].type = type;

  Sully();
}


FluorescenceOptimizer::NumericType
FluorescenceOptimizer
::GetOptimizerParameterNumericType(int index) {
  return m_OptimizerParameters[index].type;
}


void
FluorescenceOptimizer
::SetOptimizerParameterValue(int index, Variant value) {
  m_OptimizerParameters[index].value = value;
  
  Sully();
}


FluorescenceOptimizer::Variant
FluorescenceOptimizer
::GetOptimizerParameterValue(int index) {
  return m_OptimizerParameters[index].value;
}


FluorescenceOptimizer::Parameter
FluorescenceOptimizer
::GetOptimizerParameter(int index) {
  return m_OptimizerParameters[index];
}


FluorescenceOptimizer::Parameter
FluorescenceOptimizer
::GetOptimizerParameter(const std::string& name) {
  for (size_t i = 0; i < m_OptimizerParameters.size(); i++) {
    if (m_OptimizerParameters[i].name == name) {
      return m_OptimizerParameters[i];
    }
  }

  Parameter p;
  p.name = std::string();
  p.type = INVALID_TYPE;

  return p;
}


FluorescenceOptimizer::Variant
FluorescenceOptimizer
::GetOptimizerParameterValue(const std::string& name) {
  return GetOptimizerParameter(name).value;
}


double
FluorescenceOptimizer
::GetObjectiveFunctionValue() {
  return DBL_MAX; // default implementation, should be overridden.
}


void
FluorescenceOptimizer
::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;
}


void
FluorescenceOptimizer
::SetComparisonImageModelObject(ModelObject* object) {
  m_ComparisonImageModelObject = static_cast<ImageModelObject*>(object);

  Sully();
}


void
FluorescenceOptimizer
::SetComparisonImageModelObjectIndex(int index) {
  if (index < 0) {
    m_ComparisonImageModelObject = NULL;
    return;
  }

  ModelObjectPtr mo = m_ModelObjectList->GetModelObjectAtIndex(index, ImageModelObject::OBJECT_TYPE_NAME);
  SetComparisonImageModelObject(mo);

  Sully();
}


ImageModelObject*
FluorescenceOptimizer
::GetComparisonImageModelObject() {
  return m_ComparisonImageModelObject;
}


void
FluorescenceOptimizer
::SetObjectiveFunctionByName(const std::string& name) {
  m_ActiveObjectiveFunctionName = name;
  std::cout << "Setting objective function to '" << name << "'." << std::endl;
}


int
FluorescenceOptimizer
::GetNumberOfParameters() {
  if (m_ModelObjectList && 
      m_FluoroSim && 
      m_FluoroSim->GetFluorescenceImageSource()) {
    return m_FluoroSim->GetFluorescenceImageSource()->GetNumberOfParameters();
  }

  return 0;
}


void
FluorescenceOptimizer
::SetParameters(double* params) {
  if (!m_ModelObjectList)
    return;

  m_FluoroSim->GetFluorescenceImageSource()->SetParameters(params);

  Sully();
}


int
FluorescenceOptimizer
::GetNumberOfAvailableObjectiveFunctions() const {
  return m_ObjectiveFunctionNames.size();
}


std::string
FluorescenceOptimizer
::GetAvailableObjectiveFunctionName(int index) {
  if (index < 0 || static_cast<unsigned int>(index) >= m_ObjectiveFunctionNames.size()) {
    return std::string();
  }

  return m_ObjectiveFunctionNames[index];
}


void
FluorescenceOptimizer
::AddObjectiveFunctionName(const std::string& name) {
  m_ObjectiveFunctionNames.push_back(name);
}


void
FluorescenceOptimizer
::AddOptimizerParameter(const std::string& name, NumericType type, Variant value) {
  Parameter param;
  param.name = name;
  param.type = type;
  param.value = value;
  m_OptimizerParameters.push_back(param);
}
