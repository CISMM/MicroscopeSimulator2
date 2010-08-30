#include <SphereModelObject.h>

#include <ModelObjectPropertyList.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>

#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkVolumetricEllipsoidSource.h>


const char* SphereModelObject::OBJECT_TYPE_NAME = "SphereModel";

const char* SphereModelObject::RADIUS_PROP        = "Radius";
const char* SphereModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";
const char* SphereModelObject::VOLUME_FLUOR_PROP  = "Volume Fluorophore Model";


SphereModelObject
::SphereModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener, CreateProperties()) {

  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Sphere");

  // Set up geometry
  m_SphereSource = vtkSmartPointer<vtkVolumetricEllipsoidSource>::New();
  m_SphereSource->SetThetaResolution(32);
  m_SphereSource->SetPhiResolution(16);

  vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
    vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  surfaceFilter->SetInputConnection(m_SphereSource->GetOutputPort());

  m_GeometrySource = vtkSmartPointer<vtkPolyDataNormals>::New();
  m_GeometrySource->SetInputConnection(surfaceFilter->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(RADIUS_PROP, 100.0, "nanometers"));

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_GeometrySource));
  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP, m_SphereSource));

  // Must call this after setting up properties
  Update();
}


SphereModelObject
::~SphereModelObject() {

}


void
SphereModelObject
::Update() {
  double radius = GetProperty("Radius")->GetDoubleValue();
  m_SphereSource->SetRadius(radius, radius, radius);
}


ModelObjectPropertyList*
SphereModelObject
::CreateProperties() {
  ModelObjectPropertyList* props = new ModelObjectPropertyList();
  props->AddProperty(new ModelObjectProperty(NAME_PROP, ModelObjectProperty::STRING_TYPE,
                                             "-", true, false));
  props->AddProperty(new ModelObjectProperty(VISIBLE_PROP, true, "-", true, false));
  props->AddProperty(new ModelObjectProperty(SCANNABLE_PROP, true, "-", true, false));
  props->AddProperty(new ModelObjectProperty(X_POSITION_PROP, 0.0, "nanometers"));
  props->AddProperty(new ModelObjectProperty(Y_POSITION_PROP, 0.0, "nanometers"));
  props->AddProperty(new ModelObjectProperty(Z_POSITION_PROP, 0.0, "nanometers"));

  return props;
}
