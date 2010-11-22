#include <EllipsoidModelObject.h>

#include <ModelObjectPropertyList.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>
#include <GridBasedFluorophoreProperty.h>

#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkVolumetricEllipsoidSource.h>


const char* EllipsoidModelObject::OBJECT_TYPE_NAME = "EllipsoidModel";

const char* EllipsoidModelObject::RADIUS_X_PROP      = "Radius X";
const char* EllipsoidModelObject::RADIUS_Y_PROP      = "Radius Y";
const char* EllipsoidModelObject::RADIUS_Z_PROP      = "Radius Z";
const char* EllipsoidModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";
const char* EllipsoidModelObject::VOLUME_FLUOR_PROP  = "Volume Fluorophore Model";
const char* EllipsoidModelObject::GRID_FLUOR_PROP    = "Grid Fluorophore Model";



EllipsoidModelObject
::EllipsoidModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {

  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Ellipsoid");

  // Set up geometry
  m_EllipsoidSource = vtkSmartPointer<vtkVolumetricEllipsoidSource>::New();
  m_EllipsoidSource->SetThetaResolution(32);
  m_EllipsoidSource->SetPhiResolution(16);

  vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
    vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  surfaceFilter->SetInputConnection(m_EllipsoidSource->GetOutputPort());

  m_GeometrySource = vtkSmartPointer<vtkPolyDataNormals>::New();
  m_GeometrySource->SetInputConnection(surfaceFilter->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(RADIUS_X_PROP, 100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(RADIUS_Y_PROP, 100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(RADIUS_Z_PROP, 100.0, "nanometers"));

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_GeometrySource));
  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP, m_EllipsoidSource));
  AddProperty(new GridBasedFluorophoreProperty
              (GRID_FLUOR_PROP, m_EllipsoidSource));

  // Must call this after setting up properties
  Update();
}


EllipsoidModelObject
::~EllipsoidModelObject() {

}


void
EllipsoidModelObject
::Update() {
  double radiusX = GetProperty(RADIUS_X_PROP)->GetDoubleValue();
  double radiusY = GetProperty(RADIUS_Y_PROP)->GetDoubleValue();
  double radiusZ = GetProperty(RADIUS_Z_PROP)->GetDoubleValue();
  m_EllipsoidSource->SetRadius(radiusX, radiusY, radiusZ);

  // Call superclass update method
  ModelObject::Update();
}
