#include <TorusModelObject.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>

#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkVolumetricTorusSource.h>


const char* TorusModelObject::OBJECT_TYPE_NAME          = "TorusModel";

const char* TorusModelObject::CROSS_SECTION_RADIUS_PROP = "Cross Section Radius";
const char* TorusModelObject::RING_RADIUS_PROP          = "Ring Radius";
const char* TorusModelObject::SURFACE_FLUOR_PROP        = "Surface Fluorophore Model";
const char* TorusModelObject::VOLUME_FLUOR_PROP         = "Volume Fluorophore Model";


TorusModelObject
::TorusModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Torus");

  // Set up geometry
  m_TorusSource = vtkSmartPointer<vtkVolumetricTorusSource>::New();
  m_TorusSource->SetThetaResolution(32);
  m_TorusSource->SetPhiResolution(16);

  vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
    vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  surfaceFilter->SetInputConnection(m_TorusSource->GetOutputPort());

  m_GeometrySource = vtkSmartPointer<vtkPolyDataNormals>::New();
  m_GeometrySource->SetInputConnection(surfaceFilter->GetOutputPort());
 
  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(CROSS_SECTION_RADIUS_PROP, 100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(RING_RADIUS_PROP, 500.0, "nanometers"));

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, surfaceFilter));
  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP, m_TorusSource));

  // Must call this after setting up properties
  Update();
}


TorusModelObject
::~TorusModelObject() {

}


void
TorusModelObject
::Update() {
  m_TorusSource->SetCrossSectionRadius(GetProperty(CROSS_SECTION_RADIUS_PROP)->GetDoubleValue());
  m_TorusSource->SetRingRadius(GetProperty(RING_RADIUS_PROP)->GetDoubleValue());
}


