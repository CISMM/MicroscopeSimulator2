#include <UniformFluorophoreProperty.h>


#include <vtkPointRingSource.h>
#include <vtkGlyph3D.h>
#include <vtkUniformPointSampler.h>


UniformFluorophoreProperty::
UniformFluorophoreProperty(const std::string& name,
                           vtkPolyDataAlgorithm* geometry,
                           bool editable, bool optimizable) 
  : FluorophoreModelObjectProperty(name, geometry, editable, optimizable) {

  // Subclasses need to set up specific samplers and connect them to
  // the point ring glypher and the set it as the fluorophore output.

  m_UsePointRingCluster = false;
  m_PointRingRadius = 10.0;
  m_NumberOfFluorophores = 2;

  m_PointRingSource = vtkSmartPointer<vtkPointRingSource>::New();
  m_PointRingSource->SetRadius(0.0);
  m_PointRingSource->SetNumberOfPoints(1);

  m_PointRingGlypher = vtkSmartPointer<vtkGlyph3D>::New();
  m_PointRingGlypher->SetSourceConnection(m_PointRingSource->GetOutputPort());

  m_FluorophoreOutput = m_PointRingGlypher;
}


UniformFluorophoreProperty
::~UniformFluorophoreProperty() {

}



void
UniformFluorophoreProperty
::UseFixedDensity() {
  m_Sampler->UseFixedNumberOfSamplesOff();
}


bool
UniformFluorophoreProperty
::GetUseFixedDensity() {
  return !static_cast<bool>(m_Sampler->GetUseFixedNumberOfSamples());
}


void
UniformFluorophoreProperty
::UseFixedNumberOfFluorophores() {
  m_Sampler->UseFixedNumberOfSamplesOn();
}


bool
UniformFluorophoreProperty
::GetUseFixedNumberOfFluorophores() {
  return static_cast<bool>(m_Sampler->GetUseFixedNumberOfSamples());
}


void
UniformFluorophoreProperty
::SetDensity(double density) {
  m_Density = density;
  m_Sampler->SetDensity(m_Density * GetDensityScale());
  m_Sampler->Modified(); // Ensure resampling is performed no matter what
  m_Sampler->Update();
}


double
UniformFluorophoreProperty
::GetDensity() {
  return m_Density;
}


void
UniformFluorophoreProperty
::SetNumberOfFluorophores(int number) {
  m_Sampler->SetNumberOfSamples(number);
}


int
UniformFluorophoreProperty
::GetNumberOfFluorophores() {
  return m_Sampler->GetNumberOfSamples();
}


void
UniformFluorophoreProperty
::UseOneFluorophorePerSample() {
  m_UsePointRingCluster = false;
  m_PointRingSource->SetRadius(0.0);
  m_PointRingSource->SetNumberOfPoints(1);
}


bool
UniformFluorophoreProperty
::GetUseOneFluorophorePerSample() {
  return !m_UsePointRingCluster;
}

void
UniformFluorophoreProperty
::UsePointRingClusterPerSample() {
  m_UsePointRingCluster = true;
  m_PointRingSource->SetRadius(m_PointRingRadius);
  m_PointRingSource->SetNumberOfPoints(m_NumberOfFluorophores);
}


bool
UniformFluorophoreProperty
::GetUsePointRingClusterPerSample() {
  return m_UsePointRingCluster;
}

void
UniformFluorophoreProperty
::SetNumberOfRingFluorophores(int numFluorophores) {
  m_NumberOfFluorophores = numFluorophores;
  if (m_UsePointRingCluster)
    m_PointRingSource->SetNumberOfPoints(numFluorophores);
}


int
UniformFluorophoreProperty
::GetNumberOfRingFluorophores() {
  return m_NumberOfFluorophores;
}

void
UniformFluorophoreProperty
::SetRingRadius(double radius) {
  m_PointRingRadius = radius;
  if (m_UsePointRingCluster)
    m_PointRingSource->SetRadius(radius);
}


double
UniformFluorophoreProperty
::GetRingRadius() {
  return  m_PointRingRadius;
}


void
UniformFluorophoreProperty
::GetXMLConfiguration(xmlNodePtr root) {
  FluorophoreModelObjectProperty::GetXMLConfiguration(root);

  char value[256];
  sprintf(value, "%f", GetDensity());
  xmlNewProp(root, BAD_CAST "density", BAD_CAST value);

  sprintf(value, "%d", GetNumberOfFluorophores());
  xmlNewProp(root, BAD_CAST "numberOfFluorophores", BAD_CAST value);

  sprintf(value, "%s", GetUseFixedNumberOfFluorophores() ? "true" : "false");
  xmlNewProp(root, BAD_CAST "useFixedNumberOfFluorophores", BAD_CAST value);

  sprintf(value, "%s", GetUsePointRingClusterPerSample() ? "true" : "false");
  xmlNewProp(root, BAD_CAST "usePointRingClusterPerSample", BAD_CAST value);

  sprintf(value, "%d", GetNumberOfRingFluorophores());
  xmlNewProp(root, BAD_CAST "numberOfRingFluorophores", BAD_CAST value);

  sprintf(value, "%f", GetRingRadius());
  xmlNewProp(root, BAD_CAST "ringRadius", BAD_CAST value);
}


void
UniformFluorophoreProperty
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

  value = (char*) xmlGetProp(root, BAD_CAST "usePointRingClusterPerSample");
  if (value) {
    std::string stringValue(value);
    if (stringValue == "true")
      UsePointRingClusterPerSample();
    else
      UseOneFluorophorePerSample();
  }

  value = (char*) xmlGetProp(root, BAD_CAST "numberOfRingFluorophores");
  if (value) {
    SetNumberOfRingFluorophores(atoi(value));
  }

  value = (char*) xmlGetProp(root, BAD_CAST "ringRadius");
  if (value) {
    SetRingRadius(atof(value));
  }
}
