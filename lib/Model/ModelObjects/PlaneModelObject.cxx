#include <PlaneModelObject.h>
#include <FluorophoreModelObjectProperty.h>

#include <vtkPlaneSource.h>
#include <vtkTriangleFilter.h>

const char* PlaneModelObject::OBJECT_TYPE_NAME = "PlaneModel";

const char* PlaneModelObject::WIDTH_ATT   = "width";
const char* PlaneModelObject::WIDTH_PROP  = "Width";

const char* PlaneModelObject::HEIGHT_ATT  = "height";
const char* PlaneModelObject::HEIGHT_PROP = "Height";

const char* PlaneModelObject::SURFACE_FLUOR_ATT = "surfaceFluorophoreModel";
const char* PlaneModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";


PlaneModelObject
::PlaneModelObject(DirtyListener* dirtyListener)
  : ModelObject(dirtyListener) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Plane");

  // Set up geometry
  m_PlaneSource = vtkSmartPointer<vtkPlaneSource>::New();
  m_PlaneSource->SetOrigin(0.0, 0.0, 0.0);
  m_PlaneSource->SetXResolution(2);
  m_PlaneSource->SetYResolution(2);

  m_GeometrySource = vtkSmartPointer<vtkTriangleFilter>::New();
  m_GeometrySource->SetInputConnection(m_PlaneSource->GetOutputPort());

  SetGeometrySubAssembly("All", m_GeometrySource);

  // Set up properties
  AddProperty(new ModelObjectProperty(WIDTH_PROP,  100.0, "nanometers"));
  AddProperty(new ModelObjectProperty(HEIGHT_PROP, 100.0, "nanometers"));
  AddProperty(new FluorophoreModelObjectProperty(SURFACE_FLUOR_PROP,
                                                 UNIFORM_RANDOM_SURFACE_SAMPLE,
                                                 m_GeometrySource));

  // Must call this after setting up properties
  Update();
}


PlaneModelObject
::~PlaneModelObject() {

}


void
PlaneModelObject
::Update() {
  double width  = GetProperty(PlaneModelObject::WIDTH_PROP)->GetDoubleValue();
  double height = GetProperty(PlaneModelObject::HEIGHT_PROP)->GetDoubleValue();
  m_PlaneSource->SetPoint1(width, 0.0, 0.0);
  m_PlaneSource->SetPoint2(0.0, height, 0.0);
}


void
PlaneModelObject
::GetXMLConfiguration(xmlNodePtr node) {
  // Create child element of node.
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST OBJECT_TYPE_NAME, NULL);

  // Fill in common properties for all model objects.
  ModelObject::GetXMLConfiguration(root);

  // Add object-specific properties.
  char doubleFormat[] = "%f";
  char buf[128];
  sprintf(buf, doubleFormat, GetProperty(WIDTH_PROP)->GetDoubleValue());
  xmlNewProp(root, BAD_CAST WIDTH_ATT, BAD_CAST buf);
  sprintf(buf, doubleFormat, GetProperty(HEIGHT_PROP)->GetDoubleValue());
  xmlNewProp(root, BAD_CAST HEIGHT_ATT, BAD_CAST buf);
}
