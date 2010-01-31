#include <FlexibleTubeGeometrySource.h>

#include <FlexibleTubeModelObject.h>

#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkPoints.h>
#include <vtkTubeFilter.h>
#include <vtkTriangleFilter.h>


FlexibleTubeGeometrySource
::FlexibleTubeGeometrySource(ModelObjectPtr modelObject) 
  : GeometrySource(modelObject) {
  m_Points = vtkSmartPointer<vtkPoints>::New();
  m_Points->Allocate(100, 100);

  m_Spline = vtkSmartPointer<vtkParametricSpline>::New();
  m_Spline->SetPoints(m_Points);

  m_SplineSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
  m_SplineSource->SetParametricFunction(m_Spline);
  m_SplineSource->SetUResolution(100);

  m_TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  m_TubeFilter->CappingOn();
  m_TubeFilter->SetNumberOfSides(16);
  m_TubeFilter->SetInputConnection(m_SplineSource->GetOutputPort());
  
  m_TubeSource = vtkSmartPointer<vtkTriangleFilter>::New();
  m_TubeSource->SetInputConnection(m_TubeFilter->GetOutputPort());

  Update();
}


FlexibleTubeGeometrySource
::~FlexibleTubeGeometrySource() {
}


vtkAlgorithmOutput*
FlexibleTubeGeometrySource
::GetPolyDataOutputPort() {
  m_TubeSource->Update();
  return m_TubeSource->GetOutputPort();
}


void
FlexibleTubeGeometrySource
::Update() {
  double radius = m_ModelObject->GetProperty
    (FlexibleTubeModelObject::RADIUS_PROP)->GetDoubleValue();
  m_TubeFilter->SetRadius(radius);

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
  m_Spline->Modified();
  m_SplineSource->Modified();
}

