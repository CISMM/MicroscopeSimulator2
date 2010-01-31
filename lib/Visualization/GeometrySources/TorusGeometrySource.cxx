#include "TorusGeometrySource.h"

#include <vtkParametricTorus.h>
#include <vtkParametricFunctionSource.h>


TorusGeometrySource
::TorusGeometrySource(ModelObjectPtr modelObject) :
  GeometrySource(modelObject) {

  m_Torus = vtkSmartPointer<vtkParametricTorus>::New();
  m_Torus->SetCrossSectionRadius(m_ModelObject->GetProperty("Cross Section Radius")->GetDoubleValue());
  m_Torus->SetRingRadius(m_ModelObject->GetProperty("Ring Radius")->GetDoubleValue());

  m_TorusSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
  m_TorusSource->SetParametricFunction(m_Torus);
 }


TorusGeometrySource
::~TorusGeometrySource() {

}


vtkAlgorithmOutput*
TorusGeometrySource
::GetPolyDataOutputPort() {
  m_TorusSource->Update();
  return m_TorusSource->GetOutputPort();
}


void
TorusGeometrySource
::Update() {
  m_Torus->SetCrossSectionRadius(m_ModelObject->GetProperty("Cross Section Radius")->GetDoubleValue());
  m_Torus->SetRingRadius(m_ModelObject->GetProperty("Ring Radius")->GetDoubleValue());
}
