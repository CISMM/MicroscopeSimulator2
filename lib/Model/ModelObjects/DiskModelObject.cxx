#include <DiskModelObject.h>
#include <GridBasedFluorophoreProperty.h>
#include <SurfaceUniformFluorophoreProperty.h>

#include <vtkDiskSource2.h>


const char* DiskModelObject::OBJECT_TYPE_NAME = "DiskModel";

const char* DiskModelObject::RADIUS_PROP        = "Radius";
const char* DiskModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";

DiskModelObject
::DiskModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Disk");

  // Set up geometry
  m_GeometrySource = vtkSmartPointer<vtkDiskSource2>::New();
  m_GeometrySource->SetCircumferentialResolution(32);

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(RADIUS_PROP, 500.0, "nanometers"));
  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_GeometrySource));

  // Must call this after setting up properties
  Update();
}


DiskModelObject
::~DiskModelObject() {

}


void
DiskModelObject
::Update() {
  m_GeometrySource->SetRadius(GetProperty("Radius")->GetDoubleValue());

  // Call superclass update method
  ModelObject::Update();
}

