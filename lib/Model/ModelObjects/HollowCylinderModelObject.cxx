#include <HollowCylinderModelObject.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>
#include <GridBasedFluorophoreProperty.h>

#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkVolumetricHollowCylinderSource.h>


const char* HollowCylinderModelObject::OBJECT_TYPE_NAME = "HollowCylinderModel";

const char* HollowCylinderModelObject::OUTER_RADIUS_PROP  = "Outer Radius";
const char* HollowCylinderModelObject::THICKNESS_PROP     = "Thickness";
const char* HollowCylinderModelObject::LENGTH_PROP        = "Length";
const char* HollowCylinderModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";
const char* HollowCylinderModelObject::VOLUME_FLUOR_PROP  = "Volume Fluorophore Model";
const char* HollowCylinderModelObject::GRID_FLUOR_PROP    = "Grid Fluorophore Model";


HollowCylinderModelObject
::HollowCylinderModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {

  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Hollow Cylinder");

  // Set up geometry
  m_HollowCylinderSource = vtkSmartPointer<vtkVolumetricHollowCylinderSource>::New();
  m_HollowCylinderSource->SetResolution(32);

  vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
    vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  surfaceFilter->SetInputConnection(m_HollowCylinderSource->GetOutputPort());

  m_GeometrySource = vtkSmartPointer<vtkPolyDataNormals>::New();
  m_GeometrySource->SetInputConnection(surfaceFilter->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(OUTER_RADIUS_PROP, 100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(THICKNESS_PROP,     10.0, "nanometers"));
  AddProperty(new ModelObjectProperty(LENGTH_PROP,      1000.0, "nanometers"));

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_GeometrySource));
  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP, m_HollowCylinderSource));
  AddProperty(new GridBasedFluorophoreProperty
              (GRID_FLUOR_PROP, m_HollowCylinderSource));

  // Must call this after setting up properties
  Update();
}


HollowCylinderModelObject
::~HollowCylinderModelObject() {

}


void
HollowCylinderModelObject
::Update() {
  double outerRadius = GetProperty(OUTER_RADIUS_PROP)->GetDoubleValue();
  double thickness   = GetProperty(THICKNESS_PROP)->GetDoubleValue();
  m_HollowCylinderSource->SetOuterRadius(GetProperty(OUTER_RADIUS_PROP)->GetDoubleValue());
  m_HollowCylinderSource->SetInnerRadius(outerRadius - thickness);
  m_HollowCylinderSource->SetHeight(GetProperty(LENGTH_PROP)->GetDoubleValue());

  // Call superclass update method
  ModelObject::Update();
}

