#include <CylinderModelObject.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>

#include <vtkCylinderSource.h>
#include <vtkTriangleFilter.h>

const char* CylinderModelObject::OBJECT_TYPE_NAME = "CylinderModel";

const char* CylinderModelObject::RADIUS_PROP        = "Radius";
const char* CylinderModelObject::LENGTH_PROP        = "Length";
const char* CylinderModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";
const char* CylinderModelObject::VOLUME_FLUOR_PROP  = "Volume Fluorophore Model";


CylinderModelObject
::CylinderModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Cylinder");

  // Set up geometry
  m_CylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
  m_CylinderSource->CappingOn();
  m_CylinderSource->SetResolution(32);

  m_GeometrySource = vtkSmartPointer<vtkTriangleFilter>::New();
  m_GeometrySource->SetInputConnection(m_CylinderSource->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(RADIUS_PROP,  100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(LENGTH_PROP, 1000.0, "nanometers"));

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_GeometrySource));
  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP,  m_GeometrySource));

  // Must call this after setting up properties
  Update();
}


CylinderModelObject
::~CylinderModelObject() {

}


void
CylinderModelObject
::Update() {
  m_CylinderSource->SetRadius(GetProperty(RADIUS_PROP)->GetDoubleValue());
  m_CylinderSource->SetHeight(GetProperty(LENGTH_PROP)->GetDoubleValue());
}
