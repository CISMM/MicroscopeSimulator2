#include <vtkImageData.h>

#include <FluorescenceImageSource.h>
#include <FluorescenceSimulation.h>
#include <ImageModelObject.h>
#include <ModelObjectList.h>
#include <VisualizationFluorescenceImageSource.h>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <FluorescenceOptimizer.h>


FluorescenceOptimizer
::FluorescenceOptimizer() {
  m_FluoroSim = NULL;
  m_ModelObjectList = NULL;
  m_ComparisonImageModelObject = NULL;
}


FluorescenceOptimizer
::~FluorescenceOptimizer() {
}


void
FluorescenceOptimizer
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  m_FluoroSim = simulation;
}


void
FluorescenceOptimizer
::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;
}


void
FluorescenceOptimizer
::SetComparisonImageModelObjectIndex(int index) {
  if (index < 0) {
    m_ComparisonImageModelObject = NULL;
    return;
  }

  ModelObjectPtr mo = m_ModelObjectList->GetModelObjectAtIndex(index, ImageModelObject::OBJECT_TYPE_NAME);
  m_ComparisonImageModelObject = static_cast<ImageModelObject*>(mo);
}


ImageModelObject*
FluorescenceOptimizer
::GetComparisonImageModelObject() {
  return m_ComparisonImageModelObject;
}


void
FluorescenceOptimizer
::SetParameters(double* params) {
  if (!m_ModelObjectList)
    return;

  m_FluoroSim->GetFluorescenceImageSource()->SetParameters(params);
}


int
FluorescenceOptimizer
::GetNumberOfAvailableObjectiveFunctions() const {
  return m_ObjectiveFunctionNames.size();
}


std::string
FluorescenceOptimizer
::GetAvailableObjectiveFunctionName(int index) {
  if (index < 0 || index > 2)
    return std::string();

  return m_ObjectiveFunctionNames[index];
}


void
FluorescenceOptimizer
::AddObjectiveFunctionName(const std::string& name) {
  m_ObjectiveFunctionNames.push_back(name);
}
