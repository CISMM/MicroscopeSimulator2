#include <VolumeUniformFluorophoreProperty.h>

#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataToTetrahedralGrid.h>
#include <vtkVolumeUniformPointSampler.h>


VolumeUniformFluorophoreProperty::
VolumeUniformFluorophoreProperty(const std::string& name,
                                 vtkPolyDataAlgorithm* geometry,
                                 bool editable, bool optimizable)
  : FluorophoreModelObjectProperty(name, geometry, editable, optimizable) {

  vtkSmartPointer<vtkPolyDataToTetrahedralGrid> tetrahedralizer =
    vtkSmartPointer<vtkPolyDataToTetrahedralGrid>::New();
  tetrahedralizer->SetInputConnection(geometry->GetOutputPort());
  
  m_Sampler = vtkSmartPointer<vtkVolumeUniformPointSampler>::New();
  m_Sampler->SetInputConnection(tetrahedralizer->GetOutputPort());

  m_FluorophoreOutput = m_Sampler;
  SetDensity(100);
}


VolumeUniformFluorophoreProperty::
~VolumeUniformFluorophoreProperty() {
}


void
VolumeUniformFluorophoreProperty
::UseFixedDensity() {
  m_Sampler->UseFixedNumberOfSamplesOff();
}


bool
VolumeUniformFluorophoreProperty
::GetUseFixedDensity() {
  return !static_cast<bool>(m_Sampler->GetUseFixedNumberOfSamples());
}


void
VolumeUniformFluorophoreProperty
::UseFixedNumberOfFluorophores() {
  m_Sampler->UseFixedNumberOfSamplesOn();
}


bool
VolumeUniformFluorophoreProperty
::GetUseFixedNumberOfFluorophores() {
  return static_cast<bool>(m_Sampler->GetUseFixedNumberOfSamples());
}


void
VolumeUniformFluorophoreProperty
::SetDensity(double density) {
  m_Density = density;
  m_Sampler->SetDensity(m_Density * 1.0e-9);
  m_Sampler->Modified(); // Ensure resampling is performed no matter what
  m_Sampler->Update();
}


double
VolumeUniformFluorophoreProperty
::GetDensity() {
  return m_Density;
}


void
VolumeUniformFluorophoreProperty
::SetNumberOfFluorophores(int number) {
  m_Sampler->SetNumberOfSamples(number);
}


int
VolumeUniformFluorophoreProperty
::GetNumberOfFluorophores() {
  return m_Sampler->GetNumberOfSamples();
}


double
VolumeUniformFluorophoreProperty
::GetGeometryVolume() {
  return m_Sampler->GetVolume() * 1.0e-9;
}



void
VolumeUniformFluorophoreProperty
::GetXMLConfiguration(xmlNodePtr root) {
  FluorophoreModelObjectProperty::GetXMLConfiguration(root);

  char value[256];
  sprintf(value, "%f", GetDensity());
  xmlNewProp(root, BAD_CAST "density", BAD_CAST value);

  sprintf(value, "%d", GetNumberOfFluorophores());
  xmlNewProp(root, BAD_CAST "numberOfFluorophores", BAD_CAST value);

  sprintf(value, "%s", GetUseFixedNumberOfFluorophores() ? "true" : "false");
  xmlNewProp(root, BAD_CAST "useFixedNumberOfFluorophores", BAD_CAST value);
}


void
VolumeUniformFluorophoreProperty
::RestoreFromXML(xmlNodePtr root) {
  FluorophoreModelObjectProperty::RestoreFromXML(root);

  char* value = (char*) xmlGetProp(root, BAD_CAST "density");
  if (value) {
    double density = atof(value);
    SetDensity(density);
  }

  value = (char*) xmlGetProp(root, BAD_CAST "numberOfFluorophores");
  if (value) {
    int numFluorophores = atoi(value);
    SetNumberOfFluorophores(numFluorophores);
  }

  value = (char*) xmlGetProp(root, BAD_CAST "useFixedNumberOfFluorophores");
  if (value) {
    std::string stringValue(value);
    if (stringValue == "true")
      UseFixedNumberOfFluorophores();
    else
      UseFixedDensity();
  }
}
