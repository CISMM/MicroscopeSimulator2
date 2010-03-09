#include <DiskModelObject.h>
#include <FluorophoreModelObjectProperty.h>

#include <vtkDiskSource.h>
#include <vtkTriangleFilter.h>


const char* DiskModelObject::OBJECT_TYPE_NAME = "DiskModel";

const char* DiskModelObject::RADIUS_PROP      = "Radius";
const char* DiskModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";


DiskModelObject
::DiskModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Disk");

  // Set up geometry
  m_DiskSource = vtkSmartPointer<vtkDiskSource>::New();
  m_DiskSource->SetInnerRadius(0.0);
  m_DiskSource->SetCircumferentialResolution(32);

  m_GeometrySource = vtkSmartPointer<vtkTriangleFilter>::New();
  m_GeometrySource->SetInputConnection(m_DiskSource->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(RADIUS_PROP, 500.0, "nanometers"));
  AddProperty(new FluorophoreModelObjectProperty(SURFACE_FLUOR_PROP,
                                                 UNIFORM_RANDOM_SURFACE_SAMPLE,
                                                 m_GeometrySource));

  // Must call this after setting up properties
  Update();
}


DiskModelObject
::~DiskModelObject() {

}


void
DiskModelObject
::Update() {
  m_DiskSource->SetOuterRadius(GetProperty("Radius")->GetDoubleValue());
}

