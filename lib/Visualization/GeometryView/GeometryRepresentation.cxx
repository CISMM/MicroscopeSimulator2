#include <GeometryRepresentation.h>

#include <ModelObject.h>
#include <ModelObjectList.h>

#include <vtkAlgorithmOutput.h>
#include <vtkModelObjectGeometryRepresentation.h>
#include <vtkView.h>


GeometryRepresentation
::GeometryRepresentation() {
  m_ModelObjectList = NULL;
  m_Observer = NULL;
}


GeometryRepresentation
::~GeometryRepresentation() {
  
}


void
GeometryRepresentation
::SetModelObjectList(ModelObjectList* list) {
  m_ModelObjectList = list;

  Update();
}


ModelObjectList*
GeometryRepresentation
::GetModelObjectList() {
  return m_ModelObjectList;
}


void
GeometryRepresentation
::SetInteractionObserver(vtkCommand* observer) {
  m_Observer = observer;
}


void
GeometryRepresentation
::SetShowFluorophores(bool value) {
  m_ShowFluorophores = value;

  std::list<vtkModelObjectGeometryRepresentation*>::iterator iter;
  for (iter = m_GeometryReps.begin(); iter != m_GeometryReps.end(); iter++) {
    (*iter)->SetShowFluorophores(m_ShowFluorophores ? 1 : 0);
  }
}


void
GeometryRepresentation
::AddToView(vtkView* view) {
  std::list<vtkModelObjectGeometryRepresentation*>::iterator iter;
  for (iter = m_GeometryReps.begin(); iter != m_GeometryReps.end(); iter++) {
    (*iter)->Update();
    view->AddRepresentation(*iter);
  }
}


void
GeometryRepresentation
::Update() {
  if (m_ModelObjectList == NULL || m_ModelObjectList->GetSize() != m_GeometryReps.size()) {
    // Something was added or deleted. We don't know what it was, so rebuild
    // the whole geometry list.
    while (!m_GeometryReps.empty()) {
      vtkModelObjectGeometryRepresentation* rep = m_GeometryReps.front();
      rep->Delete();
      m_GeometryReps.pop_front();
    }
    
    for (int i = 0; i < static_cast<int>(m_ModelObjectList->GetSize()); i++) {
      ModelObject* newModelObject = m_ModelObjectList->GetModelObjectAtIndex(i);
      m_GeometryReps.push_back(CreateRepresentation(newModelObject));
    }    
  }
  
  std::list<vtkModelObjectGeometryRepresentation*>::iterator iter;
  for (iter = m_GeometryReps.begin(); iter != m_GeometryReps.end(); iter++) {
    (*iter)->SetShowFluorophores(m_ShowFluorophores ? 1 : 0);
    (*iter)->Update();
  }
}


vtkModelObjectGeometryRepresentation*
GeometryRepresentation
::CreateRepresentation(ModelObject* modelObject) {
  std::cout << "Creating geometry representation for " << modelObject->GetName() << std::endl;

  vtkModelObjectGeometryRepresentation* geomRep =
    vtkModelObjectGeometryRepresentation::New();
  geomRep->SetModelObject(modelObject);

  // Need to call this to prevent crash in vtkDataSet->ComputeBounds
  geomRep->Update();

  if (m_Observer) {
    geomRep->AddObserver(vtkCommand::PickEvent, m_Observer);
  }
  
  return geomRep;
}
