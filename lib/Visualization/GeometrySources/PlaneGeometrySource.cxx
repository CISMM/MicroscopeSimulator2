#include <PlaneGeometrySource.h>

#include <PlaneModelObject.h>

#include <vtkPlaneSource.h>


PlaneGeometrySource
::PlaneGeometrySource(ModelObjectPtr modelObject) 
  : GeometrySource(modelObject) {

  m_PlaneSource = vtkSmartPointer<vtkPlaneSource>::New();
  m_PlaneSource->SetOrigin(0.0, 0.0, 0.0);
  m_PlaneSource->SetXResolution(2);
  m_PlaneSource->SetYResolution(2);

  Update();
}


PlaneGeometrySource
::~PlaneGeometrySource() {

}


vtkAlgorithmOutput*
PlaneGeometrySource
::GetPolyDataOutputPort() {
  m_PlaneSource->Update();
  return m_PlaneSource->GetOutputPort();
}


void
PlaneGeometrySource
::Update() {
  double width  = m_ModelObject->GetProperty(PlaneModelObject::WIDTH_PROP)->GetDoubleValue();
  double height = m_ModelObject->GetProperty(PlaneModelObject::HEIGHT_PROP)->GetDoubleValue();
  m_PlaneSource->SetPoint1(width, 0.0, 0.0);
  m_PlaneSource->SetPoint2(0.0, height, 0.0);
}
