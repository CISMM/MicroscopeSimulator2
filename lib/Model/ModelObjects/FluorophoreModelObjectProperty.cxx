#include <FluorophoreModelObjectProperty.h>

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
