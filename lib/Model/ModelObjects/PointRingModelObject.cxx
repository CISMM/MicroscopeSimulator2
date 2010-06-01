#include <PointRingModelObject.h>
#include <GeometryVerticesFluorophoreProperty.h>

#include <DirtyListener.h>
#include <ModelObjectPropertyList.h>
#include <XMLHelper.h>

#include <vtkGlyph3D.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkVertexGlyphFilter.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const char* PointRingModelObject::OBJECT_TYPE_NAME = "PointRingModel";

const char* PointRingModelObject::VISIBLE_RADIUS_PROP       = "Visible Radius";
const char* PointRingModelObject::RING_RADIUS_PROP          = "Ring Radius";
const char* PointRingModelObject::NUMBER_OF_POINTS_PROP     = "Number of Points";
const char* PointRingModelObject::VERTICES_FLUOROPHORE_PROP = "Vertices Fluorophore Model";


PointRingModelObject
::PointRingModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Point Ring");

  // Set up geometry
  m_Points = vtkSmartPointer<vtkPoints>::New();
  m_Points->Allocate(10, 10);
  
  m_PointSource = vtkSmartPointer<vtkPolyData>::New();
  m_PointSource->SetPoints(m_Points);
  
  m_SphereSource = vtkSmartPointer<vtkSphereSource>::New();
  m_SphereSource->SetThetaResolution(16);
  m_SphereSource->SetPhiResolution(8);

  m_GlyphSource = vtkSmartPointer<vtkGlyph3D>::New();
  m_GlyphSource->SetInput(m_PointSource);
  m_GlyphSource->SetSourceConnection(m_SphereSource->GetOutputPort());

  // Need a vertex source for the points so that the fluorescence renderer
  // knows it has something to render.
  m_VertexSource = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  m_VertexSource->SetInput(m_PointSource);

  SetGeometrySubAssembly("All", m_GlyphSource);
  SetGeometrySubAssembly("Points", m_VertexSource);

  // Set up properties
  AddProperty(new ModelObjectProperty(VISIBLE_RADIUS_PROP, 50.0, "nanometers", true, false));
  AddProperty(new ModelObjectProperty(RING_RADIUS_PROP, 100.0, "nanometers", true, true));
  AddProperty(new ModelObjectProperty(NUMBER_OF_POINTS_PROP, 5, "-", true, false));

  AddProperty(new GeometryVerticesFluorophoreProperty
              (VERTICES_FLUOROPHORE_PROP, m_VertexSource));

  // Update again.
  Update();
}


PointRingModelObject
::PointRingModelObject(DirtyListener* dirtyListener, ModelObjectPropertyList* properties) 
  : ModelObject(dirtyListener, properties) {
}


PointRingModelObject
::~PointRingModelObject() {

}


void
PointRingModelObject
::Update() {
  double visibleRadius = 
    GetProperty(VISIBLE_RADIUS_PROP)->GetDoubleValue();
  m_SphereSource->SetRadius(visibleRadius);

  double ringRadius =
    GetProperty(RING_RADIUS_PROP)->GetDoubleValue();
  
  int numPoints = 
    GetProperty(NUMBER_OF_POINTS_PROP)->GetIntValue();

  m_Points->SetNumberOfPoints(numPoints);

  double angleIncrement = 2.0*M_PI / static_cast<double>(numPoints);
  for (int i = 0; i < numPoints; i++) {
    double xyz[3] = {0.0, 0.0, 0.0};
    double angle = angleIncrement * static_cast<double>(i);
    xyz[1] = ringRadius * cos(angle);
    xyz[2] = ringRadius * sin(angle);
    m_Points->SetPoint(static_cast<vtkIdType>(i), xyz);
    m_Points->Modified();
  }

  GetGeometrySubAssembly("Points")->GetInput()->Modified();
}

