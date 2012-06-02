#include <PlaneModelObject.h>
#include <SurfaceUniformFluorophoreProperty.h>

#include <vtkRectangleSource.h>

const char* PlaneModelObject::OBJECT_TYPE_NAME = "PlaneModel";

const char* PlaneModelObject::WIDTH_PROP         = "Width";
const char* PlaneModelObject::HEIGHT_PROP        = "Height";
const char* PlaneModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";


PlaneModelObject
::PlaneModelObject(DirtyListener* dirtyListener)
  : ModelObject(dirtyListener) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Plane");

  // Set up geometry
  m_GeometrySource = vtkSmartPointer<vtkRectangleSource>::New();
  m_GeometrySource->SetWidthResolution(2);
  m_GeometrySource->SetHeightResolution(2);

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(WIDTH_PROP,  1000.0, "nanometers"));
  AddProperty(new ModelObjectProperty(HEIGHT_PROP, 1000.0, "nanometers"));

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_GeometrySource));

  // Must call this after setting up properties
  Update();
}


PlaneModelObject
::~PlaneModelObject() {

}


void
PlaneModelObject
::Update() {
  double width  = GetProperty(PlaneModelObject::WIDTH_PROP)->GetDoubleValue();
  double height = GetProperty(PlaneModelObject::HEIGHT_PROP)->GetDoubleValue();
  m_GeometrySource->SetWidth(width);
  m_GeometrySource->SetHeight(height);

  // Call superclass update method
  ModelObject::Update();
}

