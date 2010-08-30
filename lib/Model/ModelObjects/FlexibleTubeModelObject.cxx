#include <FlexibleTubeModelObject.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>

#include <DirtyListener.h>
#include <ModelObjectPropertyList.h>
#include <XMLHelper.h>

#include <vtkMath.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkPoints.h>
#include <vtkPolyDataToTetrahedralGrid.h>
#include <vtkSpline.h>
#include <vtkTubeFilter.h>
#include <vtkTriangleFilter.h>


const char* FlexibleTubeModelObject::OBJECT_TYPE_NAME = "FlexibleTubeModel";

const char* FlexibleTubeModelObject::RADIUS_PROP        = "Radius";
const char* FlexibleTubeModelObject::SURFACE_FLUOR_PROP = "Surface Fluorophore Model";
const char* FlexibleTubeModelObject::VOLUME_FLUOR_PROP  = "Volume Fluorophore Model";


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

  AddProperty(new SurfaceUniformFluorophoreProperty
              (SURFACE_FLUOR_PROP, m_TubeSource));

  // Perform the tetrahedralization here
  vtkSmartPointer<vtkPolyDataToTetrahedralGrid> tetrahedralizer =
    vtkSmartPointer<vtkPolyDataToTetrahedralGrid>::New();
  tetrahedralizer->SetInputConnection(m_TubeSource->GetOutputPort());

  AddProperty(new VolumeUniformFluorophoreProperty
              (VOLUME_FLUOR_PROP, tetrahedralizer));


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
    double xyz[3] = {0.0, 0.0, 0.0};
    ModelObjectProperty* prop;

    sprintf(buf, "X%d", i);
    prop = GetProperty(std::string(buf));
    if (prop)
      xyz[0] = prop->GetDoubleValue();
    
    sprintf(buf, "Y%d", i);
    prop = GetProperty(std::string(buf));
    if (prop)
      xyz[1] = prop->GetDoubleValue();
    
    sprintf(buf, "Z%d", i);
    prop = GetProperty(std::string(buf));
    if (prop)
      xyz[2] = prop->GetDoubleValue();
  
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
::RestoreFromXML(xmlNodePtr node) {
  ModelObjectProperty* numPointsProp = GetProperty(NUMBER_OF_POINTS_PROP);
  std::string elementName = numPointsProp->GetXMLElementName();
  xmlNodePtr numPointsNode =
    xmlGetFirstElementChildWithName(node, BAD_CAST elementName.c_str());
  numPointsProp->RestoreFromXML(numPointsNode);

  // Then we need to create all the necessary model object properties
  m_Points->SetNumberOfPoints(0);
  UpdatePointProperties();
  Update();

  // Now that we are ready, restore all the properties
  ModelObject::RestoreFromXML(node);
}
