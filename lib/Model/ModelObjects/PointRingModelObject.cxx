#include <PointRingModelObject.h>
#include <GeometryVerticesFluorophoreProperty.h>

#include <DirtyListener.h>
#include <ModelObjectPropertyList.h>
#include <XMLHelper.h>

#include <vtkGlyph3D.h>
#include <vtkMath.h>
#include <vtkPointRingSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkVertexGlyphFilter.h>


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
  m_PointRingSource = vtkSmartPointer<vtkPointRingSource>::New();

  m_SphereSource = vtkSmartPointer<vtkSphereSource>::New();
  m_SphereSource->SetThetaResolution(16);
  m_SphereSource->SetPhiResolution(8);

  m_GlyphSource = vtkSmartPointer<vtkGlyph3D>::New();
  m_GlyphSource->SetInputConnection(m_PointRingSource->GetOutputPort());
  m_GlyphSource->SetSourceConnection(m_SphereSource->GetOutputPort());

  // Need a vertex source for the points so that the fluorescence renderer
  // knows it has something to render.
  m_VertexSource = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  m_VertexSource->SetInputConnection(m_PointRingSource->GetOutputPort());

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

  m_PointRingSource->SetRadius(ringRadius);
  m_PointRingSource->SetNumberOfPoints(numPoints);
  m_PointRingSource->Update();

  // Call superclass update method
  ModelObject::Update();
}

