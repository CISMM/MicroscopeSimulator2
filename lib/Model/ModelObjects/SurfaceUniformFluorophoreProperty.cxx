#include <SurfaceUniformFluorophoreProperty.h>

#include <vtkMassProperties.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSurfaceUniformPointSampler.h>
#include <vtkTriangleFilter.h>


SurfaceUniformFluorophoreProperty::
SurfaceUniformFluorophoreProperty(const std::string& name,
                                  vtkPolyDataAlgorithm* geometry,
                                  bool editable, bool optimizable) 
  : FluorophoreModelObjectProperty(name, geometry, editable, optimizable) {

  vtkSmartPointer<vtkTriangleFilter> triangulizer = vtkSmartPointer<vtkTriangleFilter>::New();
  triangulizer->PassLinesOff();
  triangulizer->PassVertsOff();
  triangulizer->SetInputConnection(geometry->GetOutputPort());
  
  m_Sampler = vtkSmartPointer<vtkSurfaceUniformPointSampler>::New();
  m_Sampler->SetInputConnection(triangulizer->GetOutputPort());
  m_Sampler->Update();
  m_Sampler->GetOutput()->Update();

  m_FluorophoreOutput = m_Sampler;
  SetDensity(100);
}


SurfaceUniformFluorophoreProperty::
~SurfaceUniformFluorophoreProperty() {
}


void
SurfaceUniformFluorophoreProperty
::UseFixedDensity() {
  m_Sampler->UseFixedNumberOfSamplesOff();
}


bool
SurfaceUniformFluorophoreProperty
::GetUseFixedDensity() {
  return !static_cast<bool>(m_Sampler->GetUseFixedNumberOfSamples());
}


void
SurfaceUniformFluorophoreProperty
::UseFixedNumberOfFluorophores() {
  m_Sampler->UseFixedNumberOfSamplesOn();
}


bool
SurfaceUniformFluorophoreProperty
::GetUseFixedNumberOfFluorophores() {
  return static_cast<bool>(m_Sampler->GetUseFixedNumberOfSamples());
}


void
SurfaceUniformFluorophoreProperty
::SetDensity(double density) {
  m_Density = density;
  m_Sampler->SetDensity(m_Density * 1.0e-6);
  m_Sampler->Modified(); // Ensure resampling is performed no matter what
  m_Sampler->Update();
}


double
SurfaceUniformFluorophoreProperty
::GetDensity() {
  return m_Density;
}


void
SurfaceUniformFluorophoreProperty
::SetNumberOfFluorophores(int number) {
  m_Sampler->SetNumberOfSamples(number);
}


int
SurfaceUniformFluorophoreProperty
::GetNumberOfFluorophores() {
  return m_Sampler->GetNumberOfSamples();
}


double
SurfaceUniformFluorophoreProperty
::GetGeometryArea() {
  vtkSmartPointer<vtkMassProperties> props = vtkSmartPointer<vtkMassProperties>::New();
  props->SetInputConnection(m_GeometrySource->GetOutputPort());

  return props->GetSurfaceArea() * 1.0e-6; 
}


void
SurfaceUniformFluorophoreProperty
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
SurfaceUniformFluorophoreProperty
::RestoreFromXML(xmlNodePtr root) {
  FluorophoreModelObjectProperty::RestoreFromXML(root);

  char* value = (char*) xmlGetProp(root, BAD_CAST "density");
  if (value) {
    double density = atof(value);
    std::cout << "Density: " << density << std::endl;
    SetDensity(density);
  }

  value = (char*) xmlGetProp(root, BAD_CAST "numberOfFluorophores");
  if (value) {
    int numFluorophores = atoi(value);
    std::cout << "Num fluorophores: " << numFluorophores << std::endl;
    SetNumberOfFluorophores(numFluorophores);
  }

  value = (char*) xmlGetProp(root, BAD_CAST "useFixedNumberOfFluorophores");
  if (value) {
    std::string stringValue(value);
    std::cout << "Use fixed num fluorophores: " << value << std::endl;
    if (stringValue == "true")
      UseFixedNumberOfFluorophores();
    else
      UseFixedDensity();
  }
}
