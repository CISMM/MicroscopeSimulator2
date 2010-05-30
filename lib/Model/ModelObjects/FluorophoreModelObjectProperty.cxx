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
                                 vtkPolyDataAlgorithm* geometry,
                                 bool editable, bool optimizable)
  : ModelObjectProperty(name, ModelObjectProperty::FLUOROPHORE_MODEL_TYPE,
                        "-", editable, optimizable) {
  m_GeometrySource = geometry;
  m_FluorophoreOutput = NULL;
  SetEnabled(true);
  SetFluorophoreChannelToAll();
  SetDensity(1000.0);
}


FluorophoreModelObjectProperty
::~FluorophoreModelObjectProperty() {

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
