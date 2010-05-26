#include <FluorophoreModelObjectProperty.h>
#include <StringUtils.h>

#include <vtkMassProperties.h>
#include <vtkSmartPointer.h>


FluorophoreModelObjectProperty
::FluorophoreModelObjectProperty(const std::string& name,
                                 FluorophoreModelType type,
                                 vtkPolyDataAlgorithm* geometry,
                                 bool editable, bool optimizable)
  : ModelObjectProperty(name, ModelObjectProperty::FLUOROPHORE_MODEL_TYPE,
                        "-", editable, optimizable) {
  SetFluorophoreModelType(type);
  m_GeometrySource = geometry;
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
}


void 
FluorophoreModelObjectProperty
::SetFluorophoreModelTypeToGeometryVertices() {
  SetFluorophoreModelType(GEOMETRY_VERTICES);
}


void
FluorophoreModelObjectProperty
::SetFluorophoreModelTypeToUniformRandomSurfaceSample() {
  SetFluorophoreModelType(UNIFORM_RANDOM_SURFACE_SAMPLE);
}


void
FluorophoreModelObjectProperty
::SetFluorophoreModelTypeToUniformRandomVolumeSample() {
  SetFluorophoreModelType(UNIFORM_RANDOM_VOLUME_SAMPLE);
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
