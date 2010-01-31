#include "CylinderGeometrySource.h"

#include "CylinderModelObject.h"

#include <vtkCylinderSource.h>


CylinderGeometrySource
::CylinderGeometrySource(ModelObjectPtr modelObject) :
  GeometrySource(modelObject) {

  m_CylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
  m_CylinderSource->CappingOn();
  m_CylinderSource->SetResolution(32);

  Update();
}


CylinderGeometrySource
::~CylinderGeometrySource() {

}


vtkAlgorithmOutput*
CylinderGeometrySource
::GetPolyDataOutputPort() {
  m_CylinderSource->Update();
  return m_CylinderSource->GetOutputPort();
}


void
CylinderGeometrySource
::Update() {
  m_CylinderSource->SetRadius(m_ModelObject->GetProperty("Radius")->GetDoubleValue());
  m_CylinderSource->SetHeight(m_ModelObject->GetProperty("Height")->GetDoubleValue());
}
