#include <DiskModelObject.h>
#include <FluorophoreModelObjectProperty.h>

#include <vtkDiskSource.h>
#include <vtkTriangleFilter.h>


const char* DiskModelObject::OBJECT_TYPE_NAME = "DiskModel";

const char* DiskModelObject::RADIUS_ATT       = "radius";
const char* DiskModelObject::RADIUS_PROP      = "Radius";

const char* DiskModelObject::SURFACE_FLUOR_ATT = "surfaceFluorophoreModel";
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


void
DiskModelObject
::GetXMLConfiguration(xmlNodePtr node) {
  // Create child element of node.
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST OBJECT_TYPE_NAME, NULL);

  // Fill in common properties for all model objects.
  ModelObject::GetXMLConfiguration(root);

  // Add object-specific properties.
  char doubleFormat[] = "%f";
  char buf[128];
  sprintf(buf, doubleFormat, GetProperty(RADIUS_PROP)->GetDoubleValue());
  xmlNewProp(root, BAD_CAST RADIUS_ATT, BAD_CAST buf);

}


void
DiskModelObject
::RestoreFromXML(xmlNodePtr node) {
  ModelObject::RestoreFromXML(node);

  char* radius = (char*) xmlGetProp(node, BAD_CAST RADIUS_ATT);
  if (radius) {
    GetProperty(RADIUS_PROP)->SetDoubleValue(atof(radius));
  }
}
