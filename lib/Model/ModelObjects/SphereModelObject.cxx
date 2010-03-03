#include <SphereModelObject.h>
#include <FluorophoreModelObjectProperty.h>

#include <vtkSphereSource.h>
#include <vtkTriangleFilter.h>


const char* SphereModelObject::OBJECT_TYPE_NAME = "SphereModel";

const char* SphereModelObject::RADIUS_ATT = "radius";
const char* SphereModelObject::RADIUS_PROP = "Radius";

const char* SphereModelObject::SURFACE_FLUOR_ATT = "surfaceFluorophoreModel";
const char* SphereModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";

const char* SphereModelObject::VOLUME_FLUOR_ATT = "volumeFluorophoreModel";
const char* SphereModelObject::VOLUME_FLUOR_PROP = "Volume Fluorophore Model";


SphereModelObject
::SphereModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Sphere");

  // Set up geometry
  m_SphereSource = vtkSmartPointer<vtkSphereSource>::New();
  m_SphereSource->SetThetaResolution(32);
  m_SphereSource->SetPhiResolution(16);

  m_GeometrySource = vtkSmartPointer<vtkTriangleFilter>::New();
  m_GeometrySource->SetInputConnection(m_SphereSource->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(RADIUS_PROP, 100.0, "nanometers"));
  AddProperty(new FluorophoreModelObjectProperty(SURFACE_FLUOR_PROP,
                                                 UNIFORM_RANDOM_SURFACE_SAMPLE,
                                                 m_GeometrySource));
  AddProperty(new FluorophoreModelObjectProperty(VOLUME_FLUOR_PROP,
                                                 UNIFORM_RANDOM_VOLUME_SAMPLE,
                                                 m_GeometrySource));

  // Must call this after setting up properties
  Update();
}


SphereModelObject
::~SphereModelObject() {

}


void
SphereModelObject
::Update() {
  m_SphereSource->SetRadius(GetProperty("Radius")->GetDoubleValue());
}


void
SphereModelObject
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
SphereModelObject
::RestoreFromXML(xmlNodePtr node) {
  ModelObject::RestoreFromXML(node);

  char* radius = (char*) xmlGetProp(node, BAD_CAST RADIUS_ATT);
  if (radius) {
    GetProperty(RADIUS_PROP)->SetDoubleValue(atof(radius));
  }
}
