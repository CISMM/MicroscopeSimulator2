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
::SetSamplingMode(SamplingMode_t mode) {
  m_SamplingMode = mode;

  if (m_SamplingMode == FIXED_DENSITY) {
    m_Sampler->UseFixedNumberOfSamplesOff();
  } else {
    // FIXED_NUMBER
    m_Sampler->UseFixedNumberOfSamplesOn();
  }
}


void
UniformFluorophoreProperty
::SetSamplingModeToFixedDensity() {
  SetSamplingMode(FIXED_DENSITY);
}


void
UniformFluorophoreProperty
::SetSamplingModeToFixedNumber() {
  SetSamplingMode(FIXED_NUMBER);
}


UniformFluorophoreProperty::SamplingMode_t
UniformFluorophoreProperty
::GetSamplingMode() {
  return m_SamplingMode;
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
::SetSamplePattern(SamplePattern_t pattern) {
  m_SamplePattern = pattern;

  if (m_SamplePattern == SINGLE_POINT) {
    m_PointRingSource->SetRadius(0.0);
    m_PointRingSource->SetNumberOfPoints(1);
  } else {
    // POINT_RING
    m_PointRingSource->SetRadius(m_PointRingRadius);
    m_PointRingSource->SetNumberOfPoints(m_NumberOfFluorophores);
  }
}


void
UniformFluorophoreProperty
::SetSamplePatternToSinglePoint() {
  SetSamplePattern(SINGLE_POINT);
}


void
UniformFluorophoreProperty
::SetSamplePatternToPointRing() {
  SetSamplePattern(POINT_RING);
}


UniformFluorophoreProperty::SamplePattern_t
UniformFluorophoreProperty
::GetSamplePattern() {
  return m_SamplePattern;
}


void
UniformFluorophoreProperty
::SetNumberOfRingFluorophores(int numFluorophores) {
  m_NumberOfFluorophores = numFluorophores;
  if (GetSamplePattern() == POINT_RING) {
    m_PointRingSource->SetNumberOfPoints(numFluorophores);
    m_PointRingSource->Update();
  }
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
  if (GetSamplePattern() == POINT_RING) {
    m_PointRingSource->SetRadius(radius);
    m_PointRingSource->Update();
  }
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

  if (GetSamplingMode() == FIXED_DENSITY)
    xmlNewProp(root, BAD_CAST "samplingMode", BAD_CAST "fixedDensity");
  else if (GetSamplingMode() == FIXED_NUMBER)
    xmlNewProp(root, BAD_CAST "samplingMode", BAD_CAST "fixedNumber");

  if (GetSamplePattern() == SINGLE_POINT)
    xmlNewProp(root, BAD_CAST "samplePattern", BAD_CAST "singlePoint");
  else if (GetSamplePattern() == POINT_RING)
    xmlNewProp(root, BAD_CAST "samplePattern", BAD_CAST "pointRing");

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

  value = (char*) xmlGetProp(root, BAD_CAST "samplingMode");
  if (value) {
    std::string stringValue(value);
    if (stringValue == "fixedDensity") {
      SetSamplingModeToFixedDensity();
    } else if (stringValue == "fixedNumber") {
      SetSamplingModeToFixedNumber();
    }
  }

  value = (char*) xmlGetProp(root, BAD_CAST "samplePattern");
  if (value) {
    std::string stringValue(value);
    if (stringValue == "singlePoint") {
      SetSamplePatternToSinglePoint();
    } else if (stringValue == "samplePattern") {
      SetSamplePatternToPointRing();
    }
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
