#include "DiskGeometrySource.h"

#include "DiskModelObject.h"

#include <vtkDiskSource.h>


DiskGeometrySource
::DiskGeometrySource(ModelObjectPtr modelObject) :
  GeometrySource(modelObject) {

  m_DiskSource = vtkSmartPointer<vtkDiskSource>::New();
  m_DiskSource->SetInnerRadius(0.0);
  m_DiskSource->SetCircumferentialResolution(32);

  Update();
}


DiskGeometrySource
::~DiskGeometrySource() {

}


vtkAlgorithmOutput*
DiskGeometrySource
::GetPolyDataOutputPort() {
  m_DiskSource->Update();
  return m_DiskSource->GetOutputPort();
}


void
DiskGeometrySource
::Update() {
  m_DiskSource->SetOuterRadius(m_ModelObject->GetProperty("Radius")->GetDoubleValue());
}
