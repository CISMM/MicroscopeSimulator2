#include <PointSetModelObject.h>
#include <FluorophoreModelObjectProperty.h>

#include <DirtyListener.h>
#include <ModelObjectPropertyList.h>
#include <XMLHelper.h>

#include <vtkGlyph3D.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkVertexGlyphFilter.h>


const char* PointSetModelObject::OBJECT_TYPE_NAME = "PointSetModel";

const char* PointSetModelObject::VISIBLE_RADIUS_PROP       = "Visible Radius";
const char* PointSetModelObject::NUMBER_OF_POINTS_PROP     = "Number of Points";
const char* PointSetModelObject::VERTICES_FLUOROPHORE_PROP = "Vertices Fluorophore Model";


PointSetModelObject
::PointSetModelObject(DirtyListener* dirtyListener) :
  ModelObject(dirtyListener) {
  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Point Set");

  // Set up geometry
  m_Points = vtkSmartPointer<vtkPoints>::New();
  m_Points->Allocate(100, 100);
  
  m_PointSource = vtkSmartPointer<vtkPolyData>::New();
  m_PointSource->SetPoints(m_Points);
  
  m_SphereSource = vtkSmartPointer<vtkSphereSource>::New();
  m_SphereSource->SetThetaResolution(16);
  m_SphereSource->SetPhiResolution(8);

  m_GlyphSource = vtkSmartPointer<vtkGlyph3D>::New();
  m_GlyphSource->SetInput(m_PointSource);
  m_GlyphSource->SetSourceConnection(m_SphereSource->GetOutputPort());

  // Need a vertex source for the points so that the fluorescence renderer
  // knows it has something to render.
  m_VertexSource = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  m_VertexSource->SetInput(m_PointSource);

  SetGeometrySubAssembly("All", m_GlyphSource);
  SetGeometrySubAssembly("Points", m_VertexSource);

  // Set up properties
  AddProperty(new ModelObjectProperty(VISIBLE_RADIUS_PROP, 50.0, "nanometers", true, false));
  AddProperty(new ModelObjectProperty(NUMBER_OF_POINTS_PROP, 1, "-", true, false));
  AddProperty(new FluorophoreModelObjectProperty(VERTICES_FLUOROPHORE_PROP,
                                                 GEOMETRY_VERTICES,
                                                 m_VertexSource));

  m_PointPropertyStartingIndex = GetPropertyList()->GetSize();

  AddProperty(new ModelObjectProperty("X1", 0.0, "nanometers"));
  AddProperty(new ModelObjectProperty("Y1", 0.0, "nanometers"));
  AddProperty(new ModelObjectProperty("Z1", 0.0, "nanometers"));

  // Must call this after setting up properties
  Update();
}


PointSetModelObject
::PointSetModelObject(DirtyListener* dirtyListener, ModelObjectPropertyList* properties) 
  : ModelObject(dirtyListener, properties) {
}


PointSetModelObject
::~PointSetModelObject() {

}


void
PointSetModelObject
::Update() {
  double radius = 
    GetProperty(PointSetModelObject::VISIBLE_RADIUS_PROP)->GetDoubleValue();
  m_SphereSource->SetRadius(radius);

  int numPoints = 
    GetProperty(PointSetModelObject::NUMBER_OF_POINTS_PROP)->GetIntValue();

  UpdatePointProperties();

  m_Points->SetNumberOfPoints(numPoints);

  for (int i = 1; i <= numPoints; i++) {
    char buf[128];
    double xyz[3];

    sprintf(buf, "X%d", i);
    xyz[0] = GetProperty(std::string(buf))->GetDoubleValue();

    sprintf(buf, "Y%d", i);
    xyz[1] = GetProperty(std::string(buf))->GetDoubleValue();

    sprintf(buf, "Z%d", i);
    xyz[2] = GetProperty(std::string(buf))->GetDoubleValue();

    double* currXYZ = m_Points->GetPoint(static_cast<vtkIdType>(i-1));

    if (xyz[0] != currXYZ[0] || xyz[1] != currXYZ[1] || xyz[2] != currXYZ[2]) {
      m_Points->SetPoint(static_cast<vtkIdType>(i-1), xyz);
    }
  }

  GetGeometrySubAssembly("Points")->GetInput()->Modified();

  m_Points->Modified();
}


void
PointSetModelObject
::RestoreFromXML(xmlNodePtr node) {
  ModelObjectProperty* numPointsProp = GetProperty(NUMBER_OF_POINTS_PROP);
  std::string elementName = numPointsProp->GetXMLElementName();
  xmlNodePtr numPointsNode =
    xmlGetFirstElementChildWithName(node, BAD_CAST elementName.c_str());
  numPointsProp->RestoreFromXML(numPointsNode);

  // Then we need to create all the necessary model object properties
  UpdatePointProperties();  

  // Now that we are ready, restore all the properties
  ModelObject::RestoreFromXML(node);
}


void
PointSetModelObject
::Sully() {
  UpdatePointProperties();

  ModelObject::Sully();
}


void
PointSetModelObject
::UpdatePointProperties() {

  // Update the number of point properties according to the number of points
  int previousNumPoints = (GetNumberOfProperties() - m_PointPropertyStartingIndex + 1) / 3;
  int newNumPoints = GetProperty(NUMBER_OF_POINTS_PROP)->GetIntValue();

  if (newNumPoints <= 0)
    return;

  int diff = newNumPoints - previousNumPoints;
  char buf[128];

  if (diff > 0) {
    // Add properties
    for (int i = previousNumPoints; i < newNumPoints; i++) {
      sprintf(buf, "X%d", i+1);
      AddProperty(new ModelObjectProperty(std::string(buf), 0.0, "nanometers"));
      sprintf(buf, "Y%d", i+1);
      AddProperty(new ModelObjectProperty(std::string(buf), 0.0, "nanometers"));
      sprintf(buf, "Z%d", i+1);
      AddProperty(new ModelObjectProperty(std::string(buf), 0.0, "nanometers"));
    }

  } else if (diff < 0) {
    // Remove properties
    for (int i = previousNumPoints; i > newNumPoints; i--) {
      sprintf(buf, "X%d", i);
      delete GetProperty(std::string(buf));

      sprintf(buf, "Y%d", i);
      delete GetProperty(std::string(buf));

      sprintf(buf, "Z%d", i);
      delete GetProperty(std::string(buf));

      PopProperty();
      PopProperty();
      PopProperty();
    }
  }
}
