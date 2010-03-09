#include <HollowCylinderModelObject.h>
#include <FluorophoreModelObjectProperty.h>

#include <vtkDiskSource.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>


const char* HollowCylinderModelObject::OBJECT_TYPE_NAME = "HollowCylinderModel";

const char* HollowCylinderModelObject::OUTER_RADIUS_PROP  = "Outer Radius";
const char* HollowCylinderModelObject::THICKNESS_PROP     = "Thickness";
const char* HollowCylinderModelObject::LENGTH_PROP        = "Length";
const char* HollowCylinderModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";
const char* HollowCylinderModelObject::VOLUME_FLUOR_PROP  = "Volume Fluorophore Model";


HollowCylinderModelObject
::HollowCylinderModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {

  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Hollow Cylinder");

  // Set up geometry
  m_DiskSource = vtkSmartPointer<vtkDiskSource>::New();
  m_DiskSource->SetRadialResolution(1);
  m_DiskSource->SetCircumferentialResolution(32);

  m_Transform = vtkSmartPointer<vtkTransform>::New();
  m_Transform->RotateX(90.0);

  m_TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  m_TransformFilter->SetTransform(m_Transform);
  m_TransformFilter->SetInputConnection(m_DiskSource->GetOutputPort());

  m_ExtrusionSource = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
  m_ExtrusionSource->SetExtrusionTypeToVectorExtrusion();
  m_ExtrusionSource->CappingOn();
  m_ExtrusionSource->SetScaleFactor(1.0);
  m_ExtrusionSource->SetVector(0.0, 1.0, 0.0);
  m_ExtrusionSource->SetInputConnection(m_TransformFilter->GetOutputPort());

  m_TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  m_TriangleFilter->SetInputConnection(m_ExtrusionSource->GetOutputPort());

  m_GeometrySource = vtkSmartPointer<vtkPolyDataNormals>::New();
  m_GeometrySource->SetInputConnection(m_TriangleFilter->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(OUTER_RADIUS_PROP, 100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(THICKNESS_PROP,     10.0, "nanometers"));
  AddProperty(new ModelObjectProperty(LENGTH_PROP,      1000.0, "nanometers"));

  AddProperty(new FluorophoreModelObjectProperty(SURFACE_FLUOR_PROP,
                                                 UNIFORM_RANDOM_SURFACE_SAMPLE,
                                                 m_GeometrySource));
  AddProperty(new FluorophoreModelObjectProperty(VOLUME_FLUOR_PROP,
                                                 UNIFORM_RANDOM_VOLUME_SAMPLE,
                                                 m_GeometrySource));

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
  m_DiskSource->SetOuterRadius(GetProperty(OUTER_RADIUS_PROP)->GetDoubleValue());
  m_DiskSource->SetInnerRadius(outerRadius - thickness);
  m_ExtrusionSource->SetScaleFactor(GetProperty(LENGTH_PROP)->GetDoubleValue());
}

