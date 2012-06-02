#include <FluorophoreModelObjectProperty.h>
#include <StringUtils.h>

#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>


FluorophoreModelObjectProperty
::FluorophoreModelObjectProperty(const std::string& name,
                                 bool editable, bool optimizable)
  : ModelObjectProperty(name, ModelObjectProperty::FLUOROPHORE_MODEL_TYPE,
                        "-", editable, optimizable) {
  m_FluorophoreOutput = NULL;
  SetEnabled(true);
  SetFluorophoreChannelToAll();
  SetIntensityScale(1.0);
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
::SetDoubleValue(double value) {
  SetIntensityScale(value);
}


double
FluorophoreModelObjectProperty
::GetDoubleValue() {
  return GetIntensityScale();
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


void
FluorophoreModelObjectProperty
::SetIntensityScale(double scale) {
  m_IntensityScale = scale;
}


double
FluorophoreModelObjectProperty
::GetIntensityScale() {
  return m_IntensityScale;
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

  sprintf(value, "%f", GetIntensityScale());
  xmlNewProp(root, BAD_CAST "intensityScale", BAD_CAST value);

  if (IsOptimizable())
    xmlNewProp(root, BAD_CAST "optimize", BAD_CAST (GetOptimize() ? "true" : "false"));
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

  value = (char *) xmlGetProp(root, BAD_CAST "intensityScale");
  if (value) {
    SetIntensityScale(atof(value));
  }

  char* optimize = (char *) xmlGetProp(root, BAD_CAST "optimize");
  if (optimize)
    SetOptimize(!strcmp(optimize,"true"));
  else
    SetOptimize(false);
}
