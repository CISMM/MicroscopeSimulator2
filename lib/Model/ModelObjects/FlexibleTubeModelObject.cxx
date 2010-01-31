#include <FlexibleTubeModelObject.h>

#include <DirtyListener.h>
#include <FluorophoreModelObjectProperty.h>
#include <ModelObjectPropertyList.h>

#include <vtkMath.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkPoints.h>
#include <vtkSpline.h>
#include <vtkTubeFilter.h>
#include <vtkTriangleFilter.h>


const char* FlexibleTubeModelObject::OBJECT_TYPE_NAME = "FlexibleTubeModel";

const char* FlexibleTubeModelObject::RADIUS_ATT  = "radius";
const char* FlexibleTubeModelObject::RADIUS_PROP = "Radius";

const char* FlexibleTubeModelObject::SURFACE_FLUOR_ATT = "surfaceFluorophoreModel";
const char* FlexibleTubeModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";

const char* FlexibleTubeModelObject::VOLUME_FLUOR_ATT = "volumeFluorophoreModel";
const char* FlexibleTubeModelObject::VOLUME_FLUOR_PROP = "Volume Fluorophore Model";


FlexibleTubeModelObject
::FlexibleTubeModelObject(DirtyListener* dirtyListener) 
  : PointSetModelObject(dirtyListener, CreateProperties()) {

  m_ObjectTypeName = OBJECT_TYPE_NAME;
  SetName("Flexible Tube Model");

  // Set up geometry
  m_Points = vtkSmartPointer<vtkPoints>::New();
  m_Points->Allocate(100, 100);

  m_Spline = vtkSmartPointer<vtkParametricSpline>::New();
  m_Spline->SetPoints(m_Points);
  m_Spline->ParameterizeByLengthOff();

  m_SplineSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
  m_SplineSource->SetParametricFunction(m_Spline);
  m_SplineSource->SetUResolution(10);

  m_TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  m_TubeFilter->CappingOn();
  m_TubeFilter->SetNumberOfSides(16);
  m_TubeFilter->SetInputConnection(m_SplineSource->GetOutputPort());
  
  m_TubeSource = vtkSmartPointer<vtkTriangleFilter>::New();
  m_TubeSource->SetInputConnection(m_TubeFilter->GetOutputPort());

  SetGeometrySubAssembly("All", m_TubeSource);

  // Set up properties
  AddProperty(new ModelObjectProperty(RADIUS_PROP, 50.0, "nanometers"));
  AddProperty(new ModelObjectProperty(NUMBER_OF_POINTS_PROP, 2, "-", true, false));
  AddProperty(new FluorophoreModelObjectProperty(SURFACE_FLUOR_PROP,
                                                       UNIFORM_RANDOM_SURFACE_SAMPLE,
                                                       m_TubeSource));
  AddProperty(new FluorophoreModelObjectProperty(VOLUME_FLUOR_PROP,
                                                       UNIFORM_RANDOM_VOLUME_SAMPLE,
                                                       m_TubeSource));


  m_PointPropertyStartingIndex = GetPropertyList()->GetSize();

  AddProperty(new ModelObjectProperty("X1", -500.0, "nanometers"));
  AddProperty(new ModelObjectProperty("Y1",    0.0, "nanometers"));
  AddProperty(new ModelObjectProperty("Z1",    0.0, "nanometers"));
  AddProperty(new ModelObjectProperty("X2",  500.0, "nanometers"));
  AddProperty(new ModelObjectProperty("Y2",    0.0, "nanometers"));
  AddProperty(new ModelObjectProperty("Z2",    0.0, "nanometers"));
  
  // Must call this after setting up properties
  Update();
}


ModelObjectPropertyList*
FlexibleTubeModelObject
::CreateProperties() {
  ModelObjectPropertyList* list = CreateDefaultProperties();

  return list;
}


FlexibleTubeModelObject
::~FlexibleTubeModelObject() {
  
}


void
FlexibleTubeModelObject
::Update() {
  double radius =
    GetProperty(FlexibleTubeModelObject::RADIUS_PROP)->GetDoubleValue();
  m_TubeFilter->SetRadius(radius);

  int numPoints = 
    GetProperty(PointSetModelObject::NUMBER_OF_POINTS_PROP)->GetIntValue();

  int prevPoints = m_Points->GetNumberOfPoints();
  m_Points->SetNumberOfPoints(numPoints);

  bool needsUpdate = prevPoints != numPoints;
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
      needsUpdate = true;
    }
  }

  if (needsUpdate) {
    m_Points->Modified();
    m_Spline->Modified();
    m_SplineSource->Modified();
    m_SplineSource->Update();

    // Adjust tessellation based on curve length
    double length = GetLength();
    m_SplineSource->SetUResolution(static_cast<int>(length/100.0));
  }
}


double
FlexibleTubeModelObject
::GetLength() {
  double length = 0.0;

  // Approximate the length of the spline by the length of the
  // polyline connecting the points.

  double p1[3];
  m_Points->GetPoint(0, p1);
  for (int i = 1; i < m_Points->GetNumberOfPoints(); i++) {
    double p2[3];
    m_Points->GetPoint(i, p2);

    length += sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
    p1[0] = p2[0];   p1[1] = p2[1];   p1[2] = p2[2];
  }

  return length;
}


void
FlexibleTubeModelObject
::GetXMLConfiguration(xmlNodePtr node) {
  // Create child element of node.
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST OBJECT_TYPE_NAME, NULL);

  // Fill in common properties for all model objects.
  ModelObject::GetXMLConfiguration(root);

  // Add object-specific properties.
  char doubleFormat[] = "%f";
  char attValueBuf[128];
  sprintf(attValueBuf, doubleFormat, GetProperty(RADIUS_PROP)->GetDoubleValue());
  xmlNewProp(root, BAD_CAST RADIUS_ATT, BAD_CAST attValueBuf);

  char intFormat[] = "%d";
  sprintf(attValueBuf, intFormat, GetProperty(NUMBER_OF_POINTS_PROP)->GetIntValue());
  xmlNewProp(root, BAD_CAST NUMBER_OF_POINTS_ATT, BAD_CAST attValueBuf);

  // Print point properties
  int numPoints = GetProperty(NUMBER_OF_POINTS_PROP)->GetIntValue();
  int offset = m_PointPropertyStartingIndex;
  for (int i = 0; i < numPoints; i++) {
    xmlNodePtr pointNode = xmlNewChild(root, NULL, BAD_CAST "Point", NULL);

    sprintf(attValueBuf, doubleFormat, GetProperty(i*3+0+offset)->GetDoubleValue());
    xmlNewProp(pointNode, BAD_CAST "X", BAD_CAST attValueBuf);
    sprintf(attValueBuf, doubleFormat, GetProperty(i*3+1+offset)->GetDoubleValue());
    xmlNewProp(pointNode, BAD_CAST "Y", BAD_CAST attValueBuf);
    sprintf(attValueBuf, doubleFormat, GetProperty(i*3+2+offset)->GetDoubleValue());
    xmlNewProp(pointNode, BAD_CAST "Z", BAD_CAST attValueBuf);
  }
}
