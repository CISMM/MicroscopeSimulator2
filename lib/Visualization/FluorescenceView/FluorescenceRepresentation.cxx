#include <FluorescenceRepresentation.h>

#include <FluorophoreModelObjectProperty.h>
#include <ModelObject.h>
#include <ModelObjectList.h>
#include <ModelObjectProperty.h>
#include <ModelObjectPropertyList.h>

#include <vtkAlgorithmOutput.h>
#include <vtkModelObjectFluorescenceRepresentation.h>
#include <vtkPolyDataToTetrahedralGrid.h>
#include <vtkSurfaceUniformPointSampler.h>
#include <vtkTriangleFilter.h>
#include <vtkView.h>
#include <vtkVolumeUniformPointSampler.h>


FluorescenceRepresentation
::FluorescenceRepresentation() {
  m_ModelObjectList = NULL;
}


FluorescenceRepresentation
::~FluorescenceRepresentation() {

}


void
FluorescenceRepresentation
::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;
}


ModelObjectList*
FluorescenceRepresentation
::GetModelObjectList() {
  return m_ModelObjectList;
}


void
FluorescenceRepresentation
::SetFluorescenceSimulation(FluorescenceSimulation* simulation) {
  m_Simulation = simulation;
}


void
FluorescenceRepresentation
::AddToView(vtkView* view) {
  std::vector<vtkModelObjectFluorescenceRepresentation*>::iterator iter;
  for (iter = m_FluorescenceReps.begin(); iter != m_FluorescenceReps.end(); iter++) {
    (*iter)->Update();
    view->AddRepresentation(*iter);
  }
}


void
FluorescenceRepresentation
::Update() {
  UpdateRepresentation();
  UpdateSources();
}


float*
FluorescenceRepresentation
::GetPointsGradientForRepresentation(int repIndex, int& numPoints) {
  return m_FluorescenceReps[repIndex]->GetPointsGradient(numPoints);
}


vtkModelObjectFluorescenceRepresentation*
FluorescenceRepresentation
::CreateRepresentation(ModelObject* object,
                       FluorophoreModelObjectProperty* property) {
  std::cout << "Creating fluorescence representation for " << property->GetName() << std::endl;

  vtkModelObjectFluorescenceRepresentation* fluorRep = vtkModelObjectFluorescenceRepresentation::New();
  fluorRep->SetModelObject(object);
  fluorRep->SetFluorophoreModelObjectProperty(property);
  
  // Need to call this to prevent crash in vtkDataSet->ComputeBounds
  fluorRep->Update();
  
  return fluorRep;
}


void
FluorescenceRepresentation
::AddRepresentations(ModelObject* modelObject) {
  ModelObjectPropertyList* fluorProperties = modelObject->GetFluorophorePropertyList();
  for (int i = 0; i < modelObject->GetNumberOfFluorophoreProperties(); i++) {
    FluorophoreModelObjectProperty* fluorProp = 
      dynamic_cast<FluorophoreModelObjectProperty*>(fluorProperties->GetProperty(i));
    m_FluorescenceReps.push_back(CreateRepresentation(modelObject, fluorProp));
  }
}


void
FluorescenceRepresentation
::UpdateRepresentation() {
  // Sum up number of fluorescence representations
  unsigned int necessaryReps = 0;
  for (unsigned int i = 0; i < m_ModelObjectList->GetSize(); i++) {
    necessaryReps += m_ModelObjectList->GetModelObjectAtIndex(i)->
      GetNumberOfFluorophoreProperties();
  }
  
  if (necessaryReps != m_FluorescenceReps.size()) {
    // Rebuild the whole geometry list
    for (unsigned int i = 0; i < m_FluorescenceReps.size(); i++) {
      m_FluorescenceReps[i]->Delete();
    }
    m_FluorescenceReps.clear();
#if 0
    while (!m_FluorescenceReps.empty()) {
      vtkModelObjectFluorescenceRepresentation* rep = m_FluorescenceReps.front();
      rep->Delete();
      m_FluorescenceReps.pop_front();
    }
#endif

    for (int i = 0; i < static_cast<int>(m_ModelObjectList->GetSize()); i++) {
      ModelObject* object = m_ModelObjectList->GetModelObjectAtIndex(i);
      AddRepresentations(object);
    }
    
  }
  // If list is the same size, there is nothing to do.
}


void
FluorescenceRepresentation
::UpdateSources() {
  std::vector<vtkModelObjectFluorescenceRepresentation*>::iterator iter;
  for (iter = m_FluorescenceReps.begin(); iter != m_FluorescenceReps.end(); iter++) {
    (*iter)->Update();
  }
}
