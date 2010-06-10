#include <UniformFluorophoreProperty.h>


#include <vtkPointRingSource.h>
#include <vtkGlyph3D.h>
#include <vtkPassThrough.h>
#include <vtkUniformPointSampler.h>


const char* UniformFluorophoreProperty::DENSITY_ATT = "density";
const char* UniformFluorophoreProperty::NUMBER_OF_FLUOROPHORES_ATT = "numberOfFluorophores";
const char* UniformFluorophoreProperty::SAMPLING_MODE_ATT = "samplingMode";
const char* UniformFluorophoreProperty::FIXED_DENSITY_VALUE = "fixedDensity";
const char* UniformFluorophoreProperty::FIXED_NUMBER_VALUE = "fixedNumber";
const char* UniformFluorophoreProperty::SAMPLE_PATTERN_ATT = "samplePattern";
const char* UniformFluorophoreProperty::SINGLE_POINT_VALUE = "singlePoint";
const char* UniformFluorophoreProperty::POINT_RING_VALUE = "pointRing";
const char* UniformFluorophoreProperty::NUMBER_OF_RING_FLUOROPHORES_ATT = "numberOfRingFluorophores";
const char* UniformFluorophoreProperty::RING_RADIUS_ATT = "ringRadius";
const char* UniformFluorophoreProperty::RANDOMIZE_PATTERN_ORIENTATIONS_ATT = "randomizePatternOrientations";


UniformFluorophoreProperty::
UniformFluorophoreProperty(const std::string& name,
                           vtkPolyDataAlgorithm* geometry,
                           bool editable, bool optimizable) 
  : FluorophoreModelObjectProperty(name, geometry, editable, optimizable) {

  // Subclasses need to set up specific samplers and connect them to
  // the point ring glypher and the set it as the fluorophore output.
  m_PointRingRadius = 10.0;
  m_RandomizePatternOrientations = false;

  m_PointRingSource = vtkSmartPointer<vtkPointRingSource>::New();
  m_PointRingSource->SetRadius(0.0);
  m_PointRingSource->SetNumberOfPoints(2);

  m_PointRingGlypher = vtkSmartPointer<vtkGlyph3D>::New();
  m_PointRingGlypher->SetSourceConnection(m_PointRingSource->GetOutputPort());

  m_PassThroughFilter = vtkSmartPointer<vtkPassThrough>::New();

  m_FluorophoreOutput = m_PassThroughFilter;
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
  m_Sampler->SetDensity(density * GetDensityScale());
  m_Sampler->Modified(); // Ensure resampling is performed no matter what
  m_Sampler->Update();
}


double
UniformFluorophoreProperty
::GetDensity() {
  return m_Sampler->GetDensity() / GetDensityScale();
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
    m_PassThroughFilter->SetInputConnection(m_Sampler->GetOutputPort());
  } else if (m_SamplePattern == POINT_RING) {
    m_PassThroughFilter->SetInputConnection(m_PointRingGlypher->GetOutputPort());
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
  m_PointRingSource->SetNumberOfPoints(numFluorophores);
}


int
UniformFluorophoreProperty
::GetNumberOfRingFluorophores() {
  return m_PointRingSource->GetNumberOfPoints();
}

void
UniformFluorophoreProperty
::SetRingRadius(double radius) {
  m_PointRingRadius = radius;
  m_PointRingSource->SetRadius(radius);
}


double
UniformFluorophoreProperty
::GetRingRadius() {
  return  m_PointRingRadius;
}


void
UniformFluorophoreProperty
::SetRandomizePatternOrientations(bool enabled) {
  m_RandomizePatternOrientations = enabled;
}


void
UniformFluorophoreProperty
::RandomizePatternOrientationsOn() {
  SetRandomizePatternOrientations(true);
}


void
UniformFluorophoreProperty
::RandomizePatternOrientationsOff() {
  SetRandomizePatternOrientations(false);
}


bool
UniformFluorophoreProperty
::GetRandomizePatternOrientations() {
  return m_RandomizePatternOrientations;
}


