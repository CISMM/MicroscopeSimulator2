#include "SphereGeometrySource.h"

#include <vtkSphereSource.h>


SphereGeometrySource
::SphereGeometrySource(ModelObjectPtr modelObject) :
  GeometrySource(modelObject) {

  m_SphereSource = vtkSmartPointer<vtkSphereSource>::New();
  m_SphereSource->SetThetaResolution(32);
  m_SphereSource->SetPhiResolution(16);

  Update();
}


SphereGeometrySource
::~SphereGeometrySource() {

}


vtkAlgorithmOutput*
SphereGeometrySource
::GetPolyDataOutputPort() {
  m_SphereSource->Update();
  return m_SphereSource->GetOutputPort();
}


void
SphereGeometrySource
::Update() {
  m_SphereSource->SetRadius(m_ModelObject->GetProperty("Radius")->GetDoubleValue());
}
