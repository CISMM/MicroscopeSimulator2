#include <PointSetGeometrySource.h>

#include <ModelObject.h>
#include <PointSetModelObject.h>

#include <vtkGlyph3D.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>


PointSetGeometrySource
::PointSetGeometrySource(ModelObjectPtr modelObject) 
  : GeometrySource(modelObject) {
  m_Points = vtkSmartPointer<vtkPoints>::New();
  m_Points->Allocate(100, 100);
  
  m_PointSource = vtkSmartPointer<vtkPolyData>::New();
  m_PointSource->SetPoints(m_Points);
  
  m_SphereSource = vtkSmartPointer<vtkSphereSource>::New();
  m_SphereSource->SetThetaResolution(16);
  m_SphereSource->SetPhiResolution(8);

  m_GlyphSource = vtkSmartPointer<vtkGlyph3D>::New();
  m_GlyphSource->SetInput(m_PointSource);
  m_GlyphSource->SetSourceConnection(m_SphereSource->GetOutputPort());

  Update();
}


PointSetGeometrySource
::~PointSetGeometrySource() {
}


vtkAlgorithmOutput*
PointSetGeometrySource
::GetPolyDataOutputPort() {
  m_GlyphSource->Update();
  return m_GlyphSource->GetOutputPort();
}


void
PointSetGeometrySource
::Update() {
  double radius = m_ModelObject->GetProperty
    (PointSetModelObject::VISIBLE_RADIUS_PROP)->GetDoubleValue();
  m_SphereSource->SetRadius(radius);

  int numPoints = 
    m_ModelObject->GetProperty(PointSetModelObject::NUMBER_OF_POINTS_PROP)->GetIntValue();

  m_Points->SetNumberOfPoints(numPoints);

  for (int i = 1; i <= numPoints; i++) {
    char buf[128];
    double xyz[3];

    sprintf(buf, "X%d", i);
    xyz[0] = m_ModelObject->GetProperty(std::string(buf))->GetDoubleValue();

    sprintf(buf, "Y%d", i);
    xyz[1] = m_ModelObject->GetProperty(std::string(buf))->GetDoubleValue();

    sprintf(buf, "Z%d", i);
    xyz[2] = m_ModelObject->GetProperty(std::string(buf))->GetDoubleValue();

    m_Points->SetPoint(static_cast<vtkIdType>(i-1), xyz);
  }
  m_Points->Modified();
}