void
UniformFluorophoreProperty
::GetXMLConfiguration(xmlNodePtr root) {
  FluorophoreModelObjectProperty::GetXMLConfiguration(root);

  char value[256];
  sprintf(value, "%f", GetDensity());
  xmlNewProp(root, BAD_CAST DENSITY_ATT, BAD_CAST value);

  sprintf(value, "%d", GetNumberOfFluorophores());
  xmlNewProp(root, BAD_CAST NUMBER_OF_FLUOROPHORES_ATT, BAD_CAST value);

  if (GetSamplingMode() == FIXED_DENSITY)
    xmlNewProp(root, BAD_CAST SAMPLING_MODE_ATT, BAD_CAST FIXED_DENSITY_VALUE);
  else if (GetSamplingMode() == FIXED_NUMBER)
    xmlNewProp(root, BAD_CAST SAMPLING_MODE_ATT, BAD_CAST FIXED_NUMBER_VALUE);

  if (GetSamplePattern() == SINGLE_POINT)
    xmlNewProp(root, BAD_CAST SAMPLE_PATTERN_ATT, BAD_CAST SINGLE_POINT_VALUE);
  else if (GetSamplePattern() == POINT_RING)
    xmlNewProp(root, BAD_CAST SAMPLE_PATTERN_ATT, BAD_CAST POINT_RING_VALUE);

  sprintf(value, "%d", GetNumberOfRingFluorophores());
  xmlNewProp(root, BAD_CAST NUMBER_OF_RING_FLUOROPHORES_ATT, BAD_CAST value);

  sprintf(value, "%f", GetRingRadius());
  xmlNewProp(root, BAD_CAST RING_RADIUS_ATT, BAD_CAST value);

  if (GetRandomizePatternOrientations())
    xmlNewProp(root, BAD_CAST RANDOMIZE_PATTERN_ORIENTATIONS_ATT, BAD_CAST "true");
  else
    xmlNewProp(root, BAD_CAST RANDOMIZE_PATTERN_ORIENTATIONS_ATT, BAD_CAST "false");
}


void
UniformFluorophoreProperty
::RestoreFromXML(xmlNodePtr root) {
  FluorophoreModelObjectProperty::RestoreFromXML(root);

  char* value = (char*) xmlGetProp(root, BAD_CAST DENSITY_ATT);
  if (value) {
    double density = atof(value);
    SetDensity(density);
  }

  value = (char*) xmlGetProp(root, BAD_CAST NUMBER_OF_FLUOROPHORES_ATT);
  if (value) {
    int numFluorophores = atoi(value);
    SetNumberOfFluorophores(numFluorophores);
  }

  value = (char*) xmlGetProp(root, BAD_CAST SAMPLING_MODE_ATT);
  if (value) {
    std::string stringValue(value);
    if (stringValue == std::string(FIXED_DENSITY_VALUE)) {
      SetSamplingModeToFixedDensity();
    } else if (stringValue == std::string(FIXED_NUMBER_VALUE)) {
      SetSamplingModeToFixedNumber();
    }
  }

  value = (char*) xmlGetProp(root, BAD_CAST SAMPLE_PATTERN_ATT);
  if (value) {
    std::string stringValue(value);
    if (stringValue == std::string(SINGLE_POINT_VALUE)) {
      SetSamplePatternToSinglePoint();
    } else if (stringValue == std::string(POINT_RING_VALUE)) {
      SetSamplePatternToPointRing();
    }
  }

  value = (char*) xmlGetProp(root, BAD_CAST NUMBER_OF_RING_FLUOROPHORES_ATT);
  if (value) {
    SetNumberOfRingFluorophores(atoi(value));
  }

  value = (char*) xmlGetProp(root, BAD_CAST RING_RADIUS_ATT);
  if (value) {
    SetRingRadius(atof(value));
  }

  value = (char*) xmlGetProp(root, BAD_CAST RANDOMIZE_PATTERN_ORIENTATIONS_ATT);
  if (value) {
    if (std::string(value) == "true")
      RandomizePatternOrientationsOn();
    else 
      RandomizePatternOrientationsOff();
  }
}
