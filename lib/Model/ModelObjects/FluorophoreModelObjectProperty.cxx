#include <FluorophoreModelObjectProperty.h>
#include <StringUtils.h>

#include <vtkAlgorithm.h>
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
}


FluorophoreModelObjectProperty
::~FluorophoreModelObjectProperty() {

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


vtkAlgorithm*
FluorophoreModelObjectProperty
::GetFluorophoreOutput() {
  return m_FluorophoreOutput;
}


void
FluorophoreModelObjectProperty
::GetXMLConfiguration(xmlNodePtr root) {
  char value[256];
  sprintf(value, "%s", GetEnabled() ? "true" : "false");
  xmlNewProp(root, BAD_CAST "enabled", BAD_CAST value);

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
  xmlNewProp(root, BAD_CAST "channel", BAD_CAST value);

}


void
FluorophoreModelObjectProperty
::RestoreFromXML(xmlNodePtr root) {
  char* value = (char*) xmlGetProp(root, BAD_CAST "enabled");
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
