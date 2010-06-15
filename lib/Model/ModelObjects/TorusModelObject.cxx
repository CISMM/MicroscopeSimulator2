#include <TorusModelObject.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>

#include <vtkCleanPolyData.h>
#include <vtkParametricTorus.h>
#include <vtkParametricFunctionSource.h>
#include <vtkTriangleFilter.h>


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
  m_Torus = vtkSmartPointer<vtkParametricTorus>::New();
  m_TorusSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
  m_TorusSource->SetParametricFunction(m_Torus);

  m_GeometrySource = vtkSmartPointer<vtkTriangleFilter>::New();
  m_GeometrySource->SetInputConnection(m_TorusSource->GetOutputPort());
 
  // Slip in a point merger filter
  vtkSmartPointer<vtkCleanPolyData> merger = 
    vtkSmartPointer<vtkCleanPolyData>::New();
  merger->SetTolerance(1e-6);
  merger->SetInputConnection(m_GeometrySource->GetOutputPort());

  SetGeometrySubAssembly("All", merger);

  // Set up properties
  AddProperty(new ModelObjectProperty(CROSS_SECTION_RADIUS_PROP, 100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(RING_RADIUS_PROP, 500.0, "nanometers"));

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, merger));
  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP, merger));

  // Must call this after setting up properties
  Update();
}


TorusModelObject
::~TorusModelObject() {

}


void
TorusModelObject
::Update() {
  m_Torus->SetCrossSectionRadius(GetProperty(CROSS_SECTION_RADIUS_PROP)->GetDoubleValue());
  m_Torus->SetRingRadius(GetProperty(RING_RADIUS_PROP)->GetDoubleValue());
}


