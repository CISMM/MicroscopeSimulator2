#include <FluorophoreModelObjectProperty.h>
#include <StringUtils.h>

#include <vtkMassProperties.h>
#include <vtkPolyDataToTetrahedralGrid.h>
#include <vtkSmartPointer.h>
#include <vtkSurfaceUniformPointSampler.h>
#include <vtkTriangleFilter.h>
#include <vtkVolumeUniformPointSampler.h>


FluorophoreModelObjectProperty
::FluorophoreModelObjectProperty(const std::string& name,
                                 FluorophoreModelType type,
                                 vtkPolyDataAlgorithm* geometry,
                                 bool editable, bool optimizable)
  : ModelObjectProperty(name, ModelObjectProperty::FLUOROPHORE_MODEL_TYPE,
                        "-", editable, optimizable) {
  m_GeometrySource = geometry;
  m_FluorophoreOutput = NULL;
  SetFluorophoreModelType(type);
  SetEnabled(true);
  SetFluorophoreChannelToAll();
  SetDensity(1000.0);
}


FluorophoreModelObjectProperty
::~FluorophoreModelObjectProperty() {

}


void 
FluorophoreModelObjectProperty
::SetFluorophoreModelType(FluorophoreModelType type) {
  m_FluorophoreModelType = type;

  switch (type) {
  case GEOMETRY_VERTICES:
    SetFluorophoreModelTypeToGeometryVertices();
    break;

  case UNIFORM_RANDOM_SURFACE_SAMPLE:
    SetFluorophoreModelTypeToUniformRandomSurfaceSample();
    break;

  case UNIFORM_RANDOM_VOLUME_SAMPLE:
    SetFluorophoreModelTypeToUniformRandomVolumeSample();
    break;
  }
}


void 
FluorophoreModelObjectProperty
::SetFluorophoreModelTypeToGeometryVertices() {
  m_FluorophoreModelType = GEOMETRY_VERTICES;

  m_FluorophoreOutput = m_GeometrySource;
}


void
FluorophoreModelObjectProperty
::SetFluorophoreModelTypeToUniformRandomSurfaceSample() {
  m_FluorophoreModelType = UNIFORM_RANDOM_SURFACE_SAMPLE;

  vtkSmartPointer<vtkTriangleFilter> triangulizer = vtkSmartPointer<vtkTriangleFilter>::New();
  triangulizer->PassLinesOff();
  triangulizer->PassVertsOff();
  triangulizer->SetInputConnection(m_GeometrySource->GetOutputPort());
  
  vtkSmartPointer<vtkSurfaceUniformPointSampler> surfaceSampler = 
    vtkSmartPointer<vtkSurfaceUniformPointSampler>::New();
  surfaceSampler->SetInputConnection(triangulizer->GetOutputPort());
  surfaceSampler->Update();
  surfaceSampler->GetOutput()->Update();

  m_FluorophoreOutput = surfaceSampler;
}


void
FluorophoreModelObjectProperty
::SetFluorophoreModelTypeToUniformRandomVolumeSample() {
  m_FluorophoreModelType = UNIFORM_RANDOM_VOLUME_SAMPLE;

  vtkSmartPointer<vtkPolyDataToTetrahedralGrid> tetrahedralizer =
    vtkSmartPointer<vtkPolyDataToTetrahedralGrid>::New();
  tetrahedralizer->SetInputConnection(m_GeometrySource->GetOutputPort());
  
  vtkSmartPointer<vtkVolumeUniformPointSampler> volumeSampler = 
    vtkSmartPointer<vtkVolumeUniformPointSampler>::New();
  volumeSampler->SetInputConnection(tetrahedralizer->GetOutputPort());

  m_FluorophoreOutput = volumeSampler;
}


FluorophoreModelType
FluorophoreModelObjectProperty
::GetFluorophoreModelType() {
  return m_FluorophoreModelType;
}


void
FluorophoreModelObjectProperty
::SetDensity(double density) {
  m_Density = density;

  // Hack for now
  vtkSurfaceUniformPointSampler* surfaceSampler = 
    dynamic_cast<vtkSurfaceUniformPointSampler*>(m_FluorophoreOutput.GetPointer());
  if (surfaceSampler) {
    surfaceSampler->SetDensity(m_Density * 1e-6);
  }

  vtkVolumeUniformPointSampler* volumeSampler =
    dynamic_cast<vtkVolumeUniformPointSampler*>(m_FluorophoreOutput.GetPointer());
  if (volumeSampler) {
    volumeSampler->SetDensity(m_Density * 1e-9);
  }
}


