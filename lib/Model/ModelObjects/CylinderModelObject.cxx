#include <CylinderModelObject.h>
#include <FluorophoreModelObjectProperty.h>

#include <vtkCylinderSource.h>
#include <vtkTriangleFilter.h>

const char* CylinderModelObject::OBJECT_TYPE_NAME = "CylinderModel";

const char* CylinderModelObject::RADIUS_ATT       = "radius";
const char* CylinderModelObject::RADIUS_PROP      = "Radius";

const char* CylinderModelObject::LENGTH_ATT       = "length";
const char* CylinderModelObject::LENGTH_PROP      = "Length";

const char* CylinderModelObject::SURFACE_FLUOR_ATT = "surfaceFluorophoreModel";
const char* CylinderModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";

const char* CylinderModelObject::VOLUME_FLUOR_ATT = "volumeFluorophoreModel";
const char* CylinderModelObject::VOLUME_FLUOR_PROP = "Volume Fluorophore Model";


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

  AddProperty(new FluorophoreModelObjectProperty(SURFACE_FLUOR_PROP,
                                                 UNIFORM_RANDOM_SURFACE_SAMPLE,
                                                 m_GeometrySource));
  AddProperty(new FluorophoreModelObjectProperty(VOLUME_FLUOR_PROP,
                                                 UNIFORM_RANDOM_VOLUME_SAMPLE,
                                                 m_GeometrySource));

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


void
CylinderModelObject
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
  sprintf(buf, doubleFormat, GetProperty(LENGTH_PROP)->GetDoubleValue());
  xmlNewProp(root, BAD_CAST LENGTH_ATT, BAD_CAST buf);
}


void
CylinderModelObject
::RestoreFromXML(xmlNodePtr node) {
  ModelObject::RestoreFromXML(node);

  char* radius = (char*) xmlGetProp(node, BAD_CAST RADIUS_ATT);
  if (radius) {
    GetProperty(RADIUS_PROP)->SetDoubleValue(atof(radius));
  }

  char* length = (char*) xmlGetProp(node, BAD_CAST LENGTH_ATT);
  if (length) {
    GetProperty(LENGTH_PROP)->SetDoubleValue(atof(length));
  }
}