double
FluorophoreModelObjectProperty
::GetDensity() {
  return m_Density;
}


void
FluorophoreModelObjectProperty
::SetEnabled(bool enabled) {
  m_Enabled = enabled;
}


bool
FluorophoreModelObjectProperty
::GetEnabled() {
  return m_Enabled;
}


void
FluorophoreModelObjectProperty
::SetFluorophoreChannel(FluorophoreChannelType channel) {
  m_Channel = channel;
}


void
FluorophoreModelObjectProperty
::SetFluorophoreChannelToRed() {
  SetFluorophoreChannel(RED_CHANNEL);
}


void
FluorophoreModelObjectProperty
::SetFluorophoreChannelToGreen() {
  SetFluorophoreChannel(GREEN_CHANNEL);
}


void
FluorophoreModelObjectProperty
::SetFluorophoreChannelToBlue() {
  SetFluorophoreChannel(BLUE_CHANNEL);
}


void
FluorophoreModelObjectProperty
::SetFluorophoreChannelToAll() {
  SetFluorophoreChannel(ALL_CHANNELS);
}


FluorophoreChannelType
FluorophoreModelObjectProperty
::GetFluorophoreChannel() {
  return m_Channel;
}


vtkPolyDataAlgorithm*
FluorophoreModelObjectProperty
::GetGeometry() {
  return m_GeometrySource;
}


double
FluorophoreModelObjectProperty
::GetGeometryArea() {
  vtkSmartPointer<vtkMassProperties> props = vtkSmartPointer<vtkMassProperties>::New();
  props->SetInputConnection(m_GeometrySource->GetOutputPort());

  return props->GetSurfaceArea();
}


double
FluorophoreModelObjectProperty
::GetGeometryVolume() {
  vtkSmartPointer<vtkMassProperties> props = vtkSmartPointer<vtkMassProperties>::New();
  props->SetInputConnection(m_GeometrySource->GetOutputPort());

  return props->GetVolume();
}


vtkPolyDataAlgorithm*
FluorophoreModelObjectProperty
::GetFluorophoreOutput() {
  return m_FluorophoreOutput;
}


void
FluorophoreModelObjectProperty
::GetXMLConfiguration(xmlNodePtr root) {
  std::string nodeName(SqueezeString(m_Name));
  xmlNodePtr node = xmlNewChild(root, NULL, BAD_CAST nodeName.c_str(), NULL);

  char value[256];
  sprintf(value, "%f", GetDensity());
  xmlNewProp(node, BAD_CAST "density", BAD_CAST value);

  sprintf(value, "%s", GetEnabled() ? "true" : "false");
  xmlNewProp(node, BAD_CAST "enabled", BAD_CAST value);

  switch (GetFluorophoreChannel()) {
  case RED_CHANNEL:
    sprintf(value, "red");
    break;

  case GREEN_CHANNEL:
    sprintf(value, "green");
    break;

  case BLUE_CHANNEL:
    sprintf(value, "blue");
    break;

  case ALL_CHANNELS:
    sprintf(value, "all");
    break;

  default:
    sprintf(value, "none");
    break;
  }
  xmlNewProp(node, BAD_CAST "channel", BAD_CAST value);

}


void
FluorophoreModelObjectProperty
::RestoreFromXML(xmlNodePtr root) {
  char* value = (char*) xmlGetProp(root, BAD_CAST "density");
  if (value) {
    SetDensity(atof(value));
  }

  value = (char*) xmlGetProp(root, BAD_CAST "enabled");
  if (value) {
    std::string valueStr(value);
    SetEnabled(valueStr == "true");
  }

  value = (char*) xmlGetProp(root, BAD_CAST "channel");
  if (value) {
    std::string valueStr(value);
    if (valueStr == "red") {
      SetFluorophoreChannelToRed();
    } else if (valueStr == "green") {
      SetFluorophoreChannelToGreen();
    } else if (valueStr == "blue") {
      SetFluorophoreChannelToBlue();
    } else if (valueStr == "all") {
      SetFluorophoreChannelToAll();
    }
  }
}
